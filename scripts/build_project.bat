@echo off
setlocal enabledelayedexpansion

REM ============================================================
REM FlipGearboxFactoryTest - Automated Build Script
REM ============================================================
REM Usage: build_project.bat [Debug|Release] [clean]
REM
REM Supports:
REM   - Auto-detection of Qt installation (D:\Qt, E:\Qt, F:\Qt)
REM   - Multiple build configurations (Debug/Release)
REM   - Parallel build with all CPU cores
REM   - Optional clean build
REM ============================================================

set "BUILD_TYPE=Debug"
set "DO_CLEAN=0"
set "QT_DIR="
set "QT_VERSION=6.11.0"
set "QT_COMPILER=mingw_64"

REM Parse arguments
if not "%~1"=="" (
    if /I "%~1"=="Release" set "BUILD_TYPE=Release"
    if /I "%~1"=="Debug" set "BUILD_TYPE=Debug"
    if /I "%~1"=="clean" set "DO_CLEAN=1"
)
if not "%~2"=="" (
    if /I "%~2"=="clean" set "DO_CLEAN=1"
)

echo ========================================
echo FlipGearboxFactoryTest - Build Script
echo ========================================
echo Build Type: %BUILD_TYPE%
echo Clean Build: %DO_CLEAN%
echo.

REM Auto-detect Qt installation
set "QT_SEARCH_PATHS=D:\Qt;%USERPROFILE%\Qt;C:\Qt;E:\Qt;F:\Qt"
for %%P in (%QT_SEARCH_PATHS%) do (
    if exist "%%P\%QT_VERSION%\%QT_COMPILER%\bin\qmake.exe" (
        set "QT_DIR=%%P\%QT_VERSION%\%QT_COMPILER%"
        echo [OK] Found Qt at: !QT_DIR!
        goto :qt_found
    )
)

REM Fallback: Check environment variable
if defined QT_DIR (
    if exist "%QT_DIR%\bin\qmake.exe" (
        echo [OK] Using Qt from environment variable: %QT_DIR%
        goto :qt_found
    )
)

echo [ERROR] Qt %QT_VERSION% with %QT_COMPILER% not found.
echo         Searched in: %QT_SEARCH_PATHS%
echo         Please install Qt or set QT_DIR environment variable.
exit /b 1

:qt_found
set "PATH=%QT_DIR%\bin;%QT_DIR%\..\..\Tools\mingw1310_64\bin;%PATH%"
set "CMAKE_PREFIX_PATH=%QT_DIR%"

REM Create build directory
set "BUILD_DIR=build\Desktop_Qt_%QT_VERSION:.=_%_%QT_COMPILER%_%BUILD_TYPE%"
if %DO_CLEAN%==1 (
    echo [*] Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /S /Q "%BUILD_DIR%"
)

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo.
echo [*] Configuring CMake...
cmake -S . -B "%BUILD_DIR%" ^
    -G "MinGW Makefiles" ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_PREFIX_PATH="%CMAKE_PREFIX_PATH%" ^
    -DCMAKE_CXX_COMPILER="%QT_DIR%\..\..\Tools\mingw1310_64\bin\g++.exe" ^
    -DCMAKE_C_COMPILER="%QT_DIR%\..\..\Tools\mingw1310_64\bin\gcc.exe"

if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake configuration failed!
    exit /b 1
)

echo.
echo [*] Building project with all CPU cores...
cmake --build "%BUILD_DIR%" --parallel %NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed!
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo Output: %BUILD_DIR%
echo ========================================
exit /b 0
