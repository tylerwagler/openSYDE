@echo off
rem openSYDE Unified Build Script (Windows)
rem
rem Usage: build.bat [/t tool] [/b Release|Debug] [/c] [/j N]
rem
rem Tools: opensyde, canmonitor, sydeflash, sydesup, syde_x_gen, syde_coder_c, flash_tool, tsp_convert, all
rem Default: build all tools in Release mode
rem
rem Options:
rem   /t tool    Tool to build (repeat for multiple, or "all")
rem   /b type    Build type: Release or Debug (default: Release)
rem   /c         Clean build directory before building
rem   /j N       Parallel jobs (default: 4)
rem
rem Examples:
rem   build.bat                          Build all tools (Release)
rem   build.bat /t opensyde /t sydesup   Build openSYDE and SYDEsup
rem   build.bat /b Debug /t canmonitor   Debug build of CAN Monitor
rem   build.bat /c /t all                Clean rebuild of everything

setlocal enabledelayedexpansion

rem --- Qt / Toolchain Configuration ---
set QT_ROOT=C:\Qt\6.11.0
set MINGW_BIN=%QT_ROOT%\Tools\mingw1310_64\bin
set CMAKE_BIN=%QT_ROOT%\Tools\CMake_64\bin
set NINJA_BIN=%QT_ROOT%\Tools\Ninja
set QT_PREFIX=%QT_ROOT%\mingw_64

rem --- Defaults ---
set BUILD_TYPE=Release
set CLEAN=0
set JOBS=4
set TOOL_COUNT=0

rem --- Parse arguments ---
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="/t" (
    set /a TOOL_COUNT+=1
    set "TOOL_!TOOL_COUNT!=%~2"
    shift
    shift
    goto parse_args
)
if /i "%~1"=="/b" (
    set BUILD_TYPE=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="/c" (
    set CLEAN=1
    shift
    goto parse_args
)
if /i "%~1"=="/j" (
    set JOBS=%~2
    shift
    shift
    goto parse_args
)
if /i "%~1"=="/h" goto usage
if /i "%~1"=="/?" goto usage
echo Unknown option: %~1
goto usage
:args_done

rem Default to all if no tools specified
if %TOOL_COUNT%==0 (
    set TOOL_COUNT=1
    set TOOL_1=all
)

rem --- Set up PATH ---
SET PATH=%MINGW_BIN%;%CMAKE_BIN%;%NINJA_BIN%;%QT_PREFIX%;%PATH%

echo ========================================
echo   openSYDE Build System (Windows)
echo ========================================
echo   Build type: %BUILD_TYPE%
echo   Clean:      %CLEAN%
echo   Jobs:       %JOBS%
echo.

rem --- Dispatch builds ---
set FAILED=0
for /L %%i in (1,1,%TOOL_COUNT%) do (
    set "TOOL=!TOOL_%%i!"
    if /i "!TOOL!"=="all" (
        call :build_tool opensyde
        call :build_tool canmonitor
        call :build_tool sydeflash
        call :build_tool sydesup
        call :build_tool syde_x_gen
        call :build_tool syde_coder_c
        call :build_tool flash_tool
        call :build_tool tsp_convert
    ) else (
        call :build_tool !TOOL!
    )
)

echo.
echo ========================================
echo   Build Summary
echo ========================================
echo   Results: %~dp0result\%BUILD_TYPE%\
if %FAILED%==0 (
    echo   Status:  All tools built successfully
) else (
    echo   Status:  Some tools failed
    exit /B 1
)
exit /B 0

rem --- Build a single tool ---
:build_tool
set "TOOL_NAME=%~1"
set "PJT_DIR="
set "TOOLCHAIN="
set "BUILD_SUBDIR=%TOOL_NAME%"

if /i "%TOOL_NAME%"=="opensyde" (
    set PJT_DIR=opensyde_tool\pjt\openSYDE
    set TOOLCHAIN=pjt\toolchains\windows_x64.cmake
    set BUILD_SUBDIR=openSYDE
)
if /i "%TOOL_NAME%"=="canmonitor" (
    set PJT_DIR=opensyde_can_monitor\pjt
    set TOOLCHAIN=pjt\toolchains\windows_x64.cmake
    set BUILD_SUBDIR=openSYDE_CAN_Monitor
)
if /i "%TOOL_NAME%"=="sydeflash" (
    set PJT_DIR=opensyde_syde_flash\pjt
    set TOOLCHAIN=pjt\toolchains\windows_x64.cmake
    set BUILD_SUBDIR=SYDEflash
)
if /i "%TOOL_NAME%"=="sydesup" (
    set PJT_DIR=opensyde_syde_sup\pjt
    set TOOLCHAIN=pjt\toolchains\windows_x64.cmake
    set BUILD_SUBDIR=SYDEsup
)
if /i "%TOOL_NAME%"=="syde_x_gen" (
    set PJT_DIR=opensyde_syde_x_gen\pjt
    set TOOLCHAIN=pjt\toolchains\windows_x64.cmake
    set BUILD_SUBDIR=syde_x_gen
)
if /i "%TOOL_NAME%"=="syde_coder_c" (
    set PJT_DIR=opensyde_syde_coder_c\pjt
    set BUILD_SUBDIR=syde_coder_c
)
if /i "%TOOL_NAME%"=="flash_tool" (
    set PJT_DIR=opensyde_cmd_line_flash_tool\pjt
    set BUILD_SUBDIR=cmd_line_flash_tool
)
if /i "%TOOL_NAME%"=="tsp_convert" (
    set PJT_DIR=opensyde_tsp_convert\pjt
    set BUILD_SUBDIR=tsp_convert
)

if "%PJT_DIR%"=="" (
    echo [ERROR] Unknown tool: %TOOL_NAME%
    echo         Available: opensyde, canmonitor, sydeflash, sydesup, syde_x_gen, syde_coder_c, flash_tool, tsp_convert
    set FAILED=1
    exit /B 1
)

set "BUILD_DIR=%~dp0build\%BUILD_TYPE%\%BUILD_SUBDIR%"

echo ========================================
echo   Building %TOOL_NAME% (%BUILD_TYPE%)
echo ========================================

if %CLEAN%==1 (
    if exist "%BUILD_DIR%" (
        echo [BUILD] Cleaning %BUILD_DIR%...
        rd /s /q "%BUILD_DIR%"
    )
)
if not exist "%BUILD_DIR%" md "%BUILD_DIR%"

pushd "%BUILD_DIR%"

rem Configure
echo [BUILD] Configuring...
set CMAKE_ARGS=%~dp0%PJT_DIR% -GNinja -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if not "%TOOLCHAIN%"=="" (
    set CMAKE_ARGS=!CMAKE_ARGS! -DCMAKE_TOOLCHAIN_FILE=%~dp0%TOOLCHAIN%
)
cmake.exe !CMAKE_ARGS!
if ERRORLEVEL 1 (
    echo [ERROR] Configure failed for %TOOL_NAME%
    set FAILED=1
    popd
    exit /B 1
)

rem Build
echo [BUILD] Building (jobs=%JOBS%)...
cmake.exe --build . --target all -- -j%JOBS%
if ERRORLEVEL 1 (
    echo [ERROR] Build failed for %TOOL_NAME%
    set FAILED=1
    popd
    exit /B 1
)

rem Install
echo [BUILD] Installing...
cmake.exe --build . --target install
if ERRORLEVEL 1 (
    echo [ERROR] Install failed for %TOOL_NAME%
    set FAILED=1
    popd
    exit /B 1
)

popd
echo [BUILD] %TOOL_NAME% built successfully
echo.
exit /B 0

:usage
echo Usage: build.bat [/t tool] [/b Release^|Debug] [/c] [/j N]
echo.
echo Tools: opensyde, canmonitor, sydeflash, sydesup, syde_x_gen, syde_coder_c, flash_tool, tsp_convert, all
echo.
echo Options:
echo   /t tool    Tool to build (repeat for multiple, or "all")
echo   /b type    Build type: Release or Debug (default: Release)
echo   /c         Clean build directory before building
echo   /j N       Parallel jobs (default: 4)
exit /B 0
