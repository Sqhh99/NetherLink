# 视频通话界面美化 - 快速参考

## 📊 美化对比总览

### VideoCallDialog（视频通话对话框）

| 组件 | 美化前 | 美化后 | 改进点 |
|------|--------|--------|--------|
| **整体风格** | 简单深色 | 渐变深色主题 | ✨ 现代化视觉 |
| **对话框背景** | #2c2c2c | 渐变 #1a1a1a | ✨ 增加深度感 |
| **信息栏高度** | 50px | 60px | ✨ 更宽敞 |
| **对方名称** | 普通文本 | 卡片式徽章 | ✨ 更突出 |
| **通话时长** | 普通文本 | 绿色圆角徽章 ⏱ | ✨ 更醒目 |
| **视频容器** | 纯黑背景 | 径向渐变 | ✨ 更柔和 |
| **状态标签** | 18px 白字 | 24px 卡片式 🔗 | ✨ 更清晰 |
| **本地视频** | 200x150 | 220x165 + 绿框 + 阴影 | ✨ 更明显 |
| **控制栏高度** | 80px | 100px | ✨ 更舒适 |
| **按钮形状** | 矩形 80x60 | 圆形 70-85 | ✨ 更现代 |
| **按钮图标** | 文字 | Emoji 图标 🔇📞📹 | ✨ 更直观 |
| **挂断按钮** | 普通大小 | 最大 85x85 + 阴影 | ✨ 主操作突出 |

---

### IncomingCallDialog（来电提示对话框）

| 组件 | 美化前 | 美化后 | 改进点 |
|------|--------|--------|--------|
| **对话框大小** | 350x200 | 400x280 | ✨ 更宽敞 |
| **背景主题** | 浅色/无渐变 | 深色渐变 + 圆角 | ✨ 更现代 |
| **来电图标** | ❌ 无 | ✅ 闪烁电话图标 📞 | ✨ 吸引注意 |
| **闪烁动画** | ❌ 无 | ✅ 600ms 周期闪烁 | ✨ 强提醒 |
| **呼叫方名称** | 普通文本 18px | 卡片式 22px | ✨ 更突出 |
| **状态文字** | 灰色 14px | 绿色 15px | ✨ 更醒目 |
| **按钮形状** | 矩形 120x45 | 圆形 75x75 | ✨ 更友好 |
| **按钮标识** | 文字 | 大号符号 ✕ ✓ | ✨ 更直观 |
| **按钮说明** | ❌ 无 | ✅ 底部文字标签 | ✨ 更清晰 |
| **按钮间距** | 20px | 30px | ✨ 更舒适 |
| **悬停效果** | 简单变色 | 变色 + 放大 1.1x | ✨ 更生动 |
| **阴影效果** | ❌ 无 | ✅ 按钮阴影 | ✨ 增强立体感 |

---

## 🎨 核心设计元素

### 1. 配色方案

```
深色背景系列：
#1a1a1a  ████ 主背景（最深）
#232323  ████ 次级背景
#2c2c2c  ████ 控制栏
#2d3436  ████ 对话框背景

成功色（绿色）：
#4CAF50  ████ 主绿色
#27ae60  ████ 接听按钮
#229954  ████ 接听按钮（深）

危险色（红色）：
#e74c3c  ████ 主红色
#c0392b  ████ 挂断/拒绝按钮
#a93226  ████ 按钮边框

中性色（灰色）：
#4a4a4a  ████ 按钮背景
#5a5a5a  ████ 按钮背景（亮）
```

### 2. 尺寸规范

```
【VideoCallDialog】
┌─────────────────────────────────┐
│  信息栏 (60px)                   │ ← 对方名称 + 通话时长
├─────────────────────────────────┤
│                                 │
│  视频容器 (expand)               │
│  ┌──────────────────────┐       │
│  │  远程视频 (全屏)      │       │
│  │                      │       │
│  │              ┌──────┐│       │
│  │              │本地  ││       │ ← 220x165 右下角
│  │              │视频  ││       │
│  │              └──────┘│       │
│  └──────────────────────┘       │
│                                 │
├─────────────────────────────────┤
│  控制栏 (100px)                  │
│  [🔇] [📞] [📹]                  │ ← 70-85px 圆形
└─────────────────────────────────┘

【IncomingCallDialog】 400x280
┌─────────────────────────┐
│                         │
│         [📞]            │ ← 80x80 闪烁图标
│                         │
│    「呼叫方名称」         │ ← 22px 卡片
│                         │
│   正在呼叫您...          │ ← 15px 绿色
│                         │
│   [✕]      [✓]          │ ← 75x75 圆形
│   拒绝      接听         │ ← 文字说明
│                         │
└─────────────────────────┘
```

### 3. Emoji 图标使用

```
📞 - 电话（来电图标、挂断按钮）
⏱ - 计时器（通话时长）
🔗 - 链接（连接中状态）
🔇 - 静音图标
📹 - 摄像头图标
✕ - 拒绝符号
✓ - 接听符号
```

---

## 💡 核心代码片段

### 渐变背景示例

