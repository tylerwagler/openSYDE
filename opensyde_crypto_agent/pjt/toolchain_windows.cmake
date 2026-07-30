# cmake toolchain file for Windows compilation
# usage: cmake ../pjt -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_windows.cmake

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i386)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_C_FLAGS_INIT "-Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized -Wsign-compare \
    -Wdouble-promotion -Wmissing-include-dirs -Wshadow -Wlogical-op -Wno-multichar -Wredundant-decls")
set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized -Wsign-compare \
    -Wdouble-promotion -Wmissing-include-dirs -Wshadow -Wlogical-op -Wno-multichar -Wredundant-decls -std=c++03")

# add coverage measurement but only for debug builds:
set(CMAKE_CXX_FLAGS_DEBUG_INIT "${CMAKE_CXX_FLAGS_DEBUG_INIT} -coverage -fprofile-arcs -ftest-coverage")

# link standard libraries statically
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++ -static")

# set installation directory to place binary in result/ folder in project root
set(CMAKE_INSTALL_BINDIR ${PROJECT_SOURCE_DIR}/../result/ CACHE PATH "Installation path")
