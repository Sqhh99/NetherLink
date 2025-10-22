# WebRTC 信令连接 JWT 认证优化

## 概述

为了提高 WebRTC 信令连接的安全性，我们为 `/ws/webrtc` 端点添加了 JWT token 认证机制，防止用户 ID 被冒用以及断线重连时的身份混淆问题。

## 问题描述

**优化前的安全问题：**
- `/ws` 端点：通过 `login` 消息进行 JWT 验证并设置 `wsConn.isAuth`
- `/ws/webrtc` 端点：仅使用 URL 的 `uid` 参数，不做 token 验证
- **风险：**
  - 用户 ID 可被他人冒用
  - 断线重连时可能出现身份混淆
  - 缺少安全验证层

## 解决方案

### 1. 服务端改动 (Go)

#### 修改文件：`internal/server/websocket.go`

```go
func (s *WSServer) handleWebRTCWebSocket(c *gin.Context) {
	// 从URL参数获取用户ID和token
	userID := c.Query("uid")
	token := c.Query("token")
	
	if userID == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "缺少uid参数"})
		return
	}
	
	if token == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "缺少token参数"})
		return
	}

	// 验证token
	claims := jwt.MapClaims{}
	parsedToken, err := jwt.ParseWithClaims(token, claims, func(token *jwt.Token) (interface{}, error) {
		return []byte(config.GlobalConfig.JWT.Secret), nil
	})

	if err != nil || !parsedToken.Valid {
		log.Printf("WebRTC信令认证失败: token无效, uid=%s, error=%v", userID, err)
		c.JSON(http.StatusUnauthorized, gin.H{"error": "认证失败"})
		return
	}

	// 验证uid是否匹配
	uid, ok := claims["uid"].(string)
	if !ok || uid != userID {
		log.Printf("WebRTC信令认证失败: uid不匹配, expected=%s, actual=%s", userID, uid)
		c.JSON(http.StatusUnauthorized, gin.H{"error": "认证失败"})
		return
	}

	conn, err := s.upgrader.Upgrade(c.Writer, c.Request, nil)
	if err != nil {
		log.Printf("WebRTC WebSocket升级失败: %v", err)
		return
	}

	log.Printf("WebRTC信令连接建立(已认证): userID=%s", userID)

	// 将连接交给WebRTC信令服务器处理
	s.webrtcSignalServer.HandleConnection(conn, userID)
}
```

**关键改进：**
1. 添加 `token` 参数的获取和验证
2. 使用 JWT 验证 token 的有效性
3. 确保 URL 中的 `uid` 与 token 中的 `uid` 一致
4. 认证失败时返回 401 Unauthorized

### 2. 客户端改动 (C++/Qt)

#### 2.1 修改 `SignalClient` 类

**头文件：`include/WebRTC/signalclient.h`**

```cpp
class SignalClient : public QObject {
  Q_OBJECT
 public:
  // 连接到信令服务器（添加 token 参数）
  void Connect(const QString& server_url, 
               const QString& client_id = QString(), 
               const QString& token = QString());
  
 private:
  QString token_;  // JWT token for authentication
};
```

**实现文件：`src/WebRTC/signalclient.cc`**

```cpp
void SignalClient::Connect(const QString& server_url, 
                          const QString& client_id, 
                          const QString& token) {
  if (is_connected_) {
    qDebug() << "Already connected to signaling server";
    return;
  }
  
  server_url_ = server_url;
  token_ = token;
  
  // 生成或使用提供的客户端ID
  if (client_id.isEmpty()) {
    client_id_ = QString("qt_client_%1").arg(
        QDateTime::currentMSecsSinceEpoch() % 1000000);
  } else {
    client_id_ = client_id;
  }
  
  // 构建完整的URL,添加uid和token参数
  QString full_url = server_url;
  if (!full_url.contains("?")) {
    full_url += "?uid=" + client_id_;
  } else {
    full_url += "&uid=" + client_id_;
  }
  
  // 添加token参数(如果提供)
  if (!token_.isEmpty()) {
    full_url += "&token=" + token_;
    qDebug() << "Using token for authentication";
  } else {
    qDebug() << "Warning: No token provided for WebRTC connection";
  }
  
  manual_disconnect_ = false;
  websocket_->open(QUrl(full_url));
}

// 重连时也使用保存的 token
void SignalClient::OnReconnectTimer() {
  qDebug() << "Attempting reconnection, attempt" << reconnect_attempts_;
  Connect(server_url_, client_id_, token_);
}
```

#### 2.2 修改接口和协调器

**接口：`include/WebRTC/icall_observer.h`**

```cpp
class ICallController {
 public:
  // 信令连接（添加 token 参数）
  virtual void ConnectToSignalServer(const std::string& url, 
                                    const std::string& client_id, 
                                    const std::string& token = "") = 0;
};
```

**协调器：`include/WebRTC/call_coordinator.h` 和 `src/WebRTC/call_coordinator.cc`**

```cpp
// 头文件
void ConnectToSignalServer(const std::string& url, 
                          const std::string& client_id, 
                          const std::string& token = "") override;

// 实现文件
void CallCoordinator::ConnectToSignalServer(const std::string& url, 
                                           const std::string& client_id, 
                                           const std::string& token) {
  if (signal_client_) {
    signal_client_->Connect(QString::fromStdString(url), 
                          QString::fromStdString(client_id), 
                          QString::fromStdString(token));
  }
}
```

#### 2.3 修改 `VideoCallManager`

**文件：`src/Manager/VideoCallManager.cpp`**

```cpp
#include "Data/CurrentUser.h"  // 添加头文件

bool VideoCallManager::initialize(const QString& server_url, const QString& client_id) {
    // ... 现有代码 ...
    
    // 获取当前用户的 token
    QString token = CurrentUser::instance().getToken();
    if (token.isEmpty()) {
        qWarning() << "VideoCallManager: No token found, WebRTC connection may fail authentication";
    }
    
    // 连接到信令服务器，传递 token
    coordinator_->ConnectToSignalServer(server_url.toStdString(), 
                                       client_id.toStdString(), 
                                       token.toStdString());
    
    // ... 现有代码 ...
}
```

## 工作流程

### 连接流程

```
1. 用户登录 → 获得 JWT token → 保存到 CurrentUser::instance()

2. 初始化 VideoCallManager
   ↓
3. VideoCallManager 从 CurrentUser 获取 token
   ↓
4. 调用 coordinator_->ConnectToSignalServer(url, client_id, token)
   ↓
5. CallCoordinator 转发到 SignalClient
   ↓
6. SignalClient 构建 WebSocket URL: ws://server/ws/webrtc?uid=xxx&token=yyy
   ↓
7. 服务端接收连接
   ↓
8. 验证 token 的有效性和 uid 的匹配性
   ↓
9. 验证通过 → 建立 WebSocket 连接
   验证失败 → 返回 401 Unauthorized
```

### 断线重连流程

```
1. 检测到连接断开
   ↓
2. SignalClient 自动触发重连
   ↓
3. 使用保存的 token_ 重新连接
   ↓
4. 服务端重新验证 token
   ↓
5. 验证通过 → 恢复连接
```

## 安全改进

### 1. 防止 ID 冒用
- ✅ 必须提供有效的 JWT token
- ✅ token 中的 uid 必须与 URL 中的 uid 匹配
- ✅ 无法仅通过知道 uid 就建立连接

### 2. 防止重连混淆
- ✅ 重连时使用相同的 token
- ✅ 服务端验证 token 的一致性
- ✅ 避免不同客户端使用相同 uid 的情况

### 3. 会话安全
- ✅ token 具有过期时间（由 JWT 配置控制）
- ✅ token 被盗用后可以通过更新密钥使其失效
- ✅ 所有 WebRTC 信令操作都在认证后的连接上进行

## 配置要求

### 服务端

确保 `config.yaml` 中配置了 JWT 密钥：

```yaml
jwt:
  secret: "your-secret-key"  # 应使用强随机密钥
  expire_hours: 24
```

### 客户端

确保在调用 `VideoCallManager::initialize()` 之前，用户已经登录并且 `CurrentUser::instance()` 中保存了有效的 token。

```cpp
// 登录成功后
CurrentUser::instance().setUserInfo(uid, token);

// 初始化 VideoCallManager
VideoCallManager::instance()->initialize(server_url, client_id);
```

## 兼容性说明

- **向后兼容**：token 参数为可选参数（有默认值），旧代码可以编译
- **功能要求**：新服务端**强制**要求 token，未提供 token 的连接会被拒绝
- **迁移建议**：所有客户端应尽快更新以支持 token 认证

## 错误处理

### 客户端错误日志

```cpp
// 无 token 警告
qDebug() << "Warning: No token provided for WebRTC connection";

// 连接失败
qWarning() << "VideoCallManager: Failed to connect to signaling server";
```

### 服务端错误日志

```go
// token 无效
log.Printf("WebRTC信令认证失败: token无效, uid=%s, error=%v", userID, err)

// uid 不匹配
log.Printf("WebRTC信令认证失败: uid不匹配, expected=%s, actual=%s", userID, uid)
```

## 测试建议

1. **正常连接测试**
   - 使用有效 token 连接
   - 验证连接成功建立

2. **无 token 测试**
   - 尝试不带 token 连接
   - 验证服务端返回 400 Bad Request

3. **无效 token 测试**
   - 使用过期或伪造的 token
   - 验证服务端返回 401 Unauthorized

4. **uid 不匹配测试**
   - URL 中的 uid 与 token 中的 uid 不一致
   - 验证服务端拒绝连接

5. **重连测试**
   - 断开连接后自动重连
   - 验证使用相同 token 成功重连

## 总结

通过添加 JWT token 认证，WebRTC 信令连接的安全性得到了显著提升：

- ✅ 防止用户 ID 冒用
- ✅ 解决断线重连混淆
- ✅ 统一认证机制（与 `/ws` 一致）
- ✅ 支持自动重连
- ✅ 易于审计和日志记录

这些改进确保了 WebRTC 通话功能的安全性和可靠性。
