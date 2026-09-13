# CMake toolchain file for a native Windows build with LLVM-MinGW clang.
#
# clang is the project's unifying compiler across Linux, macOS and Windows.
# On Windows we use the LLVM-MinGW distribution: clang++ with the GNU/MinGW ABI
# and libc++ (the same standard library macOS uses), targeting
# x86_64-w64-windows-gnu. It pairs with Qt's llvm-mingw_64 kit.
#
# Like the Linux and macOS toolchains this only names the compiler and the
# standard; the environment supplies the rest -- the compiler and flex/bison come
# from PATH, Qt from Qt6_DIR/CMAKE_PREFIX_PATH, OpenSSL from OPENSSL_ROOT_DIR.
# zlib is built from source by the root CMakeLists on Windows (LLVM-MinGW, unlike
# MinGW-GCC, ships no zlib, and borrowing GCC's would drag libstdc++ headers into
# a libc++ build).
#
# Deliberately does NOT set CMAKE_SYSTEM_NAME: on a native Windows host CMake
# already reports Windows (and WIN32=TRUE, which the root CMakeLists and the
# tools' WIN32 branches key on); setting it forces CMAKE_CROSSCOMPILING and
# breaks find_package. See the note in toolchain_macos.cmake.

set(CMAKE_C_COMPILER   clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_FLAGS   "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
