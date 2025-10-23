/* guard ------------------------------------------------------------------ 80 // ! ----------------------------- 120 */

#ifndef INCLUDE_VIEW_CALL_INCOMING_CALL_DIALOG
#define INCLUDE_VIEW_CALL_INCOMING_CALL_DIALOG

/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "Utils/FramelessWindow.h"

/* class ------------------------------------------------------------------ 80 // ! ----------------------------- 120 */

/**
 * @brief 来电对话框
 * 
 * 显示来电通知,提供接听和拒绝选项
 */
class IncomingCallDialog : public FramelessWindow {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param caller_id 呼叫方ID/名称
     * @param parent 父窗口
     */
    explicit IncomingCallDialog(const QString& caller_id, QWidget* parent = nullptr);
    ~IncomingCallDialog();

signals:
    /**
     * @brief 接听通话
     */
    void accepted();

    /**
     * @brief 拒绝通话
     */
    void rejected();

private slots:
    void onAcceptClicked();
    void onRejectClicked();
    void onTimeout();
    void updateBlinkAnimation();

private:
    void setupUI();
    void setupTitleBar();
    void startBlinkAnimation();

private:
    QString caller_id_;
    QWidget* title_bar_;
    QPushButton* btn_close_;
    QLabel* caller_label_;
    QLabel* status_label_;
    QLabel* icon_label_;
    QPushButton* accept_button_;
    QPushButton* reject_button_;
    QTimer* timeout_timer_;
    QTimer* blink_timer_;
    bool blink_state_;
    
    static constexpr int RING_TIMEOUT_MS = 30000; // 30秒超时
};

#endif /* INCLUDE_VIEW_CALL_INCOMING_CALL_DIALOG */
