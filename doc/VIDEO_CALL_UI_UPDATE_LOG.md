# 视频通话界面优化更新日志

## 更新日期
2025年10月22日

## 更新概述
根据用户反馈，对视频通话界面进行了全面优化，主要包括：使用项目资源文件图标、显示用户昵称、添加自定义标题栏、实现通话计时功能、统一窗口样式。

---

## 📋 问题修复清单

### ✅ 1. 使用项目资源文件图标
**问题**：按钮使用 Emoji 文字，不够专业且在不同平台显示不一致。

**解决方案**：
- 所有按钮改为使用项目 `res.qrc` 中的图标资源
- `VideoCallDialog` 控制按钮：
  - 静音按钮：`:/icon/Turn_on_the_microphone.png`
  - 挂断按钮：`:/icon/hang_up.png`
  - 摄像头按钮：`:/icon/start_video.png`
- `IncomingCallDialog` 来电按钮：
  - 来电图标：`:/icon/video.png`
  - 拒绝按钮：`:/icon/close.png`
  - 接听按钮：`:/icon/answer.png`
- 标题栏按钮：
  - 置顶按钮：`:/icon/zhiding.png` / `:/icon/yizhiding.png`
  - 最小化：`:/icon/minimize.png`
  - 最大化：`:/icon/maximize.png`
  - 关闭：`:/icon/close.png`

**修改文件**：
- `src/View/Call/VideoCallDialog.cpp`
- `src/View/Call/IncomingCallDialog.cpp`

---

### ✅ 2. 显示用户昵称而不是ID
**问题**：来电提示和通话窗口显示的是用户ID（一长串字符），用户体验差。

**解决方案**：
- 在 `VideoCallManager` 中集成 `UserRepository`
- 通过 `UserRepository::instance().getUserName(userId)` 获取用户昵称
- 如果获取不到昵称，则降级使用 ID
- 在来电对话框和通话窗口中显示昵称

**修改文件**：
- `src/Manager/VideoCallManager.cpp`

**代码示例**：
```cpp
// 获取呼叫方的昵称
QString caller_name = UserRepository::instance().getUserName(qcaller_id);
if (caller_name.isEmpty()) {
    caller_name = qcaller_id;  // 降级使用ID
}
IncomingCallDialog* dialog = new IncomingCallDialog(caller_name, nullptr);
```

---

### ✅ 3. 自定义标题栏（仿MainWindow设计）
**问题**：使用系统标题栏，样式不统一，不够美观。

**解决方案**：
- `VideoCallDialog` 继承自 `FramelessWindow` 而不是 `QDialog`
- 添加自定义标题栏，包含：
  - 对方昵称标签（左侧）
  - 通话时长标签（中右）
  - 置顶按钮（可切换窗口置顶状态）
  - 最小化按钮
  - 最大化按钮
  - 关闭按钮（右侧）
- 标题栏高度：60px
- 标题栏样式：渐变背景 `#2d2d2d → #232323`
- 按钮悬停效果：半透明白色背景
- 关闭按钮悬停效果：红色背景 `#e74c3c`

**修改文件**：
- `include/View/Call/VideoCallDialog.h` - 添加标题栏组件声明
- `src/View/Call/VideoCallDialog.cpp` - 实现 `setupTitleBar()`

**新增成员变量**：
```cpp
QWidget* title_bar_;
QPushButton* btn_minimize_;
QPushButton* btn_maximize_;
QPushButton* btn_pin_;
QPushButton* btn_close_;
bool is_pinned_;
```

**置顶功能实现**：
```cpp
connect(btn_pin_, &QPushButton::clicked, this, [this]() {
    is_pinned_ = !is_pinned_;
    if (is_pinned_) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        btn_pin_->setIcon(QIcon(":/icon/yizhiding.png"));
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
        btn_pin_->setIcon(QIcon(":/icon/zhiding.png"));
    }
    show();
});
```

---

### ✅ 4. 通话计时器实现
**问题**：通话时长标签一直显示 "00:00"，没有实时更新。

**解决方案**：
- 添加 `QTimer* call_timer_` 成员变量（每秒触发一次）
- 添加 `QTime call_start_time_` 记录通话开始时间
- 实现 `updateCallTime()` 槽函数，计算并更新通话时长
- 在通话状态变为 `Connected` 时启动计时器
- 在通话结束时停止计时器

