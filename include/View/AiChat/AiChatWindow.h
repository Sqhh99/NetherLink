/* guard ------------------------------------------------------------------ 80 // ! ----------------------------- 120 */

#ifndef INCLUDE_VIEW_AI_CHAT_AI_CHAT_WINDOW
#define INCLUDE_VIEW_AI_CHAT_AI_CHAT_WINDOW

/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include <QBoxLayout>

#include "Components/FloatingInputBar.h"
#include "View/AiChat/AiChatListModel.h"
#include "View/AiChat/AiChatListView.h"
#include "View/AiChat/AiChatWebSocket.h"

/* class ------------------------------------------------------------------ 80 // ! ----------------------------- 120 */
class AiChatWindow : public QWidget {
    Q_OBJECT

    public:
        explicit AiChatWindow(QWidget* parent = nullptr);
        explicit AiChatWindow(const QString& conversationId, QWidget* parent = nullptr);

        void loadConversation(const QString& conversationId);

    signals:
        void conversationCreated(const QString& conversationId, const QString& title);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private slots:
        void onSendMessage(const QString& content);

        void onMessageContent(const QString& content);

        void onConversationStarted(const QString& conversationId);

        void onMessageEnded();

        void onConnectionEstablished();

        void onConnectionError(const QString& error);

        void onRequestError(const QString& errorMessage);

    private:
        void setupUI();

        void updateInputBarPosition();

        void saveMessage(const QString& role, const QString& content);

        void loadHistoryMessages();

        QVBoxLayout* m_mainLayout;
        AiChatListView* m_chatView;
        FloatingInputBar* m_inputBar;
        AiChatWebSocket* m_webSocket;
        AiChatListModel* m_model;
        QString m_currentConversationId;
        QString m_pendingAiMessage; // 用于累积AI流式响应
        QString m_firstUserMessage; // 用于保存第一条用户消息作为标题
        bool m_isNewConversation;   // 标记是否是新对话
        static constexpr int INPUT_BAR_MARGIN = 20; // 输入栏与边缘的距离
};

#endif /* INCLUDE_VIEW_AI_CHAT_AI_CHAT_WINDOW */
