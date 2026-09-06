# cmake toolchain file for Linux 64-bit compilation
# usage: cmake ../pjt -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_linux.cmake

set(CMAKE_SYSTEM_NAME Linux)

# Detect host architecture automatically
cmake_host_system_information(RESULT _host_arch QUERY OS_PLATFORM)
set(CMAKE_SYSTEM_PROCESSOR "${_host_arch}")

# Use default compiler (GCC)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -std=c++17")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "")

# Qt6 is located via find_package(Qt6 ...) in the project's own CMakeLists.txt;
# no hard-coded paths here. Do NOT set CMAKE_MAKE_PROGRAM — build.sh passes
# -G Ninja, which sets it. Keep this file minimal: it covers compiler and
# platform settings only.
