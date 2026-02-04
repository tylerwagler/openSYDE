@echo off
rem Build all openSYDE components
rem Usage: build_all.bat [Release|Debug]

set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

echo.
echo ========================================
echo Building ALL openSYDE Components
echo Build Type: %BUILD_TYPE%
echo ========================================
echo.

rem Build main GUI application
echo.
echo ========================================
echo [1/3] Building openSYDE GUI
echo ========================================
call build_%BUILD_TYPE%.bat
if ERRORLEVEL 1 (
    echo.
    echo ERROR: openSYDE GUI build failed
    exit /B 1
)

rem Build CAN Monitor
echo.
echo ========================================
echo [2/3] Building CAN Monitor
echo ========================================
call build_can_monitor_%BUILD_TYPE%.bat
if ERRORLEVEL 1 (
    echo.
    echo ERROR: CAN Monitor build failed
    exit /B 1
)

rem Build SYDEflash
echo.
echo ========================================
echo [3/3] Building SYDEflash
echo ========================================
call build_syde_flash_%BUILD_TYPE%.bat
if ERRORLEVEL 1 (
    echo.
    echo ERROR: SYDEflash build failed
    exit /B 1
)

echo.
echo ========================================
echo ALL BUILDS COMPLETED SUCCESSFULLY
echo ========================================
echo.
echo Build artifacts are in: opensyde_tool\result\
echo.
