cmake_minimum_required(VERSION 3.10)

project(NetherLink-Client VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Sql Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network WebSockets Sql Concurrent)

# --- WebRTC 路径配置 ---
set(WEBRTC_SRC_DIR "D:/webrtc-checkout/src")
set(WEBRTC_LIB_DIR "${WEBRTC_SRC_DIR}/out/Release/obj")
set(ABSEIL_INSTALL_DIR "${WEBRTC_SRC_DIR}/third_party/abseil-cpp/build_static/install")
set(JSONCPP_INSTALL_DIR "${WEBRTC_SRC_DIR}/third_party/jsoncpp/build_static/install")

# --- 检查WebRTC路径 ---
if(NOT EXISTS "${WEBRTC_SRC_DIR}")
    message(FATAL_ERROR "WebRTC source directory not found at: ${WEBRTC_SRC_DIR}")
endif()
if(NOT EXISTS "${WEBRTC_LIB_DIR}")
    message(FATAL_ERROR "WebRTC library directory not found at: ${WEBRTC_LIB_DIR}")
endif()

set(MAIN_WEBRTC_LIB_PATH "${WEBRTC_LIB_DIR}/webrtc.lib")
if(NOT EXISTS "${MAIN_WEBRTC_LIB_PATH}")
    message(FATAL_ERROR "The main library webrtc.lib was not found at: ${MAIN_WEBRTC_LIB_PATH}")
endif()

# 检查 JsonCpp 库
set(JSONCPP_LIB_PATH "${JSONCPP_INSTALL_DIR}/lib/jsoncpp.lib")
if(NOT EXISTS "${JSONCPP_LIB_PATH}")
    message(FATAL_ERROR "JsonCpp library not found at: ${JSONCPP_LIB_PATH}")
endif()

# 强制使用静态运行时库 /MT (与 WebRTC 一致)
# 注意: Qt 也需要用 /MT 编译,或者重新编译 WebRTC 为 /MD
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# 为所有配置强制设置 /MT 标志
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")

# WebRTC 编译选项和定义
add_compile_options(
    /Zc:__cplusplus  # 启用正确的 __cplusplus 宏
    /EHsc  # 异常处理
)

add_definitions(
    -DWIN32_LEAN_AND_MEAN 
    -DNOMINMAX
    -DWEBRTC_WIN 
    -D_CRT_SECURE_NO_WARNINGS 
    -D_SCL_SECURE_NO_WARNINGS
    -DWEBRTC_LITTLE_ENDIAN 
    -D_WIN32_WINNT=0x0A00 
    -D_HAS_ITERATOR_DEBUGGING=0
    -D_ITERATOR_DEBUG_LEVEL=0
    -DWEBRTC_LIBRARY_IMPL
    -DWEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=1
    -DUNICODE
    -D_UNICODE
)

# 收集所有源文件和头文件
file(GLOB_RECURSE INCLUDES "include/*.h" "include/**/*.h")
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/**/*.cpp" "src/*.cc" "src/**/*.cc")

include_directories(SYSTEM 
    ${WEBRTC_SRC_DIR}
    "${WEBRTC_SRC_DIR}/third_party/abseil-cpp"
    "${WEBRTC_SRC_DIR}/third_party/libyuv/include"
    "${JSONCPP_INSTALL_DIR}/include"
    "${WEBRTC_SRC_DIR}/api"
    "${WEBRTC_SRC_DIR}/rtc_base"
    "${WEBRTC_SRC_DIR}/media"
    "${WEBRTC_SRC_DIR}/p2p"
    "${WEBRTC_SRC_DIR}/pc"
)

# 创建可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NetherLink-Client WIN32
        MANUAL_FINALIZATION
        ${INCLUDES}
        ${SOURCES}
        res/res.qrc
    )
else()
    add_executable(NetherLink-Client
        ${INCLUDES}
        ${SOURCES}
        res/res.qrc
    )
endif()

# 配置包含目录
target_include_directories(NetherLink-Client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 配置链接目录
link_directories(
    "D:/webrtc-checkout/src/out/Release/obj"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib"
    "D:/webrtc-checkout/src/third_party/jsoncpp/build_static/install/lib"
)

# WebRTC 的必需库
set(WEBRTC_EXTRA_LIBS
    "D:/webrtc-checkout/src/out/Release/obj/api/field_trials.lib"
)

# Abseil 库
set(ABSEIL_LIBRARIES
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_parse.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_usage.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_usage_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_config.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_marshalling.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_commandlineflag.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_commandlineflag_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_private_handle_accessor.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_reflection.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_flags_program_name.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_internal_message.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_internal_check_op.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_internal_conditions.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_internal_log_sink_set.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_internal_format.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_internal_globals.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_globals.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_entry.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_severity.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_log_sink.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_stacktrace.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_symbolize.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_debugging_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_demangle_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_synchronization.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_graphcycles_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_kernel_timeout_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_time.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_time_zone.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_civil_time.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_strings.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_strings_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_str_format_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_string_view.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_cord.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_cord_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_cordz_functions.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_cordz_handle.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_cordz_info.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_base.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_malloc_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_raw_logging_internal.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_spinlock_wait.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_throw_delegate.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_int128.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_city.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_hash.lib"
    "D:/webrtc-checkout/src/third_party/abseil-cpp/build_static/install/lib/absl_raw_hash_set.lib"
)

# Windows 系统库
set(SYSTEM_LIBRARIES
    ws2_32 
    secur32 
    winmm 
    iphlpapi 
    advapi32 
    user32 
    crypt32 
    dmoguids 
    msdmo
    strmiids
    wmcodecdspuuid
    comctl32
    gdi32
    comdlg32
    ole32
    shell32
)

# 链接库
target_link_libraries(NetherLink-Client PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::WebSockets
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Concurrent
        "D:/webrtc-checkout/src/out/Release/obj/webrtc.lib"
        "D:/webrtc-checkout/src/third_party/jsoncpp/build_static/install/lib/jsoncpp.lib"
        ${WEBRTC_EXTRA_LIBS}
        ${ABSEIL_LIBRARIES}
        ${SYSTEM_LIBRARIES}
        dwmapi
        user32
)

if(MSVC)
    # MSVC 编译选项
    target_compile_options(NetherLink-Client PRIVATE
        /W4       # 对应 -Wall 和 -Wextra
        /permissive- # 对应 -pedantic
        # /WX       # 对应 -Werror
    )
else()
    # GCC/Clang 编译选项
    target_compile_options(NetherLink-Client PRIVATE
        -Wall
        -Wextra
        # -Werror
        -Wbuiltin-macro-redefined
        -Wunused-variable
        -Wunused-function
        -pedantic
        -O3
    )
endif()

set_target_properties(NetherLink-Client PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS NetherLink-Client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NetherLink-Client)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/Debug")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug")
else(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/Release")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release")
endif()

set_target_properties(NetherLink-Client PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:CONSOLE"
)