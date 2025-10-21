/* 
 * 示例:在主窗口中集成视频通话功能
 * 
 * 这是一个参考示例,展示如何在 MainWindow 或其他管理类中
 * 初始化和使用 VideoCallManager
 */

#include "Manager/VideoCallManager.h"
#include "View/Chat/ChatArea.h"
#include "Data/CurrentUser.h"
#include <QDebug>

// ============================================================================
// 步骤 1: 在用户登录成功后初始化视频通话管理器
// ============================================================================

void onUserLoggedIn(const QString& userId) {
    qDebug() << "User logged in:" << userId;
    
    // 从配置读取信令服务器地址
    // 可以从 res/data/network_config.json 读取
    QString signalServerUrl = "ws://localhost:8081/ws/webrtc";
    
    // 使用用户ID作为客户端ID初始化视频通话管理器
    bool success = VideoCallManager::instance()->initialize(signalServerUrl, userId);
    
    if (success) {
        qDebug() << "Video call manager initialized successfully";
    } else {
        qWarning() << "Failed to initialize video call manager";
    }
}

// ============================================================================
// 步骤 2: 在创建 ChatArea 时连接视频通话信号
// ============================================================================

ChatArea* createChatArea(QWidget* parent) {
    ChatArea* chatArea = new ChatArea(parent);
    
    // 连接视频通话请求信号
    QObject::connect(chatArea, &ChatArea::videoCallRequested, 
                     [](const QString& targetUserId) {
        qDebug() << "Video call requested to user:" << targetUserId;
        
        // 检查是否正在通话中
        if (VideoCallManager::instance()->isInCall()) {
            qWarning() << "Already in a call";
            // 可以显示一个提示
            return;
        }
        
        // 发起视频通话
        VideoCallManager::instance()->startCall(targetUserId);
    });
    
    return chatArea;
}

// ============================================================================
// 步骤 3: 在应用退出时清理
// ============================================================================

void onApplicationExit() {
    qDebug() << "Application exiting, cleaning up video call manager";
    
    // 关闭视频通话管理器
    VideoCallManager::instance()->shutdown();
}

// ============================================================================
// 可选: 监听视频通话事件
// ============================================================================

void setupVideoCallListeners() {
    VideoCallManager* manager = VideoCallManager::instance();
    
    // 监听来电
    QObject::connect(manager, &VideoCallManager::incomingCall, 
                     [](const QString& callerId) {
        qDebug() << "Incoming call from:" << callerId;
        // IncomingCallDialog 会自动显示,这里可以添加额外的处理
        // 例如:播放铃声、显示系统通知等
    });
    
    // 监听通话状态变化
    QObject::connect(manager, &VideoCallManager::callStateChanged,
                     [](CallState state, const QString& peerId) {
        qDebug() << "Call state changed to:" << static_cast<int>(state) 
                 << "with peer:" << peerId;
        
        // 可以在这里更新UI状态
        switch (state) {
            case CallState::Calling:
                qDebug() << "Calling...";
                break;
            case CallState::Connected:
                qDebug() << "Call connected!";
                break;
            case CallState::Idle:
                qDebug() << "Call ended";
                break;
            default:
                break;
        }
    });
    
    // 监听通话结束
    QObject::connect(manager, &VideoCallManager::callEnded,
                     []() {
        qDebug() << "Call ended event received";
        // 可以在这里执行通话结束后的清理工作
    });
}

// ============================================================================
// 完整示例:在 MainWindow 中的使用
// ============================================================================

/*
// 在 MainWindow.h 中添加成员
private:
    ChatArea* chatArea_;
    QString currentUserId_;

// 在 MainWindow.cpp 中实现
MainWindow::MainWindow(QWidget* parent) : QWidget(parent) {
    // ... 其他初始化代码
    
    // 创建聊天区域
    chatArea_ = new ChatArea(this);
    
    // 连接视频通话请求
    connect(chatArea_, &ChatArea::videoCallRequested, 
            this, &MainWindow::onVideoCallRequested);
}

void MainWindow::onLoginSuccess(const QString& userId) {
    currentUserId_ = userId;
    
    // 初始化视频通话
    QString serverUrl = "ws://localhost:8081/ws/webrtc";
    VideoCallManager::instance()->initialize(serverUrl, userId);
    
    // 设置监听器
    setupVideoCallListeners();
}

void MainWindow::onVideoCallRequested(const QString& targetUserId) {
    if (VideoCallManager::instance()->isInCall()) {
        QMessageBox::information(this, "提示", "当前正在通话中");
        return;
    }
    
    VideoCallManager::instance()->startCall(targetUserId);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // 清理视频通话资源
    VideoCallManager::instance()->shutdown();
    QWidget::closeEvent(event);
}
*/

// ============================================================================
// 测试建议
// ============================================================================

/*
1. 启动信令服务器(WebSocket 服务器,端口 8081)

2. 启动两个客户端实例:
   - 客户端A: 使用 userId = "user1" 登录
   - 客户端B: 使用 userId = "user2" 登录

3. 在客户端A中:
   - 打开与 user2 的聊天窗口
   - 点击视频通话按钮
   - 应该看到 VideoCallDialog 显示"呼叫中..."

4. 在客户端B中:
   - 应该弹出 IncomingCallDialog
   - 显示 "user1 正在呼叫您"
   - 点击"接听"按钮

5. 双方应该能看到视频通话界面:
   - 远程视频显示在主窗口
   - 本地视频显示在右下角小窗口
   - 可以点击"挂断"按钮结束通话

6. 故障排除:
   - 如果看不到视频:检查摄像头权限
   - 如果连接失败:检查信令服务器和网络连接
   - 查看控制台日志获取详细信息
*/
