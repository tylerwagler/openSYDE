# 3.25 tree-wide: the root CMakeLists requires it, and a standalone configure of one tool should not accept less
cmake_minimum_required(VERSION 3.25)

project(osy_crypto_agent LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/osy_tool_common.cmake)

#include script to generate PC-lint configuration files
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/lint_config.cmake)

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)

# --- PROJECT INCLUDE PATHS --------------------------------------------------------------------------------------------
# Set include directories before dragging in core; the core needs the "precomp_headers.hpp" and files included by it
include_directories(
   ${PROJECT_ROOT}/src
)
if(WIN32)
   include_directories(${PROJECT_ROOT}/src/windows)
else()
   include_directories(${PROJECT_ROOT}/src/linux)
endif()
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# OPENSYDE CORE
# The daemon is the server side of the crypto agent: it loads a PEM folder and answers
# RSA-1024 challenge requests. It needs the security group (PEM database + RSA) and the
# zipping group that the core's SECURITY dependency requires. The client-side
# crypto-agent access util is NOT needed here (the daemon is not a client), so it is
# skipped. Everything network/protocol/code-generation related is skipped.
set(OPENSYDE_CORE_SKIP_ZIPPING                     0)
set(OPENSYDE_CORE_SKIP_IMPORTS                     1)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             1)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            1)
set(OPENSYDE_CORE_SKIP_PARAM_SET                   1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     1)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         1)
set(OPENSYDE_CORE_SKIP_SECURITY                    0)
set(OPENSYDE_CORE_SKIP_SECURITY_CRYPTO_AGENT_UTILS 1)
if(NOT WIN32)
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          1)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           1)
   # No CAN/IP dispatchers: the daemon uses raw sockets, not the IP dispatcher.
   set(OPENSYDE_CORE_SKIP_LINUX_DRIVERS            1)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          0)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           0)
endif()
# Only pull in our own copy of opensyde_core when nothing has provided it already.
if(NOT TARGET opensyde_core)
   add_subdirectory(${PROJECT_ROOT}/../libraries/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)
endif()

# --- Project source files ----------------------------------------------------------------------------------------------
set(OSY_CRYPTO_AGENT_SOURCES
   ${PROJECT_ROOT}/src/main.cpp
   ${PROJECT_ROOT}/src/C_CaApplication.cpp
   ${PROJECT_ROOT}/src/C_CaServer.cpp
   ${PROJECT_ROOT}/src/C_CaServerLogic.cpp
)
if(WIN32)
   list(APPEND OSY_CRYPTO_AGENT_SOURCES
      ${PROJECT_ROOT}/src/windows/C_CaServerPlatform.cpp
      ${PROJECT_ROOT}/src/resources.rc
   )
else()
   list(APPEND OSY_CRYPTO_AGENT_SOURCES
      ${PROJECT_ROOT}/src/linux/C_CaServerPlatform.cpp
   )
endif()

# --- BUILD EXECUTABLE -------------------------------------------------------------------------------------------------

# define binary name and sources
add_executable(osy_crypto_agent ${OSY_CRYPTO_AGENT_SOURCES})

# add required libraries
target_link_libraries(osy_crypto_agent
   opensyde_core
)
target_link_libraries(osy_crypto_agent OpenSSL::Crypto)
if(WIN32)
   # WinSock for the TCP listener; version for the embedded .rc resource info.
   target_link_libraries(osy_crypto_agent ws2_32 version Iphlpapi)
else()
   target_link_libraries(osy_crypto_agent pthread)
   if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      target_link_libraries(osy_crypto_agent rt)
   endif()
endif()

# define precompiled headers for core and application
target_precompile_headers(opensyde_core PRIVATE ${PROJECT_ROOT}/src/precomp_headers.hpp)
target_precompile_headers(osy_crypto_agent PRIVATE ${PROJECT_ROOT}/src/precomp_headers.hpp)

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   add_pc_lint(osy_crypto_agent ${OSY_CRYPTO_AGENT_SOURCES})
endif(COMMAND add_pc_lint)

# --- INSTALL RESULTS --------------------------------------------------------------------------------------------------
set(REPO_ROOT ${PROJECT_ROOT}/..)
install(TARGETS osy_crypto_agent DESTINATION ${REPO_ROOT}/result/${CMAKE_BUILD_TYPE}/crypto_agent)

osy_tool_werror(osy_crypto_agent)
