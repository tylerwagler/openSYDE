@echo off
setlocal

rem ------------------------------------------------------------
rem 1) Add Qt toolchain directories to PATH
rem ------------------------------------------------------------
set "PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\6.10.1\mingw_64;%PATH%"

rem ------------------------------------------------------------
rem 2) Absolute paths to key locations
rem ------------------------------------------------------------
set "PROJECT_ROOT=C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\SYDEflash"
set "BUILD_DIR=C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\SYDEflash\build"
set "TOOLCHAIN=C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\toolchain_windows.cmake"
set "NINJA_EXE=C:\Qt\Tools\Ninja\ninja.exe"

rem ------------------------------------------------------------
rem 3) Create build directory (clean start)
rem ------------------------------------------------------------
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

rem ------------------------------------------------------------
rem 4) Configure the project with CMake
rem    -G Ninja  → generate Ninja build files
rem    -DCMAKE_TOOLCHAIN_FILE → use the provided toolchain
rem    -DCMAKE_MAKE_PROGRAM → explicit Ninja executable location
rem ------------------------------------------------------------
cmake "%PROJECT_ROOT%" ^
    -S "%PROJECT_ROOT%" ^
    -B "%BUILD_DIR%" ^
    -G "Ninja" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN%" ^
    -DCMAKE_MAKE_PROGRAM="%NINJA_EXE%" ^
    -DCMAKE_PREFIX_PATH="C:/Qt/6.10.1/mingw_64" ^
    -DQt6_DIR="C:/Qt/6.10.1/mingw_64/lib/cmake/Qt6"

rem ------------------------------------------------------------
rem 5) Build the project
rem ------------------------------------------------------------
cmake --build "%BUILD_DIR%" --target all -j24

rem ------------------------------------------------------------
rem 6) Done – show a brief summary
rem ------------------------------------------------------------
echo.
echo ==============================
echo Build finished – binaries are in:
echo   %BUILD_DIR%\bin
echo ==============================
echo.
endlocal