# NetherLink 视频通话功能集成指南

## 概述

本指南说明如何将 WebRTC 视频通话功能集成到 NetherLink 聊天应用中。

## 架构设计

### 组件说明

1. **VideoCallManager** (单例)
   - 管理全局视频通话状态
   - 封装 CallCoordinator
   - 处理来电通知
   - 管理视频通话窗口生命周期

2. **VideoCallDialog**
   - 视频通话界面
   - 显示本地和远程视频
   - 提供通话控制(挂断、静音等)

3. **IncomingCallDialog**
   - 来电提示对话框
   - 提供接听/拒绝选项
   - 30秒超时自动拒绝

4. **CallCoordinator** (已有组件)
   - WebRTC 核心业务协调器
   - 管理 WebRTCEngine、SignalClient、CallManager

## 集成步骤

### 1. 在应用启动时初始化视频通话管理器

在用户登录成功后,初始化视频通话管理器:

```cpp
// 在 Login.cpp 或 MainWindow.cpp 中
#include "Manager/VideoCallManager.h"

// 用户登录成功后
void onLoginSuccess(const QString& userId) {
    // 初始化视频通话管理器
    QString serverUrl = "ws://localhost:8081/ws/webrtc";  // 从配置读取
    QString clientId = userId;  // 使用用户ID作为客户端ID
    
    VideoCallManager::instance()->initialize(serverUrl, clientId);
    
    // ... 其他初始化代码
}
```

### 2. 在聊天界面中连接视频通话信号

ChatArea 已经添加了 `videoCallRequested` 信号,需要在外层连接:

```cpp
// 在创建 ChatArea 的地方 (例如 MainWindow.cpp)
ChatArea* chatArea = new ChatArea(this);

// 连接视频通话请求信号
connect(chatArea, &ChatArea::videoCallRequested, this, [](const QString& targetUserId) {
    qDebug() << "Requesting video call to:" << targetUserId;
    VideoCallManager::instance()->startCall(targetUserId);
});
```

### 3. 应用退出时清理

```cpp
// 在应用退出或用户登出时
void onApplicationExit() {
    VideoCallManager::instance()->shutdown();
}
```

## 信令服务器配置

需要配置 WebSocket 信令服务器地址,可以从配置文件读取:

```json
// network_config.json
{
    "webrtc_signal_server": "ws://localhost:8081/ws/webrtc",
    "ice_servers": [
        {
            "urls": ["stun:stun.l.google.com:19302"]
        }
    ]
}
```

## 使用流程

### 发起通话

1. 用户在聊天界面点击视频通话图标
2. `FloatingInputBar` 发射 `videoCallRequested` 信号
3. `ChatArea` 转发给外层,调用 `VideoCallManager::startCall(targetUserId)`
4. `VideoCallManager` 创建并显示 `VideoCallDialog`
5. 通过 `CallCoordinator` 发送呼叫请求到信令服务器

### 接收通话

1. 信令服务器推送来电消息
2. `VideoCallManager` 收到 `OnIncomingCall` 回调
3. 显示 `IncomingCallDialog`
4. 用户点击接听/拒绝
5. 如果接听,创建并显示 `VideoCallDialog`,建立 WebRTC 连接

### 通话过程

1. WebRTC 进行 ICE 候选交换
2. 建立 P2P 连接
3. 本地视频显示在小窗口(右下角)
4. 远程视频显示在主窗口
5. 用户可以控制静音、摄像头开关

### 结束通话

1. 用户点击挂断按钮
2. `VideoCallDialog` 发射 `hangupClicked` 信号
3. `VideoCallManager` 调用 `endCall()`
4. 关闭 WebRTC 连接
5. 隐藏视频通话窗口

## 代码示例:完整集成

### 在 MainWindow.cpp 中

```cpp
#include "Manager/VideoCallManager.h"
#include "View/Chat/ChatArea.h"
#include "Data/CurrentUser.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow() {
        // ... UI 初始化
        
        // 创建聊天区域
        chatArea = new ChatArea(this);
        
        // 连接视频通话请求
        connect(chatArea, &ChatArea::videoCallRequested, 
                this, &MainWindow::onVideoCallRequested);
    }
    
    void onUserLoggedIn(const QString& userId) {
        // 初始化视频通话
        QString serverUrl = getWebRTCServerUrl();  // 从配置读取
        
        if (VideoCallManager::instance()->initialize(serverUrl, userId)) {
            qDebug() << "Video call initialized for user:" << userId;
        } else {
            qWarning() << "Failed to initialize video call";
        }
    }
    
private slots:
    void onVideoCallRequested(const QString& targetUserId) {
        qDebug() << "Starting video call to:" << targetUserId;
        VideoCallManager::instance()->startCall(targetUserId);
    }
    
protected:
    void closeEvent(QCloseEvent* event) override {
        // 清理视频通话
        VideoCallManager::instance()->shutdown();
        QMainWindow::closeEvent(event);
    }
    
private:
    ChatArea* chatArea;
    
    QString getWebRTCServerUrl() {
        // 从 network_config.json 读取配置
        return "ws://localhost:8081/ws/webrtc";
    }
};
```

## 注意事项

### WebRTC 与 Qt emit 冲突

项目中已经处理了 WebRTC 的 `sigslot` 库与 Qt 的 `emit` 宏冲突:

```cpp
// 在包含 WebRTC 头文件前
#ifdef emit
#undef emit
#define QT_NO_EMIT_DEFINED
#endif

#include "WebRTC/call_coordinator.h"

#ifdef QT_NO_EMIT_DEFINED
#define emit
#undef QT_NO_EMIT_DEFINED
#endif
```

### 线程安全

- `CallCoordinator` 的回调可能在 WebRTC 线程中执行
- 已使用 `QMetaObject::invokeMethod` 和 `Qt::QueuedConnection` 确保 UI 更新在主线程

### 资源清理

- 通话结束时,WebRTC 会自动停止摄像头
- 确保在应用退出时调用 `VideoCallManager::shutdown()`

## 测试

### 本地测试

1. 启动信令服务器(端口 8081)
2. 启动两个客户端实例
3. 使用不同的用户ID登录
4. 在其中一个客户端发起视频通话
5. 另一个客户端应该收到来电提示

### 网络测试

1. 配置 STUN/TURN 服务器
2. 在不同网络环境测试连接
3. 验证 NAT 穿透能力

## 故障排除

### 视频无法显示

- 检查摄像头权限
- 确认 WebRTC 初始化成功
- 查看日志输出

### 连接失败

- 检查信令服务器连接
- 确认 ICE 服务器配置
- 查看网络防火墙设置

### 音视频不同步

- 检查系统音频设备
- 调整缓冲区大小
- 升级 WebRTC 版本

## 下一步改进

- [ ] 添加通话时长显示
- [ ] 实现静音和摄像头开关功能
- [ ] 添加通话质量指示器
- [ ] 支持屏幕共享
- [ ] 添加通话录制功能
- [ ] 实现群组视频通话
- [ ] 优化移动端适配

## 参考

- WebRTC官方文档: https://webrtc.org/
- 项目中的 `注意事项.md`: WebRTC 与 Qt 集成的详细说明
- `video_call_window.cc`: 参考的视频通话窗口实现
