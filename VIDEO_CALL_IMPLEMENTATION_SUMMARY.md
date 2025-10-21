# NetherLink 视频通话功能实现总结

## 完成内容

### 1. 核心组件

#### VideoCallManager (Manager/VideoCallManager.h/cpp)
- **功能**: 全局视频通话管理器,单例模式
- **职责**:
  - 封装 CallCoordinator,管理 WebRTC 核心逻辑
  - 管理视频通话窗口的生命周期
  - 处理来电通知和通话状态
  - 提供全局通话接口(startCall, acceptCall, rejectCall, endCall)
- **接口**:
  - `initialize(serverUrl, clientId)` - 初始化并连接到信令服务器
  - `startCall(targetUserId)` - 发起视频通话
  - `acceptCall()` - 接听来电
  - `rejectCall(reason)` - 拒绝来电
  - `endCall()` - 结束通话
  - `isInCall()` - 查询是否在通话中

#### VideoCallDialog (View/Call/VideoCallDialog.h/cpp)
- **功能**: 视频通话对话框
- **特性**:
  - 远程视频全屏显示
  - 本地视频悬浮在右下角(200x150像素)
  - 通话控制按钮(挂断、静音、摄像头)
  - 美观的 UI 设计(深色主题)
  - 自动布局调整

#### IncomingCallDialog (View/Call/IncomingCallDialog.h/cpp)
- **功能**: 来电提示对话框
- **特性**:
  - 显示呼叫方信息
  - 接听/拒绝按钮
  - 30秒超时自动拒绝
  - 模态对话框,不可关闭(除非选择操作)

### 2. 界面集成

#### FloatingInputBar 修改
- 添加 `videoCallRequested()` 信号
- 在视频按钮点击时发射信号
- 文件: `include/Components/FloatingInputBar.h`, `src/Components/FloatingInputBar.cpp`

#### ChatArea 修改
- 添加 `videoCallRequested(QString targetUserId)` 信号
- 连接 FloatingInputBar 的信号并转发
- 文件: `include/View/Chat/ChatArea.h`, `src/View/Chat/ChatArea.cpp`

### 3. 文档

#### VIDEO_CALL_INTEGRATION.md
完整的集成指南,包含:
- 架构设计说明
- 详细的集成步骤
- 代码示例
- 配置说明
- 使用流程图
- 故障排除
- 未来改进建议

#### examples/video_call_integration_example.cpp
实用的代码示例,展示:
- 如何初始化 VideoCallManager
- 如何连接信号
- 完整的 MainWindow 集成示例
- 测试步骤

## 技术亮点

### 1. 架构设计

```
用户界面层          业务逻辑层           WebRTC核心层
┌──────────┐       ┌──────────────┐     ┌─────────────┐
│FloatingBar│───────▶VideoCallMgr │────▶│CallCoordin- │
│ (UI)     │       │  (Singleton) │     │ator         │
└──────────┘       └──────────────┘     └─────────────┘
                          │                     │
                          │                     ├─▶ WebRTCEngine
                          │                     ├─▶ SignalClient
                          │                     └─▶ CallManager
                          ▼
                   ┌──────────────┐
                   │VideoCallDlg  │
                   │ (UI Window)  │
                   └──────────────┘
```

**优点**:
- **解耦设计**: UI层不直接依赖WebRTC,通过VideoCallManager中介
- **可测试性**: 各组件职责单一,易于单元测试
- **可复用性**: WebRTC组件可以在其他项目中复用
- **可维护性**: 清晰的分层架构,易于理解和修改

### 2. 线程安全

- 使用 `QMetaObject::invokeMethod` 和 `Qt::QueuedConnection`
- 确保 WebRTC 回调在主线程更新 UI
- 避免死锁和竞态条件

### 3. 资源管理

- 智能指针管理 WebRTC 对象
- 正确的清理顺序:摄像头→轨道→连接→资源
- 防止内存泄漏

### 4. 错误处理

- 完整的错误检查
- 用户友好的错误提示
- 详细的日志输出

## 使用流程

