/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include "View/Call/VideoCallDialog.h"
#include "WebRTC/videorenderer.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QDebug>

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

VideoCallDialog::VideoCallDialog(QWidget* parent)
    : QDialog(parent),
      local_renderer_(nullptr),
      remote_renderer_(nullptr),
      call_state_(CallState::Idle) {
    
    setWindowTitle("视频通话");
    resize(800, 600);
    setModal(false);
    
    setupUI();
}

VideoCallDialog::~VideoCallDialog() {
    if (local_renderer_) {
        local_renderer_->Stop();
    }
    if (remote_renderer_) {
        remote_renderer_->Stop();
    }
}

void VideoCallDialog::setPeerName(const QString& name) {
    peer_name_ = name;
    peer_name_label_->setText(name);
}

void VideoCallDialog::setLocalVideoTrack(webrtc::VideoTrackInterface* track) {
    qDebug() << "VideoCallDialog: Setting local video track";
    
    if (!local_renderer_) {
        local_renderer_ = new VideoRenderer(video_container_);
        local_renderer_->setFixedSize(200, 150);
        local_renderer_->raise();
        local_renderer_->show();
    }
    
    local_renderer_->SetVideoTrack(track);
    layoutLocalVideo();
}

void VideoCallDialog::setRemoteVideoTrack(webrtc::VideoTrackInterface* track) {
    qDebug() << "VideoCallDialog: Setting remote video track";
    
    if (remote_renderer_) {
        remote_renderer_->SetVideoTrack(track);
        remote_renderer_->show();
        call_status_label_->hide();
    }
}

void VideoCallDialog::stopLocalVideo() {
    qDebug() << "VideoCallDialog: Stopping local video";
    
    if (local_renderer_) {
        local_renderer_->Stop();
        local_renderer_->hide();
    }
}

void VideoCallDialog::stopRemoteVideo() {
    qDebug() << "VideoCallDialog: Stopping remote video";
    
    if (remote_renderer_) {
        remote_renderer_->Stop();
        remote_renderer_->hide();
        call_status_label_->show();
    }
}

void VideoCallDialog::updateCallState(CallState state, const QString& peer_id) {
    call_state_ = state;
    call_status_label_->setText(getCallStateString(state));
    
    // 如果连接成功,隐藏状态标签,显示视频
    if (state == CallState::Connected && remote_renderer_) {
        call_status_label_->hide();
        remote_renderer_->show();
    }
}

void VideoCallDialog::setupUI() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    
    // 顶部信息栏
    QWidget* info_bar = new QWidget(this);
    info_bar->setStyleSheet("QWidget { background-color: #2c2c2c; }");
    info_bar->setFixedHeight(50);
    
    QHBoxLayout* info_layout = new QHBoxLayout(info_bar);
    info_layout->setContentsMargins(10, 0, 10, 0);
    
    peer_name_label_ = new QLabel("对方", info_bar);
    peer_name_label_->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; }");
    info_layout->addWidget(peer_name_label_);
    
    call_time_label_ = new QLabel("00:00", info_bar);
    call_time_label_->setStyleSheet("QLabel { color: white; font-size: 14px; }");
    info_layout->addWidget(call_time_label_);
    
    info_layout->addStretch();
    
    main_layout->addWidget(info_bar);
    
    // 视频容器
    video_container_ = new QWidget(this);
    video_container_->setStyleSheet("QWidget { background-color: black; }");
    video_container_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QVBoxLayout* video_layout = new QVBoxLayout(video_container_);
    video_layout->setContentsMargins(0, 0, 0, 0);
    video_layout->setSpacing(0);
    
    // 远程视频渲染器
    remote_renderer_ = new VideoRenderer(video_container_);
    remote_renderer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    video_layout->addWidget(remote_renderer_);
    remote_renderer_->hide();
    
    // 通话状态标签
    call_status_label_ = new QLabel("连接中...", video_container_);
    call_status_label_->setAlignment(Qt::AlignCenter);
    call_status_label_->setStyleSheet("QLabel { color: white; font-size: 18px; }");
    video_layout->addWidget(call_status_label_);
    
    main_layout->addWidget(video_container_);
    
    // 底部控制栏
    QWidget* control_bar = new QWidget(this);
    control_bar->setStyleSheet("QWidget { background-color: #2c2c2c; }");
    control_bar->setFixedHeight(80);
    
    QHBoxLayout* control_layout = new QHBoxLayout(control_bar);
    control_layout->setContentsMargins(20, 10, 20, 10);
    control_layout->setSpacing(15);
    
    control_layout->addStretch();
    
    // 静音按钮
    mute_button_ = new QPushButton("静音", control_bar);
    mute_button_->setFixedSize(80, 60);
    mute_button_->setStyleSheet(
        "QPushButton {"
        "   background-color: #4a4a4a;"
        "   color: white;"
        "   border-radius: 30px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5a5a5a;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3a3a3a;"
        "}"
    );
    control_layout->addWidget(mute_button_);
    
    // 挂断按钮
    hangup_button_ = new QPushButton("挂断", control_bar);
    hangup_button_->setFixedSize(80, 60);
    hangup_button_->setStyleSheet(
        "QPushButton {"
        "   background-color: #d9534f;"
        "   color: white;"
        "   border-radius: 30px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c9302c;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #ac2925;"
        "}"
    );
    connect(hangup_button_, &QPushButton::clicked, this, &VideoCallDialog::hangupClicked);
    control_layout->addWidget(hangup_button_);
    
    // 摄像头按钮
    video_button_ = new QPushButton("摄像头", control_bar);
    video_button_->setFixedSize(80, 60);
    video_button_->setStyleSheet(
        "QPushButton {"
        "   background-color: #4a4a4a;"
        "   color: white;"
        "   border-radius: 30px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #5a5a5a;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #3a3a3a;"
        "}"
    );
    control_layout->addWidget(video_button_);
    
    control_layout->addStretch();
    
    main_layout->addWidget(control_bar);
}

void VideoCallDialog::layoutLocalVideo() {
    if (!local_renderer_ || !video_container_) {
        return;
    }
    
    // 将本地视频放在右下角
    int margin = 10;
    int x = video_container_->width() - local_renderer_->width() - margin;
    int y = video_container_->height() - local_renderer_->height() - margin;
    
    local_renderer_->move(x, y);
    local_renderer_->raise();
}

QString VideoCallDialog::getCallStateString(CallState state) const {
    switch (state) {
        case CallState::Idle:
            return "空闲";
        case CallState::Calling:
            return "呼叫中...";
        case CallState::Receiving:
            return "来电中...";
        case CallState::Connecting:
            return "连接中...";
        case CallState::Connected:
            return "通话中";
        case CallState::Ending:
            return "结束中...";
        default:
            return "未知状态";
    }
}

void VideoCallDialog::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    layoutLocalVideo();
}

void VideoCallDialog::closeEvent(QCloseEvent* event) {
    // 点击关闭按钮时发射挂断信号
    emit hangupClicked();
    event->accept();
}
