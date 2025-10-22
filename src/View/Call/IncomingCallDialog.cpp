/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include "View/Call/IncomingCallDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QSize>

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

IncomingCallDialog::IncomingCallDialog(const QString& caller_id, QWidget* parent)
    : QDialog(parent),
      caller_id_(caller_id),
      icon_label_(nullptr),
      blink_state_(false) {
    
    setWindowTitle("来电");
    setModal(true);
    setFixedSize(400, 280);
    
    // 禁止关闭按钮
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    
    // 设置对话框样式 - 现代化深色主题
    setStyleSheet(
        "QDialog { "
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #2d3436, stop:1 #1a1a1a);"
        "   border: 2px solid #4a4a4a;"
        "   border-radius: 15px;"
        "}"
    );
    
    setupUI();
    startBlinkAnimation();
    
    // 创建超时定时器
    timeout_timer_ = new QTimer(this);
    timeout_timer_->setSingleShot(true);
    timeout_timer_->setInterval(RING_TIMEOUT_MS);
    connect(timeout_timer_, &QTimer::timeout, this, &IncomingCallDialog::onTimeout);
    timeout_timer_->start();
}

IncomingCallDialog::~IncomingCallDialog() {
    if (timeout_timer_) {
        timeout_timer_->stop();
    }
    if (blink_timer_) {
        blink_timer_->stop();
    }
}

void IncomingCallDialog::setupUI() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(30, 30, 30, 30);
    main_layout->setSpacing(20);
    
    // 顶部图标区域
    icon_label_ = new QLabel(this);
    icon_label_->setAlignment(Qt::AlignCenter);
    icon_label_->setPixmap(QIcon(":/icon/video.png").pixmap(48, 48));
    icon_label_->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(76, 175, 80, 0.15);"
        "   border-radius: 40px;"
        "   padding: 15px;"
        "   border: 2px solid rgba(76, 175, 80, 0.4);"
        "}"
    );
    icon_label_->setFixedSize(80, 80);
    
    QHBoxLayout* icon_layout = new QHBoxLayout();
    icon_layout->addStretch();
    icon_layout->addWidget(icon_label_);
    icon_layout->addStretch();
    main_layout->addLayout(icon_layout);
    
    main_layout->addSpacing(10);
    
    // 呼叫方标签 - 更醒目的设计
    caller_label_ = new QLabel(caller_id_, this);
    caller_label_->setAlignment(Qt::AlignCenter);
    caller_label_->setStyleSheet(
        "QLabel {"
        "   font-size: 22px;"
        "   font-weight: bold;"
        "   color: #ffffff;"
        "   padding: 8px 20px;"
        "   background-color: rgba(255, 255, 255, 0.08);"
        "   border-radius: 8px;"
        "}"
    );
    main_layout->addWidget(caller_label_);
    
    // 状态标签 - 动画效果提示
    status_label_ = new QLabel("正在呼叫您...", this);
    status_label_->setAlignment(Qt::AlignCenter);
    status_label_->setStyleSheet(
        "QLabel {"
        "   font-size: 15px;"
        "   color: #4CAF50;"
        "   font-weight: 500;"
        "}"
    );
    main_layout->addWidget(status_label_);
    
    main_layout->addSpacing(15);
    
    // 按钮布局
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->setSpacing(30);
    
    // 拒绝按钮 - 使用图标
    reject_button_ = new QPushButton(this);
    reject_button_->setFixedSize(75, 75);
    reject_button_->setCursor(Qt::PointingHandCursor);
    reject_button_->setIcon(QIcon(":/icon/close.png"));
    reject_button_->setIconSize(QSize(35, 35));
    reject_button_->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #e74c3c, stop:1 #c0392b);"
        "   border: 3px solid #a93226;"
        "   border-radius: 37px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #ec7063, stop:1 #e74c3c);"
        "   border: 3px solid #cb4335;"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #c0392b, stop:1 #a93226);"
        "}"
    );
    connect(reject_button_, &QPushButton::clicked, this, &IncomingCallDialog::onRejectClicked);
    
    // 添加拒绝按钮提示文字
    QVBoxLayout* reject_layout = new QVBoxLayout();
    reject_layout->setSpacing(8);
    reject_layout->addWidget(reject_button_, 0, Qt::AlignCenter);
    QLabel* reject_label = new QLabel("拒绝", this);
    reject_label->setAlignment(Qt::AlignCenter);
    reject_label->setStyleSheet("QLabel { color: #e74c3c; font-size: 13px; font-weight: 600; }");
    reject_layout->addWidget(reject_label);
    
    button_layout->addLayout(reject_layout);
    
    button_layout->addSpacing(20);
    
    // 接听按钮 - 使用图标
    accept_button_ = new QPushButton(this);
    accept_button_->setFixedSize(75, 75);
    accept_button_->setCursor(Qt::PointingHandCursor);
    accept_button_->setIcon(QIcon(":/icon/answer.png"));
    accept_button_->setIconSize(QSize(35, 35));
    accept_button_->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #27ae60, stop:1 #229954);"
        "   border: 3px solid #1e8449;"
        "   border-radius: 37px;"
        "}"
        "QPushButton:hover {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #2ecc71, stop:1 #27ae60);"
        "   border: 3px solid #229954;"
        "}"
        "QPushButton:pressed {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #229954, stop:1 #1e8449);"
        "}"
    );
    connect(accept_button_, &QPushButton::clicked, this, &IncomingCallDialog::onAcceptClicked);
    
    // 添加接听按钮提示文字
    QVBoxLayout* accept_layout = new QVBoxLayout();
    accept_layout->setSpacing(8);
    accept_layout->addWidget(accept_button_, 0, Qt::AlignCenter);
    QLabel* accept_label = new QLabel("接听", this);
    accept_label->setAlignment(Qt::AlignCenter);
    accept_label->setStyleSheet("QLabel { color: #27ae60; font-size: 13px; font-weight: 600; }");
    accept_layout->addWidget(accept_label);
    
    button_layout->addLayout(accept_layout);
    
    QHBoxLayout* centered_button_layout = new QHBoxLayout();
    centered_button_layout->addStretch();
    centered_button_layout->addLayout(button_layout);
    centered_button_layout->addStretch();
    
    main_layout->addLayout(centered_button_layout);
    main_layout->addStretch();
}