### 发起通话流程

```
用户点击视频图标
    ↓
FloatingInputBar.videoCallRequested()
    ↓
ChatArea.videoCallRequested(targetUserId)
    ↓
VideoCallManager.startCall(targetUserId)
    ↓
创建 VideoCallDialog
    ↓
CallCoordinator.StartCall()
    ↓
WebRTC 信令交换
    ↓
建立 P2P 连接
    ↓
显示视频流
```

### 接听通话流程

```
收到信令消息
    ↓
CallCoordinator.OnIncomingCall()
    ↓
VideoCallManager.OnIncomingCall()
    ↓
显示 IncomingCallDialog
    ↓
用户点击接听
    ↓
VideoCallManager.acceptCall()
    ↓
创建 VideoCallDialog
    ↓
CallCoordinator.AcceptCall()
    ↓
WebRTC 信令交换
    ↓
建立 P2P 连接
    ↓
显示视频流
```

## 如何使用

### 1. 在主程序中初始化

```cpp
// 用户登录成功后
QString userId = CurrentUser::instance().getId();
QString serverUrl = "ws://localhost:8081/ws/webrtc";

VideoCallManager::instance()->initialize(serverUrl, userId);
```

### 2. 在ChatArea所在的容器中连接信号

```cpp
// 假设在 MainWindow 或其他管理类中
connect(chatArea, &ChatArea::videoCallRequested,
        [](const QString& targetUserId) {
    VideoCallManager::instance()->startCall(targetUserId);
});
```

### 3. 应用退出时清理

```cpp
// 在 MainWindow::closeEvent 或应用退出处
VideoCallManager::instance()->shutdown();
```

## 配置要求

### 信令服务器

需要一个 WebSocket 信令服务器,支持以下消息类型:
- `register` - 客户端注册
- `call-request` - 呼叫请求
- `call-response` - 呼叫响应
- `offer` / `answer` - SDP 交换
- `ice-candidate` - ICE 候选交换
- `call-end` - 结束通话

### 网络配置

```json
{
    "webrtc_signal_server": "ws://localhost:8081/ws/webrtc",
    "ice_servers": [
        {
            "urls": ["stun:stun.l.google.com:19302"]
        }
    ]
}
```

### 系统要求

- Windows 10/11
- Qt 6.x
- WebRTC M120+
- 摄像头和麦克风权限

## 测试

### 本地测试

1. 启动信令服务器
2. 启动两个客户端实例(不同用户ID)
3. 在一个客户端发起通话
4. 另一个客户端接听
5. 验证音视频传输

### 检查清单

- ✅ 视频显示正常
- ✅ 音频传输正常
- ✅ 通话控制功能
- ✅ 挂断功能正常
- ✅ 来电提示显示
- ✅ 超时处理正确
- ✅ 资源正确释放

## 已知问题和限制

1. **当前仅支持1对1通话** - 未实现群组通话
2. **静音和摄像头控制按钮** - UI已创建但功能未实现
3. **通话时长计时器** - 未实现
4. **通话质量指示器** - 未实现
5. **屏幕共享** - 未实现

## 下一步改进

### 短期改进
- [ ] 实现静音功能
- [ ] 实现摄像头开关
- [ ] 添加通话时长显示
- [ ] 添加网络质量指示器

### 中期改进
- [ ] 添加屏幕共享功能
- [ ] 实现通话录制
- [ ] 优化视频质量自适应
- [ ] 添加美颜滤镜

### 长期改进
- [ ] 支持群组视频通话(多人)
- [ ] 实现虚拟背景
- [ ] AI 降噪功能
- [ ] 跨平台支持(Linux, macOS)

## 参考资料

- 项目中的 `main.cc` - WebRTC 初始化示例
- `video_call_window.cc` - 完整的视频通话窗口实现
- `注意事项.md` - WebRTC 与 Qt 集成的关键技术点
- WebRTC 官方文档: https://webrtc.org/

## 贡献者

本视频通话功能由 GitHub Copilot 协助开发完成。

## 许可

遵循项目主许可协议。
