/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include <QRandomGenerator>
#include <QScrollBar>

#include "View/AiChat/AiChatListWidget.h"
#include "Data/AiChatRepository.h"
#include "Data/CurrentUser.h"

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

AiChatListWidget::AiChatListWidget(QWidget* parent) : CustomScrollArea(parent) {
    // 将滚动区的 viewport 作为内容容器
    content = contentWidget;
    content->setMouseTracking(true);
    setStyleSheet("border-width:0px;border-style:solid;");

    // 安装事件过滤器以处理滚动事件
    viewport()->installEventFilter(this);

    // 加载真实的对话历史
    loadConversations();
    layoutContent();
}

void AiChatListWidget::loadConversations() {
    // 清空现有项
    qDeleteAll(m_items);
    m_items.clear();

    // 从数据库加载对话列表
    QString userId = CurrentUser::instance().getUserId();
    if (userId.isEmpty()) {
        qWarning() << "用户未登录，无法加载AI对话";
        return;
    }

    auto conversations = AiChatRepository::instance().getAllConversations(userId);
    
    qDebug() << "加载了" << conversations.size() << "个AI对话";

    for (const auto& conv : conversations) {
        AiChatListItem* item = new AiChatListItem(content);
        item->setTitle(conv.title);
        item->setTime(conv.updatedAt);
        item->setConversationId(conv.conversationId);  // 需要添加这个方法
        addChatItem(item);
    }
}

void AiChatListWidget::addChatItem(AiChatListItem* item) {
    connect(item, &AiChatListItem::clicked, this, &AiChatListWidget::onItemClicked);
    connect(item, &AiChatListItem::requestRename, this, &AiChatListWidget::chatItemRenamed);
    connect(item, &AiChatListItem::requestDelete, this, &AiChatListWidget::chatItemDeleted);
    connect(item, &AiChatListItem::timeUpdated, this, &AiChatListWidget::chatOrderChanged);
    m_items.append(item);
    item->setParent(content);
    item->show();
    layoutContent();
}

QString AiChatListWidget::timeSectionText(const QDateTime& dt) const {
    QDateTime now = QDateTime::currentDateTime();
    int days = dt.date().daysTo(now.date());

    if (dt.date() == now.date())
        return (QStringLiteral("今天"));
    else if (dt.date().addDays(1) == now.date())
        return (QStringLiteral("昨天"));
    else if (days <= 7)
        return (QStringLiteral("七天内"));
    else if (days <= 30)
        return (QStringLiteral("一个月内"));
    else
        return (dt.toString("yyyy-MM"));
}

void AiChatListWidget::layoutContent() {
    // 1. 按时间降序排序
    std::sort(m_items.begin(), m_items.end(), [] (AiChatListItem* a, AiChatListItem* b) {
        return (a->time() > b->time());
    });

    // 2. 布局项目
    int y = topMargin;
    const int W = content->width();

    for (auto* item : m_items) {
        int h = item->height();

        item->setGeometry(leftMargin, y, W - 2 * leftMargin, h);
        y += h + itemSpacing;
    }

    // 3. 调整内容总高度
    content->resize(W, y + topMargin);
}

QString AiChatListWidget::getCurrentVisibleTimeText() const {
    // 获取可视区域的位置
    int scrollPos = verticalScrollBar()->value();

    // 查找第一个可见的项目
    for (const auto* item : m_items) {
        // 如果项目的底部位置大于滚动位置，说明这个项目是可见的
        if ((item->y() + item->height()) >= scrollPos) {
            return (timeSectionText(item->time()));
        }
    }
    return (QString());
}

void AiChatListWidget::onItemClicked(AiChatListItem* item) {
    if (selectedItem) {
        selectedItem->setSelected(false);
    }
    selectedItem = item;
    item->setSelected(true);

    emit chatItemClicked(item);
}
