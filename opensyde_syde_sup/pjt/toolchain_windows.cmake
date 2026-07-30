# cmake toolchain file for Windows 32 bit compilation
# usage: cmake ../pjt -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_windows.cmake 

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i386)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_FLAGS_INIT "-m32 -Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized -Wsign-compare \
    -Wdouble-promotion -Wmissing-include-dirs -Wshadow -Wlogical-op -Wno-multichar -Wredundant-decls")
set(CMAKE_CXX_FLAGS_INIT "-m32 -Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized -Wsign-compare \
    -Wdouble-promotion -Wmissing-include-dirs -Wshadow -Wlogical-op -Wno-multichar -Wredundant-decls -std=c++03")

set(CMAKE_SHARED_LINKER_FLAGS_INIT "-m32")

# link standard libraries statically
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++ -static")

# install paths
set(CMAKE_INSTALL_BINDIR ${PROJECT_SOURCE_DIR}/../result/${INSTALL_SUBDIR} CACHE PATH "Installing path")