# cmake toolchain file for ubuntu compilation; this should also be of for native builds on other linux distributions
# usage: cmake ../pjt -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_ubuntu.cmake 

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR amd64)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

set(CMAKE_C_FLAGS_INIT "-Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized -Wsign-compare \
    -Wdouble-promotion -Wmissing-include-dirs -Wshadow -Wlogical-op -Wno-multichar -Wredundant-decls")
set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized -Wsign-compare \
    -Wdouble-promotion -Wmissing-include-dirs -Wshadow -Wlogical-op -Wno-multichar -Wredundant-decls -std=c++03")

set(LNX_LINK_OPENSSL_STATIC ON CACHE BOOL "Linking the openSSL library statically")

# link standard libraries statically for release
# Note that a statically linked OpenSSL will still require some glibc functions to be called dynamically.
# When building this will be reported through linker warnings like e.g.
#  "warning: Using 'getaddrinfo' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking"
# This creates a version dependency, even though we link statically.
# To see which version of the glibc is required, you can deactivate static linking (here and also for OpenSSL),
#  then call "ldd -v <executable>" and check the version of libc.so.
#  Newer versions are backwards compatible, so the output should be read as >= the version shown in the output.
set(CMAKE_EXE_LINKER_FLAGS_RELEASE_INIT "-static-libgcc -static-libstdc++ -static")

# install paths
set(CMAKE_INSTALL_BINDIR ${PROJECT_SOURCE_DIR}/../result/${INSTALL_SUBDIR} CACHE PATH "Installing path")