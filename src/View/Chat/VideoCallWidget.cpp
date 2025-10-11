#include "View/Chat/VideoCallWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTimer>


VideoCallWidget::VideoCallWidget(QWidget *parent)
    : FramelessWindow(parent), 
      m_callDuration(0), 
      m_isMuted(false), 
      m_isVideoOn(true),
      m_isPinned(false),
      m_isMaximized(false),
      m_isDragging(false),
      m_isResizing(false) {
    setMinimumSize(400, 300);
    resize(960, 720); // Default size
    setStyleSheet("background-color: #212121; color: white; font-family: 'Microsoft YaHei';");

    setupUi();
    
    // 默认显示来电界面，并设置示例信息
    m_stackedWidget->setCurrentIndex(0);
    setCallerInfo("XIAO XU", QPixmap(":/icon/blazer.png"));
}

void VideoCallWidget::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_stackedWidget = new QStackedWidget(this);
    
    QWidget *incomingCallWidget = setupIncomingCallUi();
    QWidget *inCallWidget = setupInCallUi();

    m_stackedWidget->addWidget(incomingCallWidget);
    m_stackedWidget->addWidget(inCallWidget);

    mainLayout->addWidget(m_stackedWidget);
    setLayout(mainLayout);

    // Setup window control buttons - 参考MainWindow的设计，直接放在窗口上
    setupWindowControls();
}

void VideoCallWidget::setupWindowControls() {
    // 参考MainWindow的设计，窗口控制按钮直接放在窗口上
    m_pinButton = new QPushButton(this);
    m_minimizeButton = new QPushButton(this);
    m_maximizeButton = new QPushButton(this);
    m_closeButton = new QPushButton(this);

    // 设置图标
    m_pinButton->setIcon(QIcon(":/icon/zhiding.png"));
    m_minimizeButton->setIcon(QIcon(":/icon/minimize.png"));
    m_maximizeButton->setIcon(QIcon(":/icon/maximize.png"));
    m_closeButton->setIcon(QIcon(":/icon/close.png"));

    // 设置图标大小
    m_pinButton->setIconSize(QSize(16, 16));
    m_minimizeButton->setIconSize(QSize(16, 16));
    m_maximizeButton->setIconSize(QSize(16, 16));
    m_closeButton->setIconSize(QSize(16, 16));

    // 参考MainWindow的按钮样式
    auto btnStyle = R"(
        QPushButton {
            background-color: transparent;
            border: none;
        }
        QPushButton:hover {
            background-color: #E9E9E9;
        }
    )";

    m_pinButton->setStyleSheet(btnStyle);
    m_minimizeButton->setStyleSheet(btnStyle);
    m_maximizeButton->setStyleSheet(btnStyle);
    m_closeButton->setStyleSheet(R"(
        QPushButton {
            background-color: transparent;
            border: none;
        }
        QPushButton:hover {
            background-color: #C42B1C;
        }
    )");

    // 设置按钮大小
    int btnSize = 32;
    m_pinButton->setFixedSize(btnSize, btnSize);
    m_minimizeButton->setFixedSize(btnSize, btnSize);
    m_maximizeButton->setFixedSize(btnSize, btnSize);
    m_closeButton->setFixedSize(btnSize, btnSize);

    // 连接信号
    connect(m_pinButton, &QPushButton::clicked, this, &VideoCallWidget::onPinClicked);
    connect(m_minimizeButton, &QPushButton::clicked, this, &VideoCallWidget::onMinimizeClicked);
    connect(m_maximizeButton, &QPushButton::clicked, this, &VideoCallWidget::onMaximizeClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &VideoCallWidget::onCloseClicked);

    // 安装事件过滤器用于close按钮的hover效果
    m_closeButton->installEventFilter(this);

    // 初始定位按钮
    resizeEvent(nullptr);
}

