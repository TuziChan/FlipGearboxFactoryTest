@echo off
set PATH=F:\Qt\6.11.0\llvm-mingw_64\bin;F:\Qt\Tools\llvm-mingw1706_64\bin;%PATH%
cd /d "%~dp0build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug"
appFlipGearboxFactoryTest.exe > test_run.log 2>&1
