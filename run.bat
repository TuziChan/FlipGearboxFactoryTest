@echo off
REM 齿轮箱产线测试系统启动脚本

echo 正在启动齿轮箱产线测试系统...
echo.

REM 设置Qt环境变量
set PATH=F:\Qt\6.11.0\llvm-mingw_64\bin;F:\Qt\Tools\llvm-mingw1706_64\bin;%PATH%

REM 切换到构建目录
cd /d "%~dp0build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug"

REM 启动程序
start appFlipGearboxFactoryTest.exe

echo 程序已启动！
echo.
echo 注意：
echo 1. 如果没有连接实际硬件，串口连接会失败（这是正常的）
echo 2. UI界面可以正常使用和测试
echo 3. 按Ctrl+C或关闭窗口退出程序
echo.

pause
