cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

project(NetherLink-Client VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Network WebSockets Sql Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network WebSockets Sql Concurrent)

# --- WebRTC 预构建库配置 ---
set(WEBRTC_PREBUILT_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/webrtc.windows_x86_64/webrtc"
    CACHE PATH "Path to the vendored prebuilt WebRTC package")
set(WEBRTC_INCLUDE_DIR "${WEBRTC_PREBUILT_ROOT}/include")
set(WEBRTC_LIB "${WEBRTC_PREBUILT_ROOT}/lib/webrtc.lib")

if(NOT EXISTS "${WEBRTC_INCLUDE_DIR}")
    message(FATAL_ERROR "WebRTC include directory not found at: ${WEBRTC_INCLUDE_DIR}")
endif()
if(NOT EXISTS "${WEBRTC_LIB}")
    message(FATAL_ERROR "WebRTC library not found at: ${WEBRTC_LIB}")
endif()

# 强制使用静态运行时库 /MT，与预构建 WebRTC 保持一致
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# 为所有配置强制设置 /MT 标志
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /MT")

# WebRTC 编译选项和定义
if(MSVC)
    add_compile_options(
        /Zc:__cplusplus
        /EHsc
    )
endif()

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

# 创建可执行文件
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(NetherLink-Client
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

target_include_directories(NetherLink-Client PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${WEBRTC_INCLUDE_DIR}"
    "${WEBRTC_INCLUDE_DIR}/third_party/abseil-cpp"
    "${WEBRTC_INCLUDE_DIR}/third_party/jsoncpp/source/include"
    "${WEBRTC_INCLUDE_DIR}/third_party/libyuv/include"
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

target_link_libraries(NetherLink-Client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebSockets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Concurrent
    "${WEBRTC_LIB}"
    ${SYSTEM_LIBRARIES}
    dwmapi
    user32
)

if(MSVC)
    target_compile_options(NetherLink-Client PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(NetherLink-Client PRIVATE
        -Wall
        -Wextra
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
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/bin/RelWithDebInfo"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_CURRENT_SOURCE_DIR}/bin/MinSizeRel"
)

install(TARGETS NetherLink-Client
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(NetherLink-Client)
endif()
