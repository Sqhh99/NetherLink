/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include "Manager/VideoCallManager.h"
#include "View/Call/VideoCallDialog.h"
#include "View/Call/IncomingCallDialog.h"

// Fix Qt emit macro conflict with WebRTC sigslot
#ifdef emit
#undef emit
#define QT_NO_EMIT_DEFINED
#endif

#include "api/environment/environment_factory.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/win32_socket_init.h"

#ifdef QT_NO_EMIT_DEFINED
#define emit
#undef QT_NO_EMIT_DEFINED
#endif

#include <QMessageBox>
#include <QDebug>

/* static member ---------------------------------------------------------- 80 // ! ----------------------------- 120 */

VideoCallManager* VideoCallManager::instance_ = nullptr;

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

VideoCallManager* VideoCallManager::instance() {
    if (!instance_) {
        instance_ = new VideoCallManager();
    }
    return instance_;
}

VideoCallManager::VideoCallManager(QObject* parent)
    : QObject(parent),
      call_dialog_(nullptr),
      is_initialized_(false) {
    
    qDebug() << "VideoCallManager: Initializing...";
    
    // 初始化 WebRTC 环境(静态初始化,仅一次)
    static webrtc::WinsockInitializer winsock_init;
    static bool ssl_initialized = false;
    if (!ssl_initialized) {
        webrtc::InitializeSSL();
        ssl_initialized = true;
    }
}

VideoCallManager::~VideoCallManager() {
    shutdown();
}

bool VideoCallManager::initialize(const QString& server_url, const QString& client_id) {
    if (is_initialized_) {
        qWarning() << "VideoCallManager: Already initialized";
        return true;
    }
    
    qDebug() << "VideoCallManager: Initializing with server:" << server_url << "client:" << client_id;
    
    // 创建 WebRTC 环境
    webrtc::Environment env = webrtc::CreateEnvironment();
    
    // 创建协调器
    coordinator_ = std::make_unique<CallCoordinator>(env);
    coordinator_->SetUIObserver(this);
    
    if (!coordinator_->Initialize()) {
        qCritical() << "VideoCallManager: Failed to initialize CallCoordinator";
        return false;
    }
    
    // 连接到信令服务器
    coordinator_->ConnectToSignalServer(server_url.toStdString(), client_id.toStdString());
    
    is_initialized_ = true;
    
    qDebug() << "VideoCallManager: Initialized successfully";
    return true;
}

void VideoCallManager::shutdown() {
    if (!is_initialized_) {
        return;
    }
    
    qDebug() << "VideoCallManager: Shutting down...";
    
    // 关闭通话窗口
    hideCallDialog();
    if (call_dialog_) {
        call_dialog_->deleteLater();
        call_dialog_ = nullptr;
    }
    
    // 关闭协调器
    if (coordinator_) {
        coordinator_->Shutdown();
        coordinator_.reset();
    }
    
    is_initialized_ = false;
    qDebug() << "VideoCallManager: Shutdown complete";
}

void VideoCallManager::startCall(const QString& target_user_id) {
    if (!is_initialized_) {
        qWarning() << "VideoCallManager: Not initialized, cannot start call";
        QMessageBox::warning(nullptr, "错误", "视频通话功能未初始化");
        return;
    }
    
    if (isInCall()) {
        qWarning() << "VideoCallManager: Already in a call";
        QMessageBox::information(nullptr, "提示", "当前正在通话中");
        return;
    }
    
    qDebug() << "VideoCallManager: Starting call to" << target_user_id;
    
    current_peer_id_ = target_user_id;
    
    // 创建并显示通话窗口
    createCallDialog();
    showCallDialog();
    
    // 发起通话
    coordinator_->StartCall(target_user_id.toStdString());
}

void VideoCallManager::acceptCall() {
    if (!is_initialized_ || !coordinator_) {
        return;
    }
    
    qDebug() << "VideoCallManager: Accepting call from" << incoming_caller_id_;
    
    current_peer_id_ = incoming_caller_id_;
    
    // 创建并显示通话窗口
    createCallDialog();
    showCallDialog();
    
    // 接听通话
    coordinator_->AcceptCall();
}

void VideoCallManager::rejectCall(const QString& reason) {
    if (!is_initialized_ || !coordinator_) {
        return;
    }
    
    qDebug() << "VideoCallManager: Rejecting call from" << incoming_caller_id_ << "reason:" << reason;
    
    coordinator_->RejectCall(reason.toStdString());
    incoming_caller_id_.clear();
}

void VideoCallManager::endCall() {
    if (!is_initialized_ || !coordinator_) {
        return;
    }
    
    qDebug() << "VideoCallManager: Ending call";
    
    coordinator_->EndCall();
    
    // 隐藏通话窗口
    hideCallDialog();
    
    current_peer_id_.clear();
    
    emit callEnded();
}

bool VideoCallManager::isInCall() const {
    return coordinator_ && coordinator_->IsInCall();
}

