# cmake toolchain file for ubuntu 64 bit compilation
# usage: cmake ../pjt -DCMAKE_TOOLCHAIN_FILE=../pjt/toolchain_ubuntu.cmake 

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR amd64)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_C_FLAGS "-m64 -Wno-deprecated-declarations -fPIC")
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-m64 -Wno-deprecated-declarations -fPIC")

set(CMAKE_SHARED_LINKER_FLAGS "-m64")

# Dynamically link OpenSSL — openSUSE and most modern distros don't ship
# static libcrypto/libssl by default.
set(LNX_LINK_OPENSSL_STATIC OFF CACHE BOOL "Linking the openSSL library static")
set(INSTALL_SYDESUP_LIB ON CACHE BOOL "Installing SYDEsup lib to the result")

# install paths
set(CMAKE_INSTALL_BINDIR ${PROJECT_SOURCE_DIR}/../result/${INSTALL_SUBDIR} CACHE PATH "Installing path")