# WebSocket API 文档

## 概述

NetherLink服务器使用WebSocket进行实时通信，支持多种消息类型。本文档详细说明了WebSocket消息的格式、类型以及离线消息同步功能的完整API。

## WebSocket连接

### 连接建立
- **WebSocket URL**: `ws://localhost:8081/ws`
- **认证**: 连接建立后需要先发送登录消息进行认证

### 消息格式

所有WebSocket消息都遵循统一的JSON格式：

```json
{
  "type": "消息类型",
  "payload": "消息内容(JSON对象)"
}
```

## 消息类型

### 1. 登录消息 (login)

**用途**: 建立WebSocket连接后的首次认证

**请求格式**:
```json
{
  "type": "login",
  "payload": {
    "uid": "用户ID",
    "token": "JWT令牌"
  }
}
```

**响应格式**:
```json
{
  "type": "login_success"
}
```

### 2. 聊天消息 (chat)

**用途**: 发送聊天消息

**请求格式**:
```json
{
  "type": "chat",
  "payload": {
    "to": "接收者ID",
    "content": "消息内容",
    "type": "消息类型(text/image/file等)",
    "extra": "额外数据(JSON字符串)",
    "is_group": false
  }
}
```

### 3. 同步离线消息 (sync_offline_messages)

**用途**: 主动请求同步离线消息

**请求格式**:
```json
{
  "type": "sync_offline_messages",
  "payload": {
    "page": 1,
    "page_size": 100
  }
}
```

**参数说明**:
- `page` (可选): 页码，从1开始，默认为1
- `page_size` (可选): 每页消息数量，范围1-100，默认为100

**响应格式**:
```json
{
  "type": "offline_messages",
  "payload": {
    "messages": [
      {
        "message_id": 123,
        "conversation_id": "conv_456",
        "sender_id": "user_789",
        "content": "消息内容",
        "type": "text",
        "extra": "{}",
        "timestamp": "2025-01-01T12:00:00Z"
      }
    ],
    "count": 1
  }
}
```

### 4. 好友请求 (friend_request)

**用途**: 发送好友请求

**请求格式**:
```json
{
  "type": "friend_request",
  "payload": {
    "to_uid": "目标用户ID",
    "message": "请求消息"
  }
}
```

### 5. 处理好友请求 (friend_request_handle)

**用途**: 接受或拒绝好友请求

**请求格式**:
```json
{
  "type": "friend_request_handle",
  "payload": {
    "request_id": 123,
    "action": "accept|reject"
  }
}
```

### 6. 群组加入请求 (group_join_request)

**用途**: 请求加入群组

**请求格式**:
```json
{
  "type": "group_join_request",
  "payload": {
    "group_id": "群组ID",
    "message": "申请消息"
  }
}
```

### 7. 处理群组加入请求 (group_join_request_handle)

**用途**: 批准或拒绝群组加入请求

**请求格式**:
```json
{
  "type": "group_join_request_handle",
  "payload": {
    "request_id": 123,
    "action": "approve|reject"
  }
}
```

## 离线消息同步机制

### 自动同步
- 用户登录后可选择自动同步离线消息（默认关闭）
- 一次性同步最多100条未同步的消息

### 手动同步
- 客户端主动发送 `sync_offline_messages` 消息
- 支持分页查询，适合大量离线消息的场景

### 同步流程
1. 客户端发送同步请求（可选分页参数）
2. 服务器查询数据库中未同步的离线消息
3. 服务器将消息打包发送给客户端
4. 客户端收到消息后可调用HTTP API标记消息为已同步

## HTTP API 配合使用

离线消息同步通常配合以下HTTP API使用：

### 获取离线消息
```
GET /api/messages/offline?page=1&page_size=50
```

### 标记消息已同步
```
POST /api/messages/mark-synced
Content-Type: application/json

{
  "message_ids": [123, 456, 789]
}
```

### 获取未读消息数量
```
GET /api/messages/unread-count
```

### 清理已同步消息
```
DELETE /api/messages/cleanup?days=30
```

## 错误处理

### WebSocket错误响应
```json
{
  "type": "error",
  "payload": {
    "message": "错误信息"
  }
}
```

### 常见错误
- `请先登录`: 未认证状态下发送非登录消息
- `未知的消息类型`: 发送了不支持的消息类型
- 参数验证错误: 分页参数超出范围等

## 使用示例

### JavaScript客户端示例

```javascript
// 建立WebSocket连接
const ws = new WebSocket('ws://localhost:8081/ws');

// 连接建立后发送登录消息
ws.onopen = function() {
  ws.send(JSON.stringify({
    type: 'login',
    payload: {
      uid: 'user123',
      token: 'your-jwt-token'
    }
  }));
};

// 监听消息
ws.onmessage = function(event) {
  const message = JSON.parse(event.data);

  switch(message.type) {
    case 'login_success':
      console.log('登录成功');
      // 发送同步离线消息请求
      ws.send(JSON.stringify({
        type: 'sync_offline_messages',
        payload: {
          page: 1,
          page_size: 50
        }
      }));
      break;

    case 'offline_messages':
      console.log('收到离线消息:', message.payload.messages);
      // 处理离线消息...
      break;

    case 'chat':
      console.log('收到新消息:', message.payload);
      break;
  }
};
```

## 注意事项

1. **认证要求**: 除登录消息外，所有其他消息都需要在认证后发送
2. **消息顺序**: 服务器保证消息按时间顺序发送
3. **分页限制**: 每页最多100条消息，避免单次传输数据过大
4. **连接保持**: 客户端应保持WebSocket连接活跃，处理断线重连
5. **错误处理**: 客户端应妥善处理各种错误情况和网络异常</content>
<parameter name="filePath">/home/sqhh99/workspace/cpp-workspace/ch-server/NetherLink-server/WEBSOCKET_API.md