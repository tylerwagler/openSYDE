rem build SYDEflash application

set PROJECT_FOLDER=..\pjt\SYDEflash
set TEMP_FOLDER=..\temp_SYDEflash_Release
set BUILD_TARGET=Release
set TOOLCHAIN=../pjt/toolchain_windows.cmake

rem Create logs directory if it doesn't exist
if not exist logs mkdir logs

rem Generate timestamp for log file
set TIMESTAMP=%date:~10,4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%

rem Run build and save output to log
echo Build output will be saved to: logs\build_syde_flash_%TIMESTAMP%.log
call "%~dp0\build_with_cmake.bat" %PROJECT_FOLDER% %TEMP_FOLDER% %BUILD_TARGET% %TOOLCHAIN% > logs\build_syde_flash_%TIMESTAMP%.log 2>&1

rem Show result
if ERRORLEVEL 1 (
    echo Build FAILED - see logs\build_syde_flash_%TIMESTAMP%.log for details
    exit /B 1
) else (
    echo Build SUCCESS - log saved to: logs\build_syde_flash_%TIMESTAMP%.log
)
