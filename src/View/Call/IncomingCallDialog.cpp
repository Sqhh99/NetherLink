/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include "View/Call/IncomingCallDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

IncomingCallDialog::IncomingCallDialog(const QString& caller_id, QWidget* parent)
    : QDialog(parent),
      caller_id_(caller_id) {
    
    setWindowTitle("来电");
    setModal(true);
    setFixedSize(350, 200);
    
    // 禁止关闭按钮
    setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    
    setupUI();
    
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
}

void IncomingCallDialog::setupUI() {
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(20, 20, 20, 20);
    main_layout->setSpacing(15);
    
    // 呼叫方标签
    caller_label_ = new QLabel(caller_id_, this);
    caller_label_->setAlignment(Qt::AlignCenter);
    caller_label_->setStyleSheet(
        "QLabel {"
        "   font-size: 18px;"
        "   font-weight: bold;"
        "   color: #333333;"
        "}"
    );
    main_layout->addWidget(caller_label_);
    
    // 状态标签
    status_label_ = new QLabel("正在呼叫您...", this);
    status_label_->setAlignment(Qt::AlignCenter);
    status_label_->setStyleSheet(
        "QLabel {"
        "   font-size: 14px;"
        "   color: #666666;"
        "}"
    );
    main_layout->addWidget(status_label_);
    
    main_layout->addSpacing(20);
    
    // 按钮布局
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->setSpacing(20);
    
    // 拒绝按钮
    reject_button_ = new QPushButton("拒绝", this);
    reject_button_->setFixedSize(120, 45);
    reject_button_->setCursor(Qt::PointingHandCursor);
    reject_button_->setStyleSheet(
        "QPushButton {"
        "   background-color: #d9534f;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 22px;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #c9302c;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #ac2925;"
        "}"
    );
    connect(reject_button_, &QPushButton::clicked, this, &IncomingCallDialog::onRejectClicked);
    button_layout->addWidget(reject_button_);
    
    // 接听按钮
    accept_button_ = new QPushButton("接听", this);
    accept_button_->setFixedSize(120, 45);
    accept_button_->setCursor(Qt::PointingHandCursor);
    accept_button_->setStyleSheet(
        "QPushButton {"
        "   background-color: #5cb85c;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 22px;"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "   background-color: #4cae4c;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #449d44;"
        "}"
    );
    connect(accept_button_, &QPushButton::clicked, this, &IncomingCallDialog::onAcceptClicked);
    button_layout->addWidget(accept_button_);
    
    main_layout->addLayout(button_layout);
    main_layout->addStretch();
}

void IncomingCallDialog::onAcceptClicked() {
    qDebug() << "IncomingCallDialog: Call accepted";
    
    if (timeout_timer_) {
        timeout_timer_->stop();
    }
    
    emit accepted();
    accept();
}

void IncomingCallDialog::onRejectClicked() {
    qDebug() << "IncomingCallDialog: Call rejected";
    
    if (timeout_timer_) {
        timeout_timer_->stop();
    }
    
    emit rejected();
    reject();
}

void IncomingCallDialog::onTimeout() {
    qDebug() << "IncomingCallDialog: Call timeout";
    
    // 超时自动拒绝
    emit rejected();
    reject();
}