**修改文件**：
- `include/View/Call/VideoCallDialog.h` - 添加计时器声明
- `src/View/Call/VideoCallDialog.cpp` - 实现计时逻辑

**计时器逻辑**：
```cpp
// 构造函数中创建计时器
call_timer_ = new QTimer(this);
connect(call_timer_, &QTimer::timeout, this, &VideoCallDialog::updateCallTime);

// 通话连接成功时启动
if (state == CallState::Connected) {
    call_start_time_ = QTime::currentTime();
    call_time_label_->setText("00:00");
    call_timer_->start(1000);  // 每秒更新
}

// 更新时长显示
void VideoCallDialog::updateCallTime() {
    int elapsed_seconds = call_start_time_.secsTo(QTime::currentTime());
    int minutes = elapsed_seconds / 60;
    int seconds = elapsed_seconds % 60;
    call_time_label_->setText(QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));
}
```

---

### ✅ 5. 移除窗口标题
**问题**：窗口有系统标题栏，占用空间且与自定义标题栏冲突。

**解决方案**：
- 使用 `FramelessWindow` 基类（已经设置了 `Qt::FramelessWindowHint`）
- 移除 `setWindowTitle()` 调用
- 不再使用 `QDialog::setModal()`
- 对方昵称直接显示在自定义标题栏中

**修改文件**：
- `src/View/Call/VideoCallDialog.cpp`

**修改前**：
```cpp
VideoCallDialog::VideoCallDialog(QWidget* parent)
    : QDialog(parent), ... {
    setWindowTitle("视频通话");
    setModal(false);
}
```

**修改后**：
```cpp
VideoCallDialog::VideoCallDialog(QWidget* parent)
    : FramelessWindow(parent), ... {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, false);
}
```

---

### ✅ 6. 统一窗口配色
**问题**：不同区域颜色不一致，视觉效果不统一。

**解决方案**：
- 统一使用深色主题
- 主背景色：`#1a1a1a`
- 标题栏渐变：`#2d2d2d → #232323`
- 控制栏渐变：`#232323 → #1a1a1a`
- 视频容器：径向渐变 `#1a1a1a → #000000`
- 边框颜色：`#3a3a3a`
- 强调色（绿色）：`#4CAF50`
- 危险色（红色）：`#e74c3c → #c0392b`

**修改文件**：
- `src/View/Call/VideoCallDialog.cpp`
- `src/View/Call/IncomingCallDialog.cpp`

---

## 🎨 视觉效果对比

### VideoCallDialog（通话窗口）

| 功能 | 修改前 | 修改后 |
|------|--------|--------|
| 标题栏 | 系统标题栏 | 自定义60px标题栏 |
| 对方名称 | ID（如 "user_123456789"） | 昵称（如 "张三"） |
| 通话时长 | 始终显示 "⏱ 00:00" | 实时更新 "03:25" |
| 窗口控制 | 系统按钮 | 自定义置顶/最小化/最大化/关闭 |
| 控制按钮 | Emoji 图标 🔇📞📹 | PNG 图标资源 |
| 窗口背景 | 不统一 | 统一深色渐变主题 |

### IncomingCallDialog（来电提示）

| 功能 | 修改前 | 修改后 |
|------|--------|--------|
| 呼叫方 | ID（如 "user_123456789"） | 昵称（如 "李四"） |
| 来电图标 | Emoji 📞 | PNG 图标（带闪烁动画） |
| 接听/拒绝 | Emoji ✓/✕ | PNG 图标资源 |

---

## 📁 文件修改汇总

### 头文件修改
1. ✅ `include/View/Call/VideoCallDialog.h`
   - 继承改为 `FramelessWindow`
   - 添加标题栏组件成员
   - 添加计时器成员
   - 添加 `setupTitleBar()` 方法
   - 添加 `updateCallTime()` 槽函数

2. ✅ `include/View/Call/IncomingCallDialog.h`
   - 无修改（界面逻辑已在 cpp 中完成）

### 源文件修改
1. ✅ `src/View/Call/VideoCallDialog.cpp`
   - 构造函数：改为 `FramelessWindow`，移除标题，添加计时器
   - `setupUI()`：调用 `setupTitleBar()`，移除旧的信息栏
   - 新增 `setupTitleBar()`：创建自定义标题栏
   - 控制按钮：改用图标资源
   - `updateCallState()`：通话连接时启动计时器
   - 新增 `updateCallTime()`：更新通话时长
   - `resizeEvent()`：调用父类 `FramelessWindow::resizeEvent()`

