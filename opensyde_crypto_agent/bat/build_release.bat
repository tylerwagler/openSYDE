rem delete build folder and build using cmake
rem as a side effect this will also create the required configuration files for PC-lint
@echo off
SET CWD=%~dp0


if exist ..\temp_windows rmdir /s /q ..\temp_windows
if not exist ..\temp_windows md ..\temp_windows

rem build using cmake
rem as a side effect this will also create the required configuration files for PC-lint
cd ..\temp_windows

rem set path to C,C++,resource compilers
SET PATH=C:\Qt\Qt6.8.3\Tools\mingw1310_64\bin\;%PATH%
rem set path to cmake
SET PATH=C:\Qt\Qt6.8.3\Tools\CMake_64\bin;%PATH%
rem set path to ninja
SET PATH=C:\Qt\Qt6.8.3\Tools\Ninja;%PATH%

rem run cmake
cmake.exe ..\pjt -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_windows.cmake
if ERRORLEVEL 1 exit /B 1

rem perform actual build
cmake.exe --build . --target all -- -j4
if ERRORLEVEL 1 exit /B 1

rem copy resulting binary to result:
cmake.exe --build . --target install
if ERRORLEVEL 1 exit /B 1

cd ..\bat
