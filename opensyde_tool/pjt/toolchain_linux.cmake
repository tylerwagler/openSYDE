# cmake toolchain file for Linux 64-bit compilation
# usage: cmake ../pjt -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_linux.cmake 

# Let CMake auto-detect the system
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Use default compiler (GCC)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -std=c++17")

# Enable C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Linker flags
set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "")

# Use Qt6's own CMake config system instead of hard-coded paths
# This relies on find_package(Qt6 ...) working properly
# which requires Qt6 to be installed and pkg-config or CMake config files available

# For deployment, use linuxdeployqt or manual copying
# Do not use static linking on Linux - it's not recommended for Qt applications

# Do NOT call find_package(Qt6 ...) here - let the project's CMakeLists.txt do it
# The project already has find_package(Qt6 ...) and set(CMAKE_AUTOMOC ON) etc.

# Disable Windows-specific features
# (Already handled by CMakeLists.txt)

# Do NOT set CMAKE_MAKE_PROGRAM here - let CMake auto-detect
# The build.sh script will use -G Ninja, which sets it properly

# This toolchain file is only for compiler and platform settings
# All build system settings are handled by the command line

# IMPORTANT: This file should be as minimal as possible
# because the project's CMakeLists.txt handles everything else