void IncomingCallDialog::onAcceptClicked() {
    qDebug() << "IncomingCallDialog: Call accepted";
    
    if (timeout_timer_) {
        timeout_timer_->stop();
    }
    if (blink_timer_) {
        blink_timer_->stop();
    }
    
    emit accepted();
    accept();
}

void IncomingCallDialog::onRejectClicked() {
    qDebug() << "IncomingCallDialog: Call rejected";
    
    if (timeout_timer_) {
        timeout_timer_->stop();
    }
    if (blink_timer_) {
        blink_timer_->stop();
    }
    
    emit rejected();
    reject();
}

void IncomingCallDialog::onTimeout() {
    qDebug() << "IncomingCallDialog: Call timeout";
    
    if (blink_timer_) {
        blink_timer_->stop();
    }
    
    // 超时自动拒绝
    emit rejected();
    reject();
}

void IncomingCallDialog::startBlinkAnimation() {
    // 创建闪烁定时器
    blink_timer_ = new QTimer(this);
    connect(blink_timer_, &QTimer::timeout, this, &IncomingCallDialog::updateBlinkAnimation);
    blink_timer_->start(600);  // 每600ms切换一次
}

void IncomingCallDialog::updateBlinkAnimation() {
    if (!icon_label_) {
        return;
    }
    
    blink_state_ = !blink_state_;
    
    QString style_bright = "QLabel { background-color: rgba(76, 175, 80, 0.3); border-radius: 40px; padding: 15px; border: 3px solid rgba(76, 175, 80, 0.8); }";
    QString style_dim = "QLabel { background-color: rgba(76, 175, 80, 0.15); border-radius: 40px; padding: 15px; border: 2px solid rgba(76, 175, 80, 0.4); }";
    
    if (blink_state_) {
        icon_label_->setStyleSheet(style_bright);
    } else {
        icon_label_->setStyleSheet(style_dim);
    }
}

