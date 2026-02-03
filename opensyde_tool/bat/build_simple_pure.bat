@echo off
set "PROJECT_ROOT=C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\SYDEflash"
set "BUILD_DIR=C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\SYDEflash\build"
set "TOOLCHAIN=C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\toolchain_windows.cmake"
set "NINJA_EXE=C:\Qt\Tools\Ninja\ninja.exe"
set "PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\6.10.1\mingw_64;%PATH%"

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cmake "%PROJECT_ROOT%" -S "%PROJECT_ROOT%" -B "%BUILD_DIR%" -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN%" -DCMAKE_MAKE_PROGRAM="%NINJA_EXE%" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.1/mingw_64" -DQt6_DIR="C:/Qt/6.10.1/mingw_64/lib/cmake/Qt6"

cmake --build "%BUILD_DIR%" --target all -j24