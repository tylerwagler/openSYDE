@echo off
REM ==============================================================
REM Build SYDEflash using absolute paths – no relative lookup issues
REM ==============================================================

REM -----------------------------------------------------------------
REM 1) Environment setup – add Qt toolchain directories to PATH
REM -----------------------------------------------------------------
set "PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\6.10.1\mingw_64;%PATH%"

REM -----------------------------------------------------------------
REM 2) Absolute paths to key locations
REM -----------------------------------------------------------------
set "PROJECT_ROOT=C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\SYDEflash"
set "BUILD_DIR=%PROJECT_ROOT%\build"
set "TOOLCHAIN=C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\toolchain_windows.cmake"

REM -----------------------------------------------------------------
REM 3) Create build directory (clean start)
REM -----------------------------------------------------------------
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM -----------------------------------------------------------------
REM 4) Configure the project with CMake
REM    -G Ninja  → generate Ninja build files
REM    -DCMAKE_TOOLCHAIN_FILE → use the provided toolchain
REM    -DCMAKE_MAKE_PROGRAM → explicit Ninja executable location
REM -----------------------------------------------------------------
cmake "%PROJECT_ROOT%" ^
    -S "%PROJECT_ROOT%" ^
    -B "%BUILD_DIR%" ^
    -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN%" ^
    -DCMAKE_MAKE_PROGRAM="%ProgramFiles%\Ninja\ninja.exe"

REM -----------------------------------------------------------------
REM 5) Build the project
REM -----------------------------------------------------------------
cmake --build "%BUILD_DIR%" --target all -j24

REM -----------------------------------------------------------------
REM 6) Done – show a brief summary
REM -----------------------------------------------------------------
echo.
echo ==============================
echo Build finished – binaries are in:
echo   %BUILD_DIR%\bin
echo ==============================
echo.