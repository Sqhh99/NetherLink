#ifndef AICHATREPOSITORY_H
#define AICHATREPOSITORY_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QSharedPointer>

struct AiConversation {
    QString conversationId;
    QString userId;
    QString title;
    QDateTime createdAt;
    QDateTime updatedAt;
};

struct AiMessage {
    int messageId;
    QString conversationId;
    QString role; // "user" 或 "ai"
    QString content;
    QDateTime createdAt;
};

class AiChatRepository : public QObject {
    Q_OBJECT

public:
    static AiChatRepository& instance();

    // 对话管理
    bool createConversation(const QString& conversationId, const QString& userId, const QString& title = "新对话");
    QVector<AiConversation> getAllConversations(const QString& userId);
    AiConversation getConversation(const QString& conversationId);
    bool updateConversationTitle(const QString& conversationId, const QString& newTitle);
    bool updateConversationTimestamp(const QString& conversationId);
    bool deleteConversation(const QString& conversationId);

    // 消息管理
    bool addMessage(const QString& conversationId, const QString& role, const QString& content);
    QVector<AiMessage> getMessages(const QString& conversationId);
    bool deleteMessages(const QString& conversationId);

private:
    explicit AiChatRepository(QObject* parent = nullptr);
    bool ensureDatabaseOpen();
};

#endif // AICHATREPOSITORY_H
