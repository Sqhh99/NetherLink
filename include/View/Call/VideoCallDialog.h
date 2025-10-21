/* guard ------------------------------------------------------------------ 80 // ! ----------------------------- 120 */

#ifndef INCLUDE_VIEW_CALL_VIDEO_CALL_DIALOG
#define INCLUDE_VIEW_CALL_VIDEO_CALL_DIALOG

/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <memory>

// Fix Qt emit macro conflict with WebRTC sigslot
#ifdef emit
#undef emit
#define QT_NO_EMIT_DEFINED
#endif

#include "api/media_stream_interface.h"
#include "WebRTC/callmanager.h"

#ifdef QT_NO_EMIT_DEFINED
#define emit
#undef QT_NO_EMIT_DEFINED
#endif

/* forward declarations --------------------------------------------------- 80 // ! ----------------------------- 120 */
class VideoRenderer;

/* class ------------------------------------------------------------------ 80 // ! ----------------------------- 120 */

/**
 * @brief 视频通话对话框
 * 
 * 显示视频通话界面:
 * - 远程视频(主窗口)
 * - 本地视频(小窗口,悬浮在右下角)
 * - 通话控制按钮(挂断、静音等)
 * - 通话状态信息
 */
class VideoCallDialog : public QDialog {
    Q_OBJECT

public:
    explicit VideoCallDialog(QWidget* parent = nullptr);
    ~VideoCallDialog();

    /**
     * @brief 设置对端用户名称
     */
    void setPeerName(const QString& name);

    /**
     * @brief 设置本地视频轨道
     */
    void setLocalVideoTrack(webrtc::VideoTrackInterface* track);

    /**
     * @brief 设置远程视频轨道
     */
    void setRemoteVideoTrack(webrtc::VideoTrackInterface* track);

    /**
     * @brief 停止本地视频
     */
    void stopLocalVideo();

    /**
     * @brief 停止远程视频
     */
    void stopRemoteVideo();

    /**
     * @brief 更新通话状态
     */
    void updateCallState(CallState state, const QString& peer_id);

signals:
    /**
     * @brief 点击挂断按钮
     */
    void hangupClicked();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUI();
    void layoutLocalVideo();
    QString getCallStateString(CallState state) const;

private:
    // UI 组件
    QWidget* video_container_;
    VideoRenderer* local_renderer_;
    VideoRenderer* remote_renderer_;
    QLabel* call_status_label_;
    QLabel* peer_name_label_;
    QLabel* call_time_label_;
    QPushButton* hangup_button_;
    QPushButton* mute_button_;
    QPushButton* video_button_;
    
    // 通话信息
    QString peer_name_;
    CallState call_state_;
};

#endif /* INCLUDE_VIEW_CALL_VIDEO_CALL_DIALOG */
