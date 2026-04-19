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

set "QT_BIN=%QT_ROOT%\%QT_VERSION%\%QT_COMPILER%\bin"
set "MINGW_BIN=%QT_ROOT%\Tools\llvm-mingw1706_64\bin"
set "PATH=%QT_BIN%;%MINGW_BIN%;%PATH%"

set "PROJECT_ROOT=%~dp0"
set "BUILD_DIR=%PROJECT_ROOT%build\Desktop_Qt_6_11_0_llvm_mingw_64_bit-Debug"
set "EXE_NAME=appFlipGearboxFactoryTest.exe"

echo ============================================
echo   FlipGearboxFactoryTest - Build and Run
echo ============================================
echo.

REM Check for command line argument
set "ACTION=%~1"
if "%ACTION%"=="" set "ACTION=run"

if "%ACTION%"=="build" goto :build_only
if "%ACTION%"=="run" goto :build_and_run
if "%ACTION%"=="clean" goto :clean

echo [ERROR] Unknown action: %ACTION%
echo Usage: run.bat [build^|run^|clean]
echo   build - Only build the project
echo   run   - Build and run (default)
echo   clean - Clean build directory
exit /b 1

:clean
echo [*] Cleaning build directory...
if exist "%BUILD_DIR%" (
    rmdir /s /q "%BUILD_DIR%"
    echo [OK] Build directory cleaned.
) else (
    echo [!] Build directory does not exist.
)
exit /b 0

:build_only
if not exist "%BUILD_DIR%" (
    echo [ERROR] Build directory does not exist: %BUILD_DIR%
    echo         Run the initial CMake configure step in Qt Creator first.
    exit /b 1
)

echo [*] Building...
echo.
cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo [OK] Build succeeded.
exit /b 0

:build_and_run
tasklist /FI "IMAGENAME eq %EXE_NAME%" 2>nul | find /i "%EXE_NAME%" >nul
if %errorlevel% equ 0 (
    echo [!] Closing existing %EXE_NAME% process...
    taskkill /F /IM "%EXE_NAME%" >nul 2>&1
    timeout /t 2 /nobreak >nul
    echo [OK] Existing process closed.
    echo.
)

if not exist "%BUILD_DIR%" (
    echo [ERROR] Build directory does not exist: %BUILD_DIR%
    echo         Run the initial CMake configure step in Qt Creator first.
    exit /b 1
)

echo [*] Building...
echo.
cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo [OK] Build succeeded.
echo.

echo [*] Launching application...
echo.
pushd "%BUILD_DIR%"
start "" "%EXE_NAME%"
popd

echo [OK] Application launched.
echo.
echo Notes:
echo   1. Serial port connection errors are expected without hardware.
echo   2. The UI can still be opened and tested.
echo   3. Close the app window to exit.
echo.

exit /b 0