QWidget* VideoCallWidget::setupIncomingCallUi() {
    auto *widget = new QWidget(this);
    auto *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(40, 80, 40, 40);

    // 顶部：来电者信息
    auto *callerInfoWidget = new QWidget();
    auto *callerInfoLayout = new QVBoxLayout(callerInfoWidget);
    callerInfoLayout->setAlignment(Qt::AlignHCenter);
    callerInfoLayout->setSpacing(15);
    
    m_callerAvatarLabel = new QLabel();
    m_callerAvatarLabel->setFixedSize(120, 120);
    m_callerAvatarLabel->setScaledContents(true);
    
    m_callerNameLabel = new QLabel();
    m_callerNameLabel->setStyleSheet("font-size: 28px; font-weight: bold;");
    
    auto *callStatusLabel = new QLabel("邀请您加入视频通话");
    callStatusLabel->setStyleSheet("font-size: 16px; color: #AAAAAA;");
    
    callerInfoLayout->addWidget(m_callerAvatarLabel, 0, Qt::AlignHCenter);
    callerInfoLayout->addSpacing(10);
    callerInfoLayout->addWidget(m_callerNameLabel, 0, Qt::AlignHCenter);
    callerInfoLayout->addWidget(callStatusLabel, 0, Qt::AlignHCenter);

    // 底部：控制按钮和自己的信息
    auto *bottomWidget = new QWidget();
    auto *bottomLayout = new QVBoxLayout(bottomWidget);
    bottomLayout->setSpacing(20);

    auto *myNameLabel = new QLabel("Sqhh99");
    myNameLabel->setAlignment(Qt::AlignHCenter);
    myNameLabel->setStyleSheet("font-size: 16px;");

    auto *controlsWidget = new QWidget();
    auto *controlsLayout = new QHBoxLayout(controlsWidget);
    controlsLayout->setSpacing(60);
    controlsLayout->setAlignment(Qt::AlignCenter);

    auto createButton = [](const QString& text, const QString& iconPath, const QString& styleSheet) {
        auto *button = new QPushButton(text);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(32, 32));
        button->setFixedSize(120, 80);
        button->setStyleSheet(
            "QPushButton { background-color: " + styleSheet + "; border-radius: 12px; color: white; font-size: 14px; }"
            "QPushButton:hover { background-color: " + styleSheet.left(styleSheet.indexOf(';')) + "99; }" // Add alpha for hover
        );
        return button;
    };

    auto *voiceAnswerButton = createButton("语音接听", ":/icon/answer.png", "#333333");
    auto *answerButton = createButton("接听", ":/icon/video.png", "#28a745");
    auto *hangUpButton = createButton("挂断", ":/icon/hang_up.png", "#dc3545");

    controlsLayout->addWidget(voiceAnswerButton);
    controlsLayout->addWidget(answerButton);
    controlsLayout->addWidget(hangUpButton);
    
    bottomLayout->addWidget(myNameLabel);
    bottomLayout->addWidget(controlsWidget);

    layout->addStretch();
    layout->addWidget(callerInfoWidget);
    layout->addStretch();
    layout->addWidget(bottomWidget);

    connect(answerButton, &QPushButton::clicked, this, &VideoCallWidget::onAnswerClicked);
    connect(hangUpButton, &QPushButton::clicked, this, &VideoCallWidget::onHangUpClicked);
    connect(voiceAnswerButton, &QPushButton::clicked, this, &VideoCallWidget::onVoiceAnswerClicked);
    
    return widget;
}

