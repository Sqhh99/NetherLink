/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include <QPainter>

#include "View/AiChat/AiChatApplication.h"
#include "View/AiChat/AiChatWindow.h"
#include "Data/AiChatRepository.h"

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

AiChatApplication::AiChatApplication(QWidget* parent) : QWidget(parent), m_leftPane(new LeftPane(this)), m_defaultPage(new DefaultPage(this)), m_splitter(new QSplitter(Qt::Horizontal, this)), m_rightPane(new QStackedWidget(this)) {
    m_splitter->addWidget(m_leftPane);
    m_rightPane->addWidget(m_defaultPage);
    m_splitter->addWidget(m_rightPane);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setSizes({250, width() - 250});
    m_splitter->handle(1)->setCursor(Qt::SizeHorCursor);
    connect(m_leftPane->chatList(), &AiChatListWidget::chatItemClicked, this, &AiChatApplication::onChatItemClicked);
    connect(m_leftPane->chatList(), &AiChatListWidget::chatItemRenamed, this, &AiChatApplication::onChatItemRenamed);
    connect(m_leftPane->chatList(), &AiChatListWidget::chatItemDeleted, this, &AiChatApplication::onChatItemDeleted);
}

void AiChatApplication::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    m_splitter->setGeometry(0, 0, width(), height());
}

void AiChatApplication::paintEvent(QPaintEvent* /* event */) {
    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);
}

void AiChatApplication::onChatItemClicked(AiChatListItem* item) {
    QString conversationId = item->conversationId();

    // 使用conversationId作为窗口的key
    if (!m_chatWindows.contains(conversationId)) {
        // 创建窗口时传入conversationId，这样会自动加载历史消息
        auto* chatWindow = new AiChatWindow(conversationId, this);
        m_rightPane->addWidget(chatWindow);
        m_chatWindows[conversationId] = chatWindow;

        // 如果是新对话（conversationId为空），连接信号来更新item标题
        if (conversationId.isEmpty()) {
            connect(chatWindow, &AiChatWindow::conversationCreated, this, [this, item](const QString& convId, const QString& title) {
                onConversationCreated(item, convId, title);
            });
        }
    }
    m_rightPane->setCurrentWidget(m_chatWindows[conversationId]);
}

void AiChatApplication::onChatItemRenamed(AiChatListItem* item) {
    // 更新数据库中的对话标题
    QString conversationId = item->conversationId();
    QString newTitle = item->title();
    
    AiChatRepository::instance().updateConversationTitle(conversationId, newTitle);
    qDebug() << "对话重命名成功:" << conversationId << newTitle;
}

void AiChatApplication::onChatItemDeleted(AiChatListItem* item) {
    QString conversationId = item->conversationId();

    // 删除数据库中的对话和消息
    AiChatRepository::instance().deleteConversation(conversationId);
    
    // 删除对应的窗口
    if (m_chatWindows.contains(conversationId)) {
        auto* chatWindow = m_chatWindows.take(conversationId);
        m_rightPane->removeWidget(chatWindow);
        delete chatWindow;
    }
    
    qDebug() << "对话删除成功:" << conversationId;
}

void AiChatApplication::onConversationCreated(AiChatListItem* item, const QString& conversationId, const QString& title) {
    // 更新item的标题和conversationId
    item->setTitle(title);
    item->setConversationId(conversationId);

    // 更新窗口映射的key
    if (m_chatWindows.contains("")) {
        auto* chatWindow = m_chatWindows.take("");
        m_chatWindows[conversationId] = chatWindow;
    }

    qDebug() << "对话创建完成，更新UI:" << conversationId << title;
}
