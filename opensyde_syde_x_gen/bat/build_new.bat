<arg_value>rem build using cmake
rem as a side effect this will also create the required configuration files for PC-lint
@echo off
SET CWD=%~dp0
if not exist ..\temp md ..\temp

rem clean folder ...
del /s /f /q ..\temp\*.*

cd ..\temp
rem set path to C,C++,resource compilers
SET PATH=C:\Qt\Qt6.10.1\mingw_64\bin;%PATH%
rem set path to cmake
SET PATH=C:\Qt\Qt6.10.1\Tools\CMake_64\bin;%PATH%
rem set path to ninja
SET PATH=C:\Qt\Qt6.10.1\Tools\Ninja;%PATH%

rem run cmake
cmake.exe ..\pjt -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_windows_64.cmake
if ERRORLEVEL 1 goto done

rem perform actual build
cmake.exe --build . --target all
if ERRORLEVEL 1 goto done

rem copy resulting binary to result:
cmake.exe --build . --target install
if ERRORLEVEL 1 goto done

cd ..\bat

:done"