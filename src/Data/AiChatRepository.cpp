/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include "Data/AiChatRepository.h"
#include "Data/DatabaseManager.h"
#include "Data/CurrentUser.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

AiChatRepository::AiChatRepository(QObject* parent) : QObject(parent) {}

AiChatRepository& AiChatRepository::instance() {
    static AiChatRepository repo;
    return repo;
}

bool AiChatRepository::ensureDatabaseOpen() {
    QString userId = CurrentUser::instance().getUserId();
    if (userId.isEmpty()) {
        qWarning() << "Current user ID is not set";
        return false;
    }
    return DatabaseManager::instance().initUserDatabase(userId);
}

bool AiChatRepository::createConversation(const QString& conversationId, const QString& userId, const QString& title) {
    qDebug() << "AiChatRepository::createConversation - conversationId:" << conversationId << "userId:" << userId << "title:" << title;
    
    if (!ensureDatabaseOpen()) {
        qWarning() << "无法打开数据库";
        return false;
    }

    QSqlDatabase db = DatabaseManager::instance().getUserDatabase(CurrentUser::instance().getUserId());
    
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "数据库无效或未打开";
        return false;
    }
    
    qDebug() << "数据库路径:" << db.databaseName();
    
    QSqlQuery query(db);

    query.prepare("INSERT INTO ai_conversations (conversation_id, user_id, title, created_at, updated_at) "
                  "VALUES (?, ?, ?, datetime('now', 'localtime'), datetime('now', 'localtime'))");
    query.addBindValue(conversationId);
    query.addBindValue(userId);
    query.addBindValue(title);

    if (!query.exec()) {
        qWarning() << "Failed to create conversation:" << query.lastError().text();
        qWarning() << "SQL:" << query.executedQuery();
        return false;
    }

    qDebug() << "创建对话成功:" << conversationId << title;
    return true;
}

QVector<AiConversation> AiChatRepository::getAllConversations(const QString& userId) {
    QVector<AiConversation> conversations;

    if (!ensureDatabaseOpen()) {
        return conversations;
    }

    QSqlDatabase db = DatabaseManager::instance().getUserDatabase(CurrentUser::instance().getUserId());
    QSqlQuery query(db);

    query.prepare("SELECT conversation_id, user_id, title, created_at, updated_at "
                  "FROM ai_conversations WHERE user_id = ? ORDER BY updated_at DESC");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            AiConversation conv;
            conv.conversationId = query.value(0).toString();
            conv.userId = query.value(1).toString();
            conv.title = query.value(2).toString();
            conv.createdAt = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
            conv.updatedAt = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
            conversations.append(conv);
        }
        qDebug() << "加载了" << conversations.size() << "个AI对话";
    } else {
        qWarning() << "Failed to get conversations:" << query.lastError().text();
    }

    return conversations;
}

AiConversation AiChatRepository::getConversation(const QString& conversationId) {
    AiConversation conv;

    if (!ensureDatabaseOpen()) {
        return conv;
    }

    QSqlDatabase db = DatabaseManager::instance().getUserDatabase(CurrentUser::instance().getUserId());
    QSqlQuery query(db);

    query.prepare("SELECT conversation_id, user_id, title, created_at, updated_at "
                  "FROM ai_conversations WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (query.exec() && query.next()) {
        conv.conversationId = query.value(0).toString();
        conv.userId = query.value(1).toString();
        conv.title = query.value(2).toString();
        conv.createdAt = QDateTime::fromString(query.value(3).toString(), Qt::ISODate);
        conv.updatedAt = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
    } else {
        qWarning() << "Failed to get conversation:" << query.lastError().text();
    }

    return conv;
}

bool AiChatRepository::updateConversationTitle(const QString& conversationId, const QString& newTitle) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlDatabase db = DatabaseManager::instance().getUserDatabase(CurrentUser::instance().getUserId());
    QSqlQuery query(db);

    query.prepare("UPDATE ai_conversations SET title = ?, updated_at = datetime('now', 'localtime') "
                  "WHERE conversation_id = ?");
    query.addBindValue(newTitle);
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to update conversation title:" << query.lastError().text();
        return false;
    }

    return true;
}

bool AiChatRepository::updateConversationTimestamp(const QString& conversationId) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlDatabase db = DatabaseManager::instance().getUserDatabase(CurrentUser::instance().getUserId());
    QSqlQuery query(db);

    query.prepare("UPDATE ai_conversations SET updated_at = datetime('now', 'localtime') "
                  "WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to update conversation timestamp:" << query.lastError().text();
        return false;
    }

    return true;
}

bool AiChatRepository::deleteConversation(const QString& conversationId) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlDatabase db = DatabaseManager::instance().getUserDatabase(CurrentUser::instance().getUserId());
    QSqlQuery query(db);

    query.prepare("DELETE FROM ai_conversations WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to delete conversation:" << query.lastError().text();
        return false;
    }

    qDebug() << "删除对话成功:" << conversationId;
    return true;
}

bool AiChatRepository::addMessage(const QString& conversationId, const QString& role, const QString& content) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlDatabase db = DatabaseManager::instance().getUserDatabase(CurrentUser::instance().getUserId());
    QSqlQuery query(db);

    query.prepare("INSERT INTO ai_messages (conversation_id, role, content, created_at) "
                  "VALUES (?, ?, ?, datetime('now', 'localtime'))");
    query.addBindValue(conversationId);
    query.addBindValue(role);
    query.addBindValue(content);

    if (!query.exec()) {
        qWarning() << "Failed to add message:" << query.lastError().text();
        return false;
    }

    // 更新对话的最后更新时间
    updateConversationTimestamp(conversationId);

    return true;
}

QVector<AiMessage> AiChatRepository::getMessages(const QString& conversationId) {
    QVector<AiMessage> messages;

    if (!ensureDatabaseOpen()) {
        return messages;
    }

    QSqlDatabase db = DatabaseManager::instance().getUserDatabase(CurrentUser::instance().getUserId());
    QSqlQuery query(db);

    query.prepare("SELECT message_id, conversation_id, role, content, created_at "
                  "FROM ai_messages WHERE conversation_id = ? ORDER BY message_id ASC");
    query.addBindValue(conversationId);

    if (query.exec()) {
        while (query.next()) {
            AiMessage msg;
            msg.messageId = query.value(0).toInt();
            msg.conversationId = query.value(1).toString();
            msg.role = query.value(2).toString();
            msg.content = query.value(3).toString();
            msg.createdAt = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
            messages.append(msg);
        }
        qDebug() << "加载了" << messages.size() << "条消息，对话ID:" << conversationId;
    } else {
        qWarning() << "Failed to get messages:" << query.lastError().text();
    }

    return messages;
}

bool AiChatRepository::deleteMessages(const QString& conversationId) {
    if (!ensureDatabaseOpen()) {
        return false;
    }

    QSqlDatabase db = DatabaseManager::instance().getUserDatabase(CurrentUser::instance().getUserId());
    QSqlQuery query(db);

    query.prepare("DELETE FROM ai_messages WHERE conversation_id = ?");
    query.addBindValue(conversationId);

    if (!query.exec()) {
        qWarning() << "Failed to delete messages:" << query.lastError().text();
        return false;
    }

    return true;
}
