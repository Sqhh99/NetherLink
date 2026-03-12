@echo off
setlocal EnableExtensions EnableDelayedExpansion

for %%I in ("%~dp0.") do set "SCRIPT_DIR=%%~fI"
cd /d "%SCRIPT_DIR%"

set "BUILD_FLAVOR=%~1"
if "%BUILD_FLAVOR%"=="" set "BUILD_FLAVOR=release"

if /I "%BUILD_FLAVOR%"=="release" (
    set "CMAKE_CONFIG=Release"
) else if /I "%BUILD_FLAVOR%"=="debug" (
    set "CMAKE_CONFIG=Debug"
) else (
    echo Usage: build.cmd [release^|debug]
    exit /b 1
)

if not defined WEBRTC_ROOT set "WEBRTC_ROOT=%SCRIPT_DIR%\third_party\webrtc.windows_x86_64\webrtc"
if not defined QT_BIN_DIR (
    if defined QT_ROOT_DIR (
        set "QT_BIN_DIR=%QT_ROOT_DIR%\bin"
    ) else (
        set "QT_BIN_DIR=D:\Qt\6.10.0\msvc2022_64\bin"
    )
)
set "QT_ROOT=%QT_BIN_DIR%\.."
for %%I in ("%QT_ROOT%") do set "QT_ROOT=%%~fI"
set "BUILD_DIR=%SCRIPT_DIR%\build\ninja-%BUILD_FLAVOR%"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%WEBRTC_ROOT%\include" (
    echo Missing WebRTC headers: "%WEBRTC_ROOT%\include"
    exit /b 1
)

if not exist "%WEBRTC_ROOT%\lib\webrtc.lib" (
    echo Missing WebRTC library: "%WEBRTC_ROOT%\lib\webrtc.lib"
    exit /b 1
)

if not exist "%QT_BIN_DIR%\qmake.exe" (
    echo Qt was not found at "%QT_BIN_DIR%".
    exit /b 1
)

if not exist "%QT_ROOT%\lib\cmake\Qt6\Qt6Config.cmake" (
    echo Qt6Config.cmake was not found under "%QT_ROOT%".
    exit /b 1
)

where cmake >nul 2>&1
if errorlevel 1 (
    echo CMake was not found in PATH.
    exit /b 1
)

where ninja >nul 2>&1
if errorlevel 1 (
    echo Ninja was not found in PATH.
    exit /b 1
)

call :init_msvc
if errorlevel 1 exit /b 1

set "PATH=%QT_BIN_DIR%;%PATH%"
echo Using Qt: %QT_ROOT%
echo Using generator: Ninja
echo Using compiler: cl.exe

cmake --fresh -S "%SCRIPT_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=%CMAKE_CONFIG% -DCMAKE_PREFIX_PATH="%QT_ROOT%" -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe
if errorlevel 1 exit /b 1

cmake --build "%BUILD_DIR%"
if errorlevel 1 exit /b 1

echo Build succeeded.
echo Output: "%SCRIPT_DIR%\bin\%CMAKE_CONFIG%\NetherLink-Client.exe"
exit /b 0

:init_msvc
if not exist "%VSWHERE%" (
    echo vswhere.exe was not found at "%VSWHERE%".
    exit /b 1
)

set "VS_INSTALL_DIR="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VS_INSTALL_DIR=%%I"
)

if not defined VS_INSTALL_DIR (
    echo Visual Studio 2022 with MSVC tools was not found.
    exit /b 1
)

if not exist "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvars64.bat" (
    echo vcvars64.bat was not found under "%VS_INSTALL_DIR%".
    exit /b 1
)

call "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo Failed to initialize the MSVC build environment.
    exit /b 1
)

where cl >nul 2>&1
if errorlevel 1 (
    echo cl.exe was not found after initializing MSVC.
    exit /b 1
)

exit /b 0