```cpp
// 线性渐变（垂直）
setStyleSheet(
    "QWidget { "
    "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
    "       stop:0 #2d2d2d, stop:1 #232323);"
    "}"
);

// 径向渐变（从中心向外）
setStyleSheet(
    "QWidget { "
    "   background: qradialgradient(cx:0.5, cy:0.5, radius:1, "
    "       fx:0.5, fy:0.5, stop:0 #1a1a1a, stop:1 #000000);"
    "}"
);
```

### 圆形按钮示例

```cpp
// 挂断按钮（85x85 圆形）
hangup_button_ = new QPushButton("📞", this);
hangup_button_->setFixedSize(85, 85);
hangup_button_->setStyleSheet(
    "QPushButton {"
    "   background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
    "       stop:0 #e74c3c, stop:1 #c0392b);"
    "   border-radius: 42px;"  // 半径 = 尺寸/2
    "   border: 3px solid #a93226;"
    "   box-shadow: 0 6px 12px rgba(231, 76, 60, 0.4);"
    "}"
);
```

### 闪烁动画示例

```cpp
// 启动闪烁定时器
void IncomingCallDialog::startBlinkAnimation() {
    blink_timer_ = new QTimer(this);
    connect(blink_timer_, &QTimer::timeout, 
            this, &IncomingCallDialog::updateBlinkAnimation);
    blink_timer_->start(600);  // 每600ms切换
}

// 更新闪烁状态
void IncomingCallDialog::updateBlinkAnimation() {
    blink_state_ = !blink_state_;
    if (blink_state_) {
        // 亮起：增加透明度和边框
        icon_label_->setStyleSheet(
            "border: 3px solid rgba(76, 175, 80, 0.8);"
        );
    } else {
        // 暗淡：减少透明度和边框
        icon_label_->setStyleSheet(
            "border: 2px solid rgba(76, 175, 80, 0.4);"
        );
    }
}
```

---

## 🎯 使用建议

### 何时使用 VideoCallDialog
- ✅ 一对一视频通话
- ✅ 需要显示本地和远程视频
- ✅ 需要通话控制（静音、摄像头、挂断）
- ✅ 需要显示通话时长

### 何时使用 IncomingCallDialog
- ✅ 接收来电通知
- ✅ 需要用户确认接听/拒绝
- ✅ 需要醒目的视觉提醒
- ✅ 需要自动超时处理（30秒）

---

## 🔧 自定义调整

### 修改闪烁速度
```cpp
// 在 IncomingCallDialog::startBlinkAnimation()
blink_timer_->start(600);  // 改为 400 更快，800 更慢
```

### 修改超时时间
```cpp
// 在 IncomingCallDialog.h
static constexpr int RING_TIMEOUT_MS = 30000;  // 改为其他值
```

### 修改按钮尺寸
```cpp
// 在 VideoCallDialog::setupUI()
hangup_button_->setFixedSize(100, 100);  // 改为更大
// 别忘了修改 border-radius 为 50（尺寸的一半）
```

### 修改主题色
```cpp
// 替换所有 #e74c3c (红色) 为你的颜色
// 替换所有 #4CAF50 (绿色) 为你的颜色
// 使用编辑器的批量替换功能
```

---

## 📱 响应式建议

虽然当前实现是固定尺寸，如果需要响应式：

```cpp
// 可以在 resizeEvent 中动态调整
void VideoCallDialog::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);
    
    // 根据窗口大小调整本地视频尺寸
    int width = this->width();
    if (width < 600) {
        local_renderer_->setFixedSize(160, 120);
    } else {
        local_renderer_->setFixedSize(220, 165);
    }
    
    layoutLocalVideo();
}
```

---

## 🌟 特色功能

### 1. 自动布局本地视频
- 始终在右下角
- 自动适应窗口大小
- 始终在最上层（raise()）

### 2. 来电闪烁提醒
- 600ms 周期性闪烁
- 接听/拒绝后自动停止
- 超时后自动停止

### 3. 按钮悬停效果
- 渐变背景变化
- 轻微放大（1.1x）
- 平滑过渡

### 4. 清晰的视觉层次
- 主操作按钮最大（挂断 85x85）
- 次要操作中等（静音/摄像头 70x70）
- 信息展示有卡片背景

---

## 📋 测试清单

美化后请测试以下场景：

- [ ] VideoCallDialog 显示正常
- [ ] 本地视频窗口位置正确（右下角）
- [ ] 远程视频全屏显示
- [ ] 通话时长显示正确
- [ ] 挂断按钮点击有效
- [ ] 静音/摄像头按钮可点击
- [ ] IncomingCallDialog 显示正常
- [ ] 来电图标闪烁正常
- [ ] 接听按钮功能正常
- [ ] 拒绝按钮功能正常
- [ ] 30秒超时自动拒绝
- [ ] 所有按钮悬停效果正常
- [ ] 高DPI显示正常
- [ ] 不同分辨率下显示正常

---

## 📚 相关文档

- [详细美化总结](UI_ENHANCEMENT_SUMMARY.md)
- [视频通话实现总结](VIDEO_CALL_IMPLEMENTATION_SUMMARY.md)
- [视频通话集成指南](VIDEO_CALL_INTEGRATION.md)

---

**快速参考版本**：v1.0  
**最后更新**：2025年10月22日