QString VideoCallManager::getCurrentPeerId() const {
    return current_peer_id_;
}

// ============================================================================
// ICallUIObserver 接口实现
// ============================================================================

void VideoCallManager::OnStartLocalRenderer(webrtc::VideoTrackInterface* track) {
    qDebug() << "VideoCallManager: Start local renderer";
    if (call_dialog_) {
        call_dialog_->setLocalVideoTrack(track);
    }
}

void VideoCallManager::OnStopLocalRenderer() {
    qDebug() << "VideoCallManager: Stop local renderer";
    if (call_dialog_) {
        call_dialog_->stopLocalVideo();
    }
}

void VideoCallManager::OnStartRemoteRenderer(webrtc::VideoTrackInterface* track) {
    qDebug() << "VideoCallManager: Start remote renderer";
    if (call_dialog_) {
        call_dialog_->setRemoteVideoTrack(track);
    }
}

void VideoCallManager::OnStopRemoteRenderer() {
    qDebug() << "VideoCallManager: Stop remote renderer";
    if (call_dialog_) {
        call_dialog_->stopRemoteVideo();
    }
}

void VideoCallManager::OnLogMessage(const std::string& message, const std::string& level) {
    QString qmsg = QString::fromStdString(message);
    QString qlevel = QString::fromStdString(level);
    
    if (qlevel == "error") {
        qWarning() << "VideoCallManager:" << qmsg;
    } else {
        qDebug() << "VideoCallManager:" << qmsg;
    }
}

void VideoCallManager::OnShowError(const std::string& title, const std::string& message) {
    QMessageBox::critical(nullptr, 
                         QString::fromStdString(title), 
                         QString::fromStdString(message));
}

void VideoCallManager::OnShowInfo(const std::string& title, const std::string& message) {
    QMessageBox::information(nullptr, 
                            QString::fromStdString(title), 
                            QString::fromStdString(message));
}

void VideoCallManager::OnSignalConnected(const std::string& client_id) {
    qDebug() << "VideoCallManager: Signal connected, client ID:" << QString::fromStdString(client_id);
}

void VideoCallManager::OnSignalDisconnected() {
    qDebug() << "VideoCallManager: Signal disconnected";
}

void VideoCallManager::OnSignalError(const std::string& error) {
    qWarning() << "VideoCallManager: Signal error:" << QString::fromStdString(error);
}

void VideoCallManager::OnClientListUpdate(const QJsonArray& clients) {
    qDebug() << "VideoCallManager: Client list updated, count:" << clients.size();
}

void VideoCallManager::OnCallStateChanged(CallState state, const std::string& peer_id) {
    QString qpeer_id = QString::fromStdString(peer_id);
    qDebug() << "VideoCallManager: Call state changed to" << static_cast<int>(state) << "peer:" << qpeer_id;
    
    emit callStateChanged(state, qpeer_id);
    
    // 更新通话窗口状态
    if (call_dialog_) {
        call_dialog_->updateCallState(state, qpeer_id);
    }
    
    // 如果通话结束,隐藏窗口
    if (state == CallState::Idle) {
        hideCallDialog();
        current_peer_id_.clear();
        emit callEnded();
    }
}

void VideoCallManager::OnIncomingCall(const std::string& caller_id) {
    QString qcaller_id = QString::fromStdString(caller_id);
    qDebug() << "VideoCallManager: Incoming call from" << qcaller_id;
    
    incoming_caller_id_ = qcaller_id;
    
    // 发射信号
    emit incomingCall(qcaller_id);
    
    // 显示来电对话框
    IncomingCallDialog* dialog = new IncomingCallDialog(qcaller_id, nullptr);
    
    connect(dialog, &IncomingCallDialog::accepted, this, [this]() {
        acceptCall();
    });
    
    connect(dialog, &IncomingCallDialog::rejected, this, [this]() {
        rejectCall("用户拒绝");
    });
    
    dialog->exec();
    dialog->deleteLater();
}

// ============================================================================
// 私有方法
// ============================================================================

void VideoCallManager::createCallDialog() {
    if (call_dialog_) {
        return;
    }
    
    qDebug() << "VideoCallManager: Creating call dialog";
    
    call_dialog_ = new VideoCallDialog(nullptr);
    call_dialog_->setWindowTitle("视频通话");
    call_dialog_->setPeerName(current_peer_id_);
    
    // 连接挂断信号
    connect(call_dialog_, &VideoCallDialog::hangupClicked, this, &VideoCallManager::endCall);
}

void VideoCallManager::showCallDialog() {
    if (!call_dialog_) {
        createCallDialog();
    }
    
    qDebug() << "VideoCallManager: Showing call dialog";
    call_dialog_->show();
    call_dialog_->raise();
    call_dialog_->activateWindow();
}

void VideoCallManager::hideCallDialog() {
    if (call_dialog_) {
        qDebug() << "VideoCallManager: Hiding call dialog";
        call_dialog_->hide();
    }
}
