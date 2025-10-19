/* include ---------------------------------------------------------------- 80 // ! ----------------------------- 120 */

#include <QApplication>
#include <QFontDatabase>
#include <QPixmapCache>

#include "View/Mainwindow/Login.h"

#include <windows.h> // 包含 Windows API 头文件
#include <shellapi.h>  // must come after windows.h // 包含 shellapi.h，必须在 windows.h 之后
// clang-format on // 重新启用 clang-format

#include <cstddef> // 包含 cstddef 头文件
#include <cstdio> // 包含 cstdio 头文件
#include <cwchar> // 包含 cwchar 头文件
#include <memory> // 包含 memory 头文件
#include <string> // 包含 string 头文件
#include <vector> // 包含 vector 头文件

#ifdef emit
#undef emit
#define QT_NO_EMIT_DEFINED
#endif
#include "absl/flags/flag.h" // 包含 absl flags 头文件
#include "absl/flags/parse.h" // 包含 absl parse 头文件
#include "api/environment/environment.h" // 包含 WebRTC 环境头文件
#include "api/environment/environment_factory.h" // 包含环境工厂头文件
#include "api/field_trials.h" // 包含字段试验头文件
#include "api/make_ref_counted.h" // 包含 make_ref_counted 头文件
#include "WebRTC/conductor.h" // 包含 conductor 头文件
#include "WebRTC/flag_defs.h" // 包含 flag 定义头文件
#include "WebRTC/mainwindow.h" // 包含主窗口头文件
#include "WebRTC/signalclient.h" // 包含信号客户端头文件
#include "WebRTC/callmanager.h" // 包含呼叫管理器头文件
#include "rtc_base/checks.h" // 包含检查头文件
#include "rtc_base/physical_socket_server.h" // 包含物理 socket 服务器头文件
#include "rtc_base/ssl_adapter.h" // 包含 SSL 适配器头文件
#include "rtc_base/string_utils.h"  // For ToUtf8 // 包含字符串工具头文件，用于 ToUtf8
#include "rtc_base/thread.h" // 包含线程头文件
#include "rtc_base/win32_socket_init.h" // 包含 Win32 socket 初始化头文件
#ifdef QT_NO_EMIT_DEFINED
#define emit
#undef QT_NO_EMIT_DEFINED
#endif

#include <QApplication> // 包含 Qt 应用程序头文件
#include <QTimer> // 包含 Qt 定时器头文件

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

int main(int argc, char*argv[]) {
    webrtc::WinsockInitializer winsock_init;
    webrtc::PhysicalSocketServer ss;
    webrtc::AutoSocketServerThread main_thread(&ss);

    absl::ParseCommandLine(argc, argv);

    webrtc::Environment env = webrtc::CreateEnvironment(); // 创建 WebRTC 环境，使用默认配置

    // Abort if the user specifies a port that is outside the allowed
    // range [1, 65535].
    if ((absl::GetFlag(FLAGS_port) < 1) || (absl::GetFlag(FLAGS_port) > 65535)) {
        printf("Error: %i is not a valid port.\n", absl::GetFlag(FLAGS_port));
        return -1;
    }
    QApplication a(argc, argv);
    QApplication::setAttribute(Qt::AA_Use96Dpi, true);
    QPixmapCache::setCacheLimit(20480);

    QPalette palette;
    palette.setColor(QPalette::WindowText, Qt::black);
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::PlaceholderText, QColor(0x808080));
    palette.setColor(QPalette::ToolTipText, Qt::black);

    QApplication::setPalette(palette);

    // Login l;

    // l.show();

      // Initialize SSL
    webrtc::InitializeSSL(); // 初始化 SSL

    // Create main window
    MainWnd main_wnd; // 创建主窗口
    main_wnd.show(); // 显示主窗口

    // Create conductor
    auto conductor = std::make_unique<Conductor>(env, &main_wnd); // 创建 conductor
    main_wnd.SetConductor(conductor.get()); // 设置 conductor

    // Register conductor as observer
    SignalClient* signal_client = main_wnd.GetSignalClient(); // 获取信号客户端
    CallManager* call_manager = main_wnd.GetCallManager(); // 获取呼叫管理器
    
    signal_client->RegisterObserver(conductor.get()); // 注册观察者
    call_manager->RegisterObserver(conductor.get()); // 注册观察者

    // Initialize conductor
    if (!conductor->Initialize()) { // 初始化 conductor
        printf("Failed to initialize conductor\n"); // 打印失败
        return -1; // 返回错误
    }

    // Use QTimer to process WebRTC messages
    QTimer timer; // 创建定时器
    QObject::connect(&timer, &QTimer::timeout, []() { // 连接超时信号
        MSG msg; // 消息结构体
        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { // 循环处理消息
        ::TranslateMessage(&msg); // 翻译消息
        ::DispatchMessage(&msg); // 分发消息
        }
    });
    timer.start(10);  // Process messages every 10ms // 每 10ms 处理一次消息

    // Run Qt event loop
    int result = a.exec(); // 运行 Qt 事件循环

    // Cleanup resources before exit
    timer.stop(); // 停止定时器
    
    // Shutdown conductor (releases WebRTC resources)
    if (conductor) { // 如果 conductor 存在
        conductor->Shutdown(); // 关闭 conductor
    }
    
    // Clear unique_ptr to ensure destructor is called
    conductor.reset(); // 重置 unique_ptr，确保析构函数被调用

    // Cleanup WebRTC SSL
    webrtc::CleanupSSL(); // 清理 SSL
    
    return result; // 返回结果
}
