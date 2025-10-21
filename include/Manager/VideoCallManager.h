/* guard ------------------------------------------------------------------ 80 // ! ----------------------------- 120 */

#ifndef INCLUDE_MANAGER_VIDEO_CALL_MANAGER
#define INCLUDE_MANAGER_VIDEO_CALL_MANAGER

/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include <QObject>
#include <QJsonArray>
#include <memory>

// Fix Qt emit macro conflict with WebRTC sigslot
#ifdef emit
#undef emit
#define QT_NO_EMIT_DEFINED
#endif

#include "WebRTC/call_coordinator.h"
#include "WebRTC/icall_observer.h"
#include "api/environment/environment.h"

#ifdef QT_NO_EMIT_DEFINED
#define emit
#undef QT_NO_EMIT_DEFINED
#endif

/* forward declarations --------------------------------------------------- 80 // ! ----------------------------- 120 */
class VideoCallDialog;

/* class ------------------------------------------------------------------ 80 // ! ----------------------------- 120 */

/**
 * @brief 视频通话管理器 - 单例类
 * 
 * 负责管理整个应用的视频通话功能:
 * - 管理 CallCoordinator 实例
 * - 管理视频通话窗口的显示和隐藏
 * - 处理来电通知
 * - 提供全局通话接口
 */
class VideoCallManager : public QObject, public ICallUIObserver {
    Q_OBJECT

public:
    /**
     * @brief 获取单例实例
     */
    static VideoCallManager* instance();

    /**
     * @brief 初始化视频通话管理器
     * @param server_url WebSocket信令服务器地址
     * @param client_id 客户端ID(通常使用用户ID)
     * @return true 初始化成功
     */
    bool initialize(const QString& server_url, const QString& client_id);

    /**
     * @brief 关闭管理器
     */
    void shutdown();

    /**
     * @brief 发起视频通话
     * @param target_user_id 目标用户ID
     */
    void startCall(const QString& target_user_id);

    /**
     * @brief 接听来电
     */
    void acceptCall();

    /**
     * @brief 拒绝来电
     * @param reason 拒绝原因
     */
    void rejectCall(const QString& reason = "用户拒绝");

    /**
     * @brief 结束当前通话
     */
    void endCall();

    /**
     * @brief 是否正在通话中
     */
    bool isInCall() const;

    /**
     * @brief 获取当前通话对方ID
     */
    QString getCurrentPeerId() const;

signals:
    /**
     * @brief 收到来电信号
     * @param caller_id 呼叫方ID
     */
    void incomingCall(const QString& caller_id);

    /**
     * @brief 通话状态改变
     * @param state 新的通话状态
     * @param peer_id 对端ID
     */
    void callStateChanged(CallState state, const QString& peer_id);

    /**
     * @brief 通话结束
     */
    void callEnded();

protected:
    // ICallUIObserver 接口实现
    void OnStartLocalRenderer(webrtc::VideoTrackInterface* track) override;
    void OnStopLocalRenderer() override;
    void OnStartRemoteRenderer(webrtc::VideoTrackInterface* track) override;
    void OnStopRemoteRenderer() override;
    void OnLogMessage(const std::string& message, const std::string& level) override;
    void OnShowError(const std::string& title, const std::string& message) override;
    void OnShowInfo(const std::string& title, const std::string& message) override;
    void OnSignalConnected(const std::string& client_id) override;
    void OnSignalDisconnected() override;
    void OnSignalError(const std::string& error) override;
    void OnClientListUpdate(const QJsonArray& clients) override;
    void OnCallStateChanged(CallState state, const std::string& peer_id) override;
    void OnIncomingCall(const std::string& caller_id) override;

private:
    explicit VideoCallManager(QObject* parent = nullptr);
    ~VideoCallManager();
    VideoCallManager(const VideoCallManager&) = delete;
    VideoCallManager& operator=(const VideoCallManager&) = delete;

    void createCallDialog();
    void showCallDialog();
    void hideCallDialog();

private:
    static VideoCallManager* instance_;
    
    std::unique_ptr<CallCoordinator> coordinator_;
    VideoCallDialog* call_dialog_;
    
    bool is_initialized_;
    QString current_peer_id_;
    QString incoming_caller_id_;
};

#endif /* INCLUDE_MANAGER_VIDEO_CALL_MANAGER */
