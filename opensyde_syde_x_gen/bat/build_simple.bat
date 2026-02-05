<arg_value>@echo off
SET CWD=%~dp0
if not exist ..\temp md ..\temp
del /s /f /q ..\temp\*.*
cd ..\temp
SET PATH=C:\Qt\Qt6.10.1\mingw_64\bin;%PATH%
SET PATH=C:\Qt\Qt6.10.1\Tools\CMake_64\bin;%PATH%
SET PATH=C:\Qt\Qt6.10.1\Tools\Ninja;%PATH%
cmake.exe ..\pjt -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_windows_64.cmake
if ERRORLEVEL 1 goto done
cmake.exe --build . --target all
if ERRORLEVEL 1 goto done
cmake.exe --build . --target install
if ERRORLEVEL 1 goto done
cd ..\bat
:done"