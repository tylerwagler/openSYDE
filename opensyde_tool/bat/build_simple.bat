@echo off
REM ==============================================================
REM Build SYDEflash using explicit Qt toolchain paths
REM ==============================================================

REM Change to the project directory (absolute path)
pushd "C:\Users\tyler\Dev\repos\openSYDE\opensyde_tool\pjt\SYDEflash"

REM Add required toolchain directories to PATH
set "PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\6.10.1\mingw_64;%PATH%"

REM Configure the project with CMake (Ninja generator)
C:\Qt\Tools\CMake_64\bin\cmake.exe . -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=..\pjt\toolchain_windows.cmake

REM Build the project
C:\Qt\Tools\CMake_64\bin\cmake.exe --build . --target all -j24

REM Return to the original directory
popd