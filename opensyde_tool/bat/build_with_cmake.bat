rem build cmake application
rem parameters
rem %1 Relative project folder path
rem %2 Relative temporary folder
rem %3 Build target (Release/Debug)
rem %4 Toolchain file
rem build using cmake
rem as a side effect this will also create the required configuration files for PC-lint
@echo off
SET CWD=%~dp0

rem Validate parameters
if "%1"=="" (
    echo ERROR: Missing project folder parameter
    echo Usage: build_with_cmake.bat ^<project_folder^> ^<temp_folder^> ^<build_type^> ^<toolchain_file^>
    exit /B 1
)

echo ========================================
echo Building: %1
echo Build type: %3
echo Temp folder: %2
echo ========================================
echo.

rem Check Qt installation
if not exist "C:\Qt\6.10.1\mingw_64" (
    echo ERROR: Qt 6.10.1 MinGW not found at C:\Qt\6.10.1\mingw_64
    echo Please install Qt 6.10.1 with MinGW 64-bit component
    exit /B 1
)

if not exist "C:\Qt\Tools\CMake_64\bin\cmake.exe" (
    echo ERROR: CMake not found at C:\Qt\Tools\CMake_64
    echo Please install Qt Tools (CMake, Ninja, MinGW)
    exit /B 1
)

if not exist %2 (
    echo Creating build directory: %2
    md %2
)

cd %2
rem set path to C,C++,resource compilers
SET PATH=C:\Qt\Tools\mingw1310_64\bin\;%PATH%
rem set path to cmake
SET PATH=C:\Qt\Tools\CMake_64\bin;%PATH%
rem set path to ninja
SET PATH=C:\Qt\Tools\Ninja;%PATH%
rem set path to cmake Qt utilities
SET PATH=c:\Qt\6.10.1\mingw_64\;%PATH%

rem only run cmake configure if build.ninja does not exist
if not exist build.ninja (
   echo [STEP 1/3] Running CMake configure...
   cmake.exe %1 -GNinja -DCMAKE_BUILD_TYPE=%3 -DCMAKE_TOOLCHAIN_FILE=%4
   if ERRORLEVEL 1 (
       echo ERROR: CMake configure failed
       cd ..\bat
       exit /B 1
   )
   echo CMake configure completed successfully
   echo.
) else (
   echo [STEP 1/3] Using existing CMake configuration (delete build.ninja to reconfigure)
   echo.
)

rem perform actual build
echo [STEP 2/3] Building with Ninja (parallel jobs: 64)...
cmake.exe --build . --target all -- -j64
if ERRORLEVEL 1 (
    echo ERROR: Build failed
    cd ..\bat
    exit /B 1
)
echo Build completed successfully
echo.

rem copy resulting binary to result:
echo [STEP 3/3] Installing binaries to result folder...
cmake.exe --build . --target install
if ERRORLEVEL 1 (
    echo ERROR: Install failed
    cd ..\bat
    exit /B 1
)
echo Install completed successfully
echo.

cd ..\bat

echo ========================================
echo BUILD SUCCESS
echo ========================================
exit /B 0
