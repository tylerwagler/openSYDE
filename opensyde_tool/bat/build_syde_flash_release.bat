rem build SYDEflash application

set PROJECT_FOLDER=..\pjt\SYDEflash
set TEMP_FOLDER=..\temp_SYDEflash_Release
set BUILD_TARGET=Release
set TOOLCHAIN=../pjt/toolchain_windows.cmake

rem Generate timestamp for log file
set TIMESTAMP=%date:~10,4%%date:~4,2%%date:~7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set TIMESTAMP=%TIMESTAMP: =0%

rem Run build and save output to log
call build_with_cmake %PROJECT_FOLDER% %TEMP_FOLDER% %BUILD_TARGET% %TOOLCHAIN% > build_syde_flash_%TIMESTAMP%.log 2>&1
