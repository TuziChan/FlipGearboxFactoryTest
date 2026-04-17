@echo off
echo ========================================
echo 齿轮箱产线测试系统 - 启动测试
echo ========================================
echo.

REM 设置Qt环境变量
set PATH=F:\Qt\6.11.0\llvm-mingw_64\bin;F:\Qt\Tools\llvm-mingw1706_64\bin;%PATH%

REM 切换到构建目录
cd /d "%~dp0build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug"

echo 正在启动程序...
echo.
echo 如果程序正常启动，你应该看到：
echo 1. QML debugging 提示信息
echo 2. 设备初始化信息（串口连接可能失败，这是正常的）
echo 3. 应用程序窗口
echo.
echo ========================================
echo.

REM 启动程序
appFlipGearboxFactoryTest.exe

echo.
echo ========================================
echo 程序已退出
echo ========================================
pause