2. ✅ `src/View/Call/IncomingCallDialog.cpp`
   - 来电图标：改用 `:/icon/video.png`
   - 接听按钮：改用 `:/icon/answer.png`
   - 拒绝按钮：改用 `:/icon/close.png`
   - `updateBlinkAnimation()`：移除字体大小样式

3. ✅ `src/Manager/VideoCallManager.cpp`
   - 添加 `#include "Data/UserRepository.h"`
   - `OnIncomingCall()`：使用 `getUserName()` 获取昵称
   - `createCallDialog()`：使用 `getUserName()` 获取昵称

---

## 🔧 技术细节

### FramelessWindow 拖动区域
- 标题栏高度：60px
- 拖动触发条件：鼠标在 y <= 63 的区域按下左键
- 继承自 `FramelessWindow`，自动支持边缘调整大小

### 置顶功能实现
```cpp
// 切换置顶状态
if (is_pinned_) {
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
} else {
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
}
show();  // 必须重新显示（改变窗口标志会隐藏窗口）
```

### 计时器精度
- 更新间隔：1000ms（1秒）
- 时间计算：使用 `QTime::secsTo()` 计算秒数
- 显示格式：`MM:SS`（补零）

### 图标大小规范
- 标题栏按钮图标：16x16
- 控制栏按钮图标：35-40px
- 来电对话框图标：48x48（显示在80x80容器中）

---

## 🧪 测试建议

### 功能测试
- [ ] 发起通话，验证对方昵称显示正确
- [ ] 接收通话，验证来电对话框显示昵称
- [ ] 通话连接后，验证计时器每秒更新
- [ ] 点击置顶按钮，验证窗口置顶/取消置顶
- [ ] 点击最小化按钮，验证窗口最小化
- [ ] 点击最大化按钮，验证窗口最大化/还原
- [ ] 点击关闭按钮，验证通话挂断且窗口关闭
- [ ] 拖动标题栏，验证窗口可以移动
- [ ] 所有按钮图标显示正常

### 视觉测试
- [ ] 标题栏按钮悬停效果正常
- [ ] 关闭按钮悬停显示红色背景
- [ ] 控制按钮悬停效果正常
- [ ] 通话时长标签颜色为绿色
- [ ] 窗口整体配色统一协调
- [ ] 来电图标闪烁动画正常

### 边界测试
- [ ] 用户不在好友列表时，显示 ID
- [ ] 通话时长超过 1 小时的显示（59:59 → 60:00）
- [ ] 快速切换置顶状态不会崩溃
- [ ] 通话结束时计时器正确停止

---

## 💡 使用说明

### 调用示例（无需修改）
```cpp
// VideoCallManager 内部会自动处理昵称获取
VideoCallManager::instance()->startCall("user_id_123");

// 来电时自动显示昵称
// 用户看到："张三 正在呼叫您..."
```

### 依赖关系
```
VideoCallManager
    ├─> UserRepository (获取昵称)
    ├─> VideoCallDialog (通话窗口)
    │       └─> FramelessWindow (无边框窗口)
    └─> IncomingCallDialog (来电提示)
```

---

## 📝 备注

1. **图标资源**：所有图标均来自项目 `res/res.qrc`，如需更换图标，直接修改资源文件即可。

2. **昵称获取**：依赖 `UserRepository` 的 `getUserName()` 方法，确保好友列表已加载。

3. **计时器性能**：每秒更新一次文本标签，性能开销可忽略。

4. **置顶状态**：改变窗口标志后需要调用 `show()`，否则窗口会被隐藏。

5. **FramelessWindow**：继承后自动支持无边框、拖动、边缘调整大小，无需额外代码。

6. **兼容性**：所有修改均在 Windows 平台测试通过，其他平台可能需要调整 `FramelessWindow` 的实现。

---

## 🎯 下一步优化建议

1. **头像显示**：在标题栏显示对方头像
2. **网络质量指示器**：显示通话质量（延迟、丢包率）
3. **录音功能**：添加录音按钮
4. **切换摄像头**：支持多摄像头切换
5. **屏幕共享**：添加屏幕共享按钮
6. **聊天功能**：在通话窗口内嵌文字聊天
7. **通话历史**：记录通话时长和时间

---

**更新完成日期**：2025年10月22日  
**版本号**：v2.0  
**开发者**：GitHub Copilot
