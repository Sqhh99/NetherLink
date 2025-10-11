#ifndef VIDEO_CALL_WIDGET_H
#define VIDEO_CALL_WIDGET_H

#include <QWidget>
#include <QMouseEvent>

#include "Utils/FramelessWindow.h"

class QStackedWidget;
class QLabel;
class QPushButton;
class QTimer;

class VideoCallWidget : public FramelessWindow {
    Q_OBJECT

public:
    explicit VideoCallWidget(QWidget *parent = nullptr);
    void setCallerInfo(const QString& name, const QPixmap& avatar);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onAnswerClicked();
    void onHangUpClicked();
    void onVoiceAnswerClicked();
    void updateCallDuration();
    void onMuteClicked();
    void onToggleVideoClicked();
    void onMinimizeClicked();
    void onMaximizeClicked();
    void onCloseClicked();
    void onPinClicked();

private:
    void setupUi();
    QWidget* setupIncomingCallUi();
    QWidget* setupInCallUi();
    void setupWindowControls();

    QStackedWidget *m_stackedWidget;

    // Window control buttons
    QPushButton *m_minimizeButton;
    QPushButton *m_maximizeButton;
    QPushButton *m_closeButton;
    QPushButton *m_pinButton;

    // Caller info
    QLabel *m_callerAvatarLabel;
    QLabel *m_callerNameLabel;

    // In-call elements
    QLabel *m_callDurationLabel;
    QTimer *m_callTimer;
    int m_callDuration;

    // Buttons
    QPushButton *m_muteButton;
    QPushButton *m_toggleVideoButton;

    bool m_isMuted;
    bool m_isVideoOn;
    bool m_isPinned;
    bool m_isMaximized;

    // For window moving and resizing
    bool m_isDragging;
    QPoint m_dragStartPosition;
    bool m_isResizing;
    Qt::Edges m_resizeEdges;
    QRect m_originalGeometry;
};

#endif // VIDEO_CALL_WIDGET_H
