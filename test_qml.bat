@echo off
REM 测试 QML 加载是否正常

echo 测试 QML 组件加载...
echo.

REM 设置Qt环境变量
set PATH=F:\Qt\6.11.0\llvm-mingw_64\bin;F:\Qt\Tools\llvm-mingw1706_64\bin;%PATH%

REM 切换到构建目录
cd /d "%~dp0build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug"

REM 运行程序（3秒后自动关闭）
echo 启动程序（将在3秒后自动关闭）...
timeout /t 3 /nobreak | appFlipGearboxFactoryTest.exe 2>&1

echo.
echo 测试完成！
pause
