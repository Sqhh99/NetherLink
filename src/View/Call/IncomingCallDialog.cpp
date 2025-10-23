/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include "View/Call/IncomingCallDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QSize>
#include <QApplication>
#include <QScreen>

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

IncomingCallDialog::IncomingCallDialog(const QString& caller_id, QWidget* parent)
    : FramelessWindow(parent),
      caller_id_(caller_id),
      title_bar_(nullptr),
      btn_close_(nullptr),
      icon_label_(nullptr),
      blink_state_(false) {
    
    setWindowTitle("来电");
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    
    // 禁用透明背景，使用实心背景
    setAttribute(Qt::WA_TranslucentBackground, false);
    setAutoFillBackground(true);
    
    // 设置窗口背景色 - 与 VideoCallDialog 相同的方式
    setStyleSheet("IncomingCallDialog { background-color: #1a1a1a; }");
    
    // 设置窗口大小 - 增大以容纳所有组件
    setFixedSize(520, 500);
    
    // 将窗口移到屏幕中央
    auto screen = QApplication::primaryScreen();
    if (screen) {
        QRect geometry = screen->geometry();
        move(geometry.center().x() - width() / 2,
             geometry.center().y() - height() / 2);
    }
    
    setupTitleBar();
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

void IncomingCallDialog::setupTitleBar() {
    // 创建标题栏
    title_bar_ = new QWidget(this);
    title_bar_->setFixedHeight(50);
    title_bar_->setStyleSheet(
        "QWidget#title_bar { "
        "   background-color: #1a1a1a;"
        "}"
    );
    title_bar_->setObjectName("title_bar");
    
    QHBoxLayout* title_layout = new QHBoxLayout(title_bar_);
    title_layout->setContentsMargins(15, 0, 15, 0);
    
    // 标题栏只显示关闭按钮，不显示文字
    title_layout->addStretch();
    
    // 关闭按钮
    btn_close_ = new QPushButton(title_bar_);
    btn_close_->setFixedSize(36, 36);
    btn_close_->setIcon(QIcon(":/icon/close.png"));
    btn_close_->setIconSize(QSize(18, 18));
    btn_close_->setCursor(Qt::PointingHandCursor);
    btn_close_->setStyleSheet(
        "QPushButton {"
        "   background-color: transparent;"
        "   border: none;"
        "   border-radius: 18px;"
        "}"
        "QPushButton:hover {"
        "   background-color: rgba(255, 0, 0, 0.3);"
        "}"
        "QPushButton:pressed {"
        "   background-color: rgba(255, 0, 0, 0.5);"
        "}"
    );
    connect(btn_close_, &QPushButton::clicked, this, &IncomingCallDialog::onRejectClicked);
    title_layout->addWidget(btn_close_);
}

void IncomingCallDialog::setupUI() {
    // 创建中央容器来管理整个内容
    QWidget* central_widget = new QWidget(this);
    central_widget->setStyleSheet("QWidget { background-color: #1a1a1a; }");
    
    // 主布局 - 直接设置为中央容器的布局
    QVBoxLayout* layout = new QVBoxLayout(central_widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // 添加标题栏
    layout->addWidget(title_bar_);
    
    // 内容区域布局
    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->setContentsMargins(30, 0, 30, 30);
    main_layout->setSpacing(20);
    
    main_layout->addSpacing(20);
    
    // 顶部图标区域
    icon_label_ = new QLabel(central_widget);
    icon_label_->setAlignment(Qt::AlignCenter);
    icon_label_->setPixmap(QIcon(":/icon/video.png").pixmap(48, 48));
    icon_label_->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(76, 175, 80, 0.15);"
        "   border-radius: 45px;"
        "   padding: 18px;"
        "   border: 2px solid rgba(76, 175, 80, 0.4);"
        "}"
    );
    icon_label_->setFixedSize(90, 90);
    
    QHBoxLayout* icon_layout = new QHBoxLayout();
    icon_layout->addStretch();
    icon_layout->addWidget(icon_label_);
    icon_layout->addStretch();
    main_layout->addLayout(icon_layout);
    
    main_layout->addSpacing(15);
    
    // 呼叫方标签 - 更醒目的设计，自适应宽度
    caller_label_ = new QLabel(caller_id_, central_widget);
    caller_label_->setAlignment(Qt::AlignCenter);
    caller_label_->setWordWrap(true);
    caller_label_->setStyleSheet(
        "QLabel {"
        "   font-size: 24px;"
        "   font-weight: bold;"
        "   color: #ffffff;"
        "   padding: 10px 15px;"
        "   background-color: rgba(255, 255, 255, 0.08);"
        "   border-radius: 10px;"
        "}"
    );
    caller_label_->setMaximumWidth(400);
    
    QHBoxLayout* caller_layout = new QHBoxLayout();
    caller_layout->addStretch();
    caller_layout->addWidget(caller_label_);
    caller_layout->addStretch();
    main_layout->addLayout(caller_layout);
    
    // 状态标签 - 动画效果提示
    status_label_ = new QLabel("正在呼叫您...", central_widget);
    status_label_->setAlignment(Qt::AlignCenter);
    status_label_->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   color: #27ae60;"
        "   font-weight: 600;"
        "}"
    );
    main_layout->addWidget(status_label_);
    
    main_layout->addSpacing(25);
    
    // 按钮布局
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->setSpacing(40);
    
    // 拒绝按钮 - 使用图标
    reject_button_ = new QPushButton(central_widget);
    reject_button_->setFixedSize(80, 80);
    reject_button_->setCursor(Qt::PointingHandCursor);
    reject_button_->setIcon(QIcon(":/icon/close.png"));
    reject_button_->setIconSize(QSize(40, 40));
    reject_button_->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #e74c3c, stop:1 #c0392b);"
        "   border: 3px solid #a93226;"
        "   border-radius: 40px;"
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
    reject_layout->setSpacing(10);
    reject_layout->addWidget(reject_button_, 0, Qt::AlignCenter);
    QLabel* reject_label = new QLabel("拒绝", central_widget);
    reject_label->setAlignment(Qt::AlignCenter);
    reject_label->setStyleSheet("QLabel { color: #e74c3c; font-size: 14px; font-weight: 600; }");
    reject_layout->addWidget(reject_label);
    
    button_layout->addLayout(reject_layout);
    
    // 接听按钮 - 使用图标
    accept_button_ = new QPushButton(central_widget);
    accept_button_->setFixedSize(80, 80);
    accept_button_->setCursor(Qt::PointingHandCursor);
    accept_button_->setIcon(QIcon(":/icon/answer.png"));
    accept_button_->setIconSize(QSize(40, 40));
    accept_button_->setStyleSheet(
        "QPushButton {"
        "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "       stop:0 #27ae60, stop:1 #229954);"
        "   border: 3px solid #1e8449;"
        "   border-radius: 40px;"
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
    accept_layout->setSpacing(10);
    accept_layout->addWidget(accept_button_, 0, Qt::AlignCenter);
    QLabel* accept_label = new QLabel("接听", central_widget);
    accept_label->setAlignment(Qt::AlignCenter);
    accept_label->setStyleSheet("QLabel { color: #27ae60; font-size: 14px; font-weight: 600; }");
    accept_layout->addWidget(accept_label);
    
    button_layout->addLayout(accept_layout);
    
    QHBoxLayout* centered_button_layout = new QHBoxLayout();
    centered_button_layout->addStretch();
    centered_button_layout->addLayout(button_layout);
    centered_button_layout->addStretch();
    
    main_layout->addLayout(centered_button_layout);
    main_layout->addStretch();
    
    layout->addLayout(main_layout);
    
    // 设置中央容器为整个窗口的内容
    QVBoxLayout* window_layout = new QVBoxLayout(this);
    window_layout->setContentsMargins(0, 0, 0, 0);
    window_layout->setSpacing(0);
    window_layout->addWidget(central_widget);
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
    close();
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
    close();
}

void IncomingCallDialog::onTimeout() {
    qDebug() << "IncomingCallDialog: Call timeout";
    
    if (blink_timer_) {
        blink_timer_->stop();
    }
    
    // 超时自动拒绝
    emit rejected();
    close();
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
    
    QString style_bright = "QLabel { background-color: rgba(76, 175, 80, 0.3); border-radius: 45px; padding: 18px; border: 3px solid rgba(76, 175, 80, 0.8); }";
    QString style_dim = "QLabel { background-color: rgba(76, 175, 80, 0.15); border-radius: 45px; padding: 18px; border: 2px solid rgba(76, 175, 80, 0.4); }";
    
    if (blink_state_) {
        icon_label_->setStyleSheet(style_bright);
    } else {
        icon_label_->setStyleSheet(style_dim);
    }
}
