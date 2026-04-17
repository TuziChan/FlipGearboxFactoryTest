@echo off
chcp 65001 >nul 2>&1
setlocal enabledelayedexpansion

REM 齿轮箱产线测试系统 - 构建与启动脚本

set "QT_BIN=F:\Qt\6.11.0\llvm-mingw_64\bin"
set "MINGW_BIN=F:\Qt\Tools\llvm-mingw1706_64\bin"
set "PATH=%QT_BIN%;%MINGW_BIN%;%PATH%"

set "PROJECT_ROOT=%~dp0"
set "BUILD_DIR=%PROJECT_ROOT%build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug"
set "EXE_NAME=appFlipGearboxFactoryTest.exe"

echo ============================================
echo   齿轮箱产线测试系统 - 构建与启动
echo ============================================
echo.

REM 检查并关闭正在运行的旧进程
tasklist /FI "IMAGENAME eq %EXE_NAME%" 2>nul | find /i "%EXE_NAME%" >nul
if %errorlevel% equ 0 (
    echo [!] 检测到 %EXE_NAME% 正在运行，正在关闭旧进程...
    taskkill /F /IM "%EXE_NAME%" >nul 2>&1
    timeout /t 2 /nobreak >nul
    echo [√] 旧进程已关闭
    echo.
)

REM 检查构建目录是否存在
if not exist "%BUILD_DIR%" (
    echo [×] 构建目录不存在: %BUILD_DIR%
    echo     请先通过 Qt Creator 执行初始配置。
    pause
    exit /b 1
)

REM 执行构建
echo [*] 开始构建...
echo.
cmake --build "%BUILD_DIR%" 2>&1

if %errorlevel% neq 0 (
    echo.
    echo [×] 构建失败！请检查上方错误日志。
    pause
    exit /b 1
)

echo.
echo [√] 构建成功！
echo.

REM 启动程序
echo [*] 启动程序...
echo.
cd /d "%BUILD_DIR%"
start "" "%EXE_NAME%"

echo [√] 程序已启动！
echo.
echo 注意：
echo   1. 未连接实际硬件时串口连接会失败（正常现象）
echo   2. UI界面可正常使用和测试
echo   3. 关闭窗口即可退出程序
echo.

pause