QWidget* VideoCallWidget::setupInCallUi() {
    auto *widget = new QWidget(this);
    auto *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(20, 20, 20, 40);

    // 顶部：通话时长
    m_callDurationLabel = new QLabel("00:00");
    m_callDurationLabel->setAlignment(Qt::AlignHCenter);
    m_callDurationLabel->setStyleSheet("font-size: 16px; color: #FFFFFF;");

    // 中间：对方视频/头像
    auto *remoteVideoLabel = new QLabel();
    remoteVideoLabel->setAlignment(Qt::AlignCenter);
    remoteVideoLabel->setPixmap(QPixmap(":/icon/nether.png").scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 底部：控制按钮
    auto *bottomWidget = new QWidget();
    auto *bottomLayout = new QVBoxLayout(bottomWidget);
    bottomLayout->setSpacing(20);

    auto *myNameLabel = new QLabel("Sqhh99");
    myNameLabel->setAlignment(Qt::AlignHCenter);
    myNameLabel->setStyleSheet("font-size: 16px;");

    auto *controlsWidget = new QWidget();
    auto *controlsLayout = new QHBoxLayout(controlsWidget);
    controlsLayout->setSpacing(30);
    controlsLayout->setAlignment(Qt::AlignCenter);

    auto createButton = [](const QString& text, const QString& iconPath) {
        auto *button = new QPushButton(text);
        button->setIcon(QIcon(iconPath));
        button->setIconSize(QSize(28, 28));
        button->setFixedSize(130, 70);
        button->setStyleSheet(
            "QPushButton { background-color: #333333; border-radius: 12px; color: white; font-size: 14px; }"
            "QPushButton:hover { background-color: #444444; }"
        );
        return button;
    };

    m_muteButton = createButton("关闭麦克风", ":/icon/Turn_on_the_microphone.png");
    m_toggleVideoButton = createButton("关闭视频", ":/icon/start_video.png");
    auto *hangUpButton = createButton("退出通话", ":/icon/hang_up.png");
    hangUpButton->setStyleSheet(
        "QPushButton { background-color: #dc3545; border-radius: 12px; color: white; font-size: 14px; }"
        "QPushButton:hover { background-color: #c82333; }"
    );

    controlsLayout->addWidget(m_muteButton);
    controlsLayout->addWidget(m_toggleVideoButton);
    controlsLayout->addWidget(hangUpButton);
    
    bottomLayout->addWidget(myNameLabel);
    bottomLayout->addWidget(controlsWidget);

    layout->addWidget(m_callDurationLabel, 0, Qt::AlignHCenter);
    layout->addWidget(remoteVideoLabel, 1);
    layout->addWidget(bottomWidget);

    m_callTimer = new QTimer(this);
    connect(m_callTimer, &QTimer::timeout, this, &VideoCallWidget::updateCallDuration);
    connect(hangUpButton, &QPushButton::clicked, this, &VideoCallWidget::onHangUpClicked);
    connect(m_muteButton, &QPushButton::clicked, this, &VideoCallWidget::onMuteClicked);
    connect(m_toggleVideoButton, &QPushButton::clicked, this, &VideoCallWidget::onToggleVideoClicked);

    return widget;
}

void VideoCallWidget::setCallerInfo(const QString& name, const QPixmap& avatar) {
    m_callerNameLabel->setText(name);
    m_callerAvatarLabel->setPixmap(avatar.scaled(120, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void VideoCallWidget::onAnswerClicked() {
    m_stackedWidget->setCurrentIndex(1);
    m_callTimer->start(1000);
}

void VideoCallWidget::onHangUpClicked() {
    m_callTimer->stop();
    close();
}

void VideoCallWidget::onVoiceAnswerClicked() {
    onAnswerClicked();
    m_isVideoOn = false; // 开始时关闭视频
    m_toggleVideoButton->setText("开启视频");
    m_toggleVideoButton->setIcon(QIcon(":/icon/close_video.png"));
}

void VideoCallWidget::updateCallDuration() {
    m_callDuration++;
    int minutes = m_callDuration / 60;
    int seconds = m_callDuration % 60;
    m_callDurationLabel->setText(QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
}

void VideoCallWidget::onMuteClicked() {
    m_isMuted = !m_isMuted;
    if (m_isMuted) {
        m_muteButton->setText("开启麦克风");
        m_muteButton->setIcon(QIcon(":/icon/mute_the_microphone.png"));
    } else {
        m_muteButton->setText("关闭麦克风");
        m_muteButton->setIcon(QIcon(":/icon/Turn_on_the_microphone.png"));
    }
}

void VideoCallWidget::onToggleVideoClicked() {
    m_isVideoOn = !m_isVideoOn;
    if (m_isVideoOn) {
        m_toggleVideoButton->setText("关闭视频");
        m_toggleVideoButton->setIcon(QIcon(":/icon/start_video.png"));
    } else {
        m_toggleVideoButton->setText("开启视频");
        m_toggleVideoButton->setIcon(QIcon(":/icon/close_video.png"));
    }
}

void VideoCallWidget::onMinimizeClicked() {
    showMinimized();
}

void VideoCallWidget::onMaximizeClicked() {
    if (m_isMaximized) {
        showNormal();
        m_isMaximized = false;
    } else {
        showMaximized();
        m_isMaximized = true;
    }
}

void VideoCallWidget::onCloseClicked() {
    close();
}

void VideoCallWidget::onPinClicked() {
    m_isPinned = !m_isPinned;
    if (m_isPinned) {
        setWindowFlag(Qt::WindowStaysOnTopHint, true);
        m_pinButton->setIcon(QIcon(":/icon/yizhiding.png"));
    } else {
        setWindowFlag(Qt::WindowStaysOnTopHint, false);
        m_pinButton->setIcon(QIcon(":/icon/zhiding.png"));
    }
    show(); // Re-apply window flags
}

void VideoCallWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        
        // Check if clicking on title area (top 40 pixels) for dragging
        if (pos.y() <= 40) {
            m_isDragging = true;
            m_dragStartPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
            return;
        }
        
        // Check if resizing
        m_resizeEdges = Qt::Edges();
        const int margin = 10;
        
        if (pos.x() <= margin) m_resizeEdges |= Qt::LeftEdge;
        if (pos.x() >= width() - margin) m_resizeEdges |= Qt::RightEdge;
        if (pos.y() <= margin) m_resizeEdges |= Qt::TopEdge;
        if (pos.y() >= height() - margin) m_resizeEdges |= Qt::BottomEdge;
        
        if (m_resizeEdges != Qt::Edges()) {
            m_isResizing = true;
            m_originalGeometry = geometry();
            event->accept();
            return;
        }
    }
    FramelessWindow::mousePressEvent(event);
}

void VideoCallWidget::mouseMoveEvent(QMouseEvent *event) {
    if (m_isDragging) {
        move(event->globalPosition().toPoint() - m_dragStartPosition);
        event->accept();
        return;
    }
    
    if (m_isResizing) {
        QRect newGeometry = m_originalGeometry;
        QPoint globalPos = event->globalPosition().toPoint();
        
        if (m_resizeEdges & Qt::LeftEdge) {
            newGeometry.setLeft(globalPos.x());
        }
        if (m_resizeEdges & Qt::RightEdge) {
            newGeometry.setRight(globalPos.x());
        }
        if (m_resizeEdges & Qt::TopEdge) {
            newGeometry.setTop(globalPos.y());
        }
        if (m_resizeEdges & Qt::BottomEdge) {
            newGeometry.setBottom(globalPos.y());
        }
        
        // Ensure minimum size
        if (newGeometry.width() >= minimumWidth() && newGeometry.height() >= minimumHeight()) {
            setGeometry(newGeometry);
        }
        event->accept();
        return;
    }
    
    // Update cursor for resize edges
    QPoint pos = event->pos();
    Qt::Edges edges;
    const int margin = 10;
    
    if (pos.x() <= margin) edges |= Qt::LeftEdge;
    if (pos.x() >= width() - margin) edges |= Qt::RightEdge;
    if (pos.y() <= margin) edges |= Qt::TopEdge;
    if (pos.y() >= height() - margin) edges |= Qt::BottomEdge;
    
    if (edges & Qt::LeftEdge && edges & Qt::TopEdge) setCursor(Qt::SizeFDiagCursor);
    else if (edges & Qt::RightEdge && edges & Qt::TopEdge) setCursor(Qt::SizeBDiagCursor);
    else if (edges & Qt::LeftEdge && edges & Qt::BottomEdge) setCursor(Qt::SizeBDiagCursor);
    else if (edges & Qt::RightEdge && edges & Qt::BottomEdge) setCursor(Qt::SizeFDiagCursor);
    else if (edges & Qt::LeftEdge || edges & Qt::RightEdge) setCursor(Qt::SizeHorCursor);
    else if (edges & Qt::TopEdge || edges & Qt::BottomEdge) setCursor(Qt::SizeVerCursor);
    else setCursor(Qt::ArrowCursor);
    
    QWidget::mouseMoveEvent(event);
}

void VideoCallWidget::mouseReleaseEvent(QMouseEvent *event) {
    m_isDragging = false;
    m_isResizing = false;
    m_resizeEdges = Qt::Edges();
    setCursor(Qt::ArrowCursor);
    QWidget::mouseReleaseEvent(event);
}

void VideoCallWidget::resizeEvent(QResizeEvent *event) {
    FramelessWindow::resizeEvent(event);

    // 参考MainWindow的设计，重新定位窗口控制按钮
    if (m_closeButton) {
        int w = width();
        int bw = m_closeButton->width();

        m_closeButton->move(w - bw, 0);
        m_maximizeButton->move(w - bw * 2, 0);
        m_minimizeButton->move(w - bw * 3, 0);
        m_pinButton->move(w - bw * 4, 0);
    }
}

bool VideoCallWidget::eventFilter(QObject *watched, QEvent *event) {
    if (watched == m_closeButton) {
        if (event->type() == QEvent::Enter) {
            m_closeButton->setIcon(QIcon(":/icon/hovered_close.png"));
        } else if (event->type() == QEvent::Leave) {
            m_closeButton->setIcon(QIcon(":/icon/close.png"));
        }
    }
    return QWidget::eventFilter(watched, event);
}
