@echo off
setlocal enabledelayedexpansion

REM ============================================
REM Auto-detect Qt installation
REM ============================================

set "QT_ROOT="
set "QT_VERSION=6.11.0"
set "QT_COMPILER=llvm-mingw_64"

echo [*] Searching for Qt installation...

REM Search in common drives (C to F)
for %%D in (C D E F) do (
    if exist "%%D:\Qt\%QT_VERSION%\%QT_COMPILER%\bin" (
        set "QT_ROOT=%%D:\Qt"
        echo [OK] Found Qt at: !QT_ROOT!
        goto :qt_found
    )
)

REM If not found, show error
echo [ERROR] Qt %QT_VERSION% with %QT_COMPILER% not found in C:, D:, E:, or F: drives.
echo         Please install Qt or update the script with the correct path.
exit /b 1

:qt_found

set "PATH=%QT_ROOT%\%QT_VERSION%\%QT_COMPILER%\bin;%QT_ROOT%\Tools\llvm-mingw1706_64\bin;%%PATH%%"
cd /d "%%~dp0build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug"
appFlipGearboxFactoryTest.exe > test_run.log 2>&1
