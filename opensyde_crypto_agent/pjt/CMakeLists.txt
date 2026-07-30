# minimum 3.16 for precompiled headers
cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME "osy_crypto_agent")
project(${PROJECT_NAME})

# Make installation directory variables known so we can use them with "install"
include(GNUInstallDirs)

# As we have a nested project structure by default paths for cmake object storage can become quite deep
# Tell cmake to abbreviate paths to stay within a safe margin. Otherwise this could cause issues with e.g. gcovr.
set(CMAKE_OBJECT_PATH_MAX 200)

#include script to generate PC-lint configuration files
include(lint_config.cmake)

# Static linking can be configured for Linux targets.
if (NOT WIN32)
if (${LNX_LINK_OPENSSL_STATIC})
   #link OpenSSL statically
   set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()
   find_package(OpenSSL 3.0.0 REQUIRED)
endif()

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)

# --- PROJECT INCLUDE PATHS --------------------------------------------------------------------------------------------
set(APPLICATION_INCLUDE_DIRECTORIES
   ${PROJECT_ROOT}/src
   )

if (WIN32)
   set(APPLICATION_INCLUDE_DIRECTORIES
      ${APPLICATION_INCLUDE_DIRECTORIES}
      ${PROJECT_ROOT}/src/windows
      ${PROJECT_ROOT}/libs/openssl/include
      )
else()
   set(APPLICATION_INCLUDE_DIRECTORIES
      ${APPLICATION_INCLUDE_DIRECTORIES}
      ${PROJECT_ROOT}/src/linux
      )
endif()

include_directories(${APPLICATION_INCLUDE_DIRECTORIES})

# --- PROJECT SOURCE FILE PATHS ----------------------------------------------------------------------------------------
set(APPLICATION_SOURCES
   ${PROJECT_ROOT}/src/main.cpp
   ${PROJECT_ROOT}/src/C_CaApplication.cpp
   ${PROJECT_ROOT}/src/C_CaApplication.hpp
   ${PROJECT_ROOT}/src/C_CaServer.cpp
   ${PROJECT_ROOT}/src/C_CaServer.hpp
   ${PROJECT_ROOT}/src/C_CaServerLogic.cpp
   ${PROJECT_ROOT}/src/C_CaServerLogic.hpp

   ${PROJECT_ROOT}/src/version_config.hpp
)

if (WIN32)
set(APPLICATION_SOURCES
   ${APPLICATION_SOURCES}
   ${PROJECT_ROOT}/src/windows/C_CaServerPlatform.cpp
   ${PROJECT_ROOT}/src/windows/C_CaServerPlatform.hpp
   ${PROJECT_ROOT}/src/resources.rc
   )
else()
set(APPLICATION_SOURCES
   ${APPLICATION_SOURCES}
   ${PROJECT_ROOT}/src/linux/C_CaServerPlatform.cpp
   ${PROJECT_ROOT}/src/linux/C_CaServerPlatform.hpp
   )
endif()

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# SET LIBRARIES
# OPENSYDE CORE
# only include relevant modules (include directories and source files)
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
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         1)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     1)
set(OPENSYDE_CORE_SKIP_SECURITY                    0)
set(OPENSYDE_CORE_SKIP_SECURITY_CRYPTO_AGENT_UTILS 1) # we do not need the utilities for using ourself
set(OPENSYDE_CORE_SKIP_LINUX_DRIVERS               1) # no communication drivers needed
set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS             1) # no communication drivers needed

if(WIN32)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET   0)
   set(OPENSYDE_CORE_SKIP_LINUX_TARGET     1)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET   1)
   set(OPENSYDE_CORE_SKIP_LINUX_TARGET     0)
endif()

add_subdirectory(${PROJECT_ROOT}/libs/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)

if (WIN32)
set(APPLICATION_LINK_LIBRARIES
   version
   opensyde_core
   ws2_32   #WinSock
   crypto   #OpenSSL
)
link_directories(
   ../libs/openssl
)
else()
set(APPLICATION_LINK_LIBRARIES
   rt
   opensyde_core
   OpenSSL::Crypto
)
endif()

# --- BUILD EXECUTABLE -------------------------------------------------------------------------------------------------
if (WIN32)
# for Windows we build as WIN32 application, so we do not get a console window by default
add_executable(${PROJECT_NAME} WIN32 ${APPLICATION_SOURCES})
else()
add_executable(${PROJECT_NAME} ${APPLICATION_SOURCES})
endif()

target_link_libraries(${PROJECT_NAME} ${APPLICATION_LINK_LIBRARIES})

target_include_directories(${PROJECT_NAME} PRIVATE ${APPLICATION_INCLUDE_DIRECTORIES})

# precompiled headers for core and application
target_precompile_headers(${PROJECT_NAME} PRIVATE ${PROJECT_ROOT}/src/precomp_headers.hpp)
target_precompile_headers(opensyde_core PRIVATE ${PROJECT_ROOT}/src/precomp_headers.hpp)

# do not complain about type mismatch in external library source
set_source_files_properties(../libs/opensyde_core/xml_parser/tinyxml2/tinyxml2.cpp TARGET_DIRECTORY opensyde_core PROPERTIES COMPILE_OPTIONS "-Wno-double-promotion")

# do not issue GCC deprecated build warning on OpenSSL-based implementation (see comments there for details)
set_source_files_properties(../libs/opensyde_core/security/C_OscSecurityPemSecUpdate.cpp TARGET_DIRECTORY opensyde_core PROPERTIES COMPILE_OPTIONS "-Wno-deprecated-declarations")
set_source_files_properties(../libs/opensyde_core/security/C_OscSecurityEcdsa.cpp TARGET_DIRECTORY opensyde_core PROPERTIES COMPILE_OPTIONS "-Wno-deprecated-declarations")
set_source_files_properties(../libs/opensyde_core/security/C_OscSecurityEcdhAes.cpp TARGET_DIRECTORY opensyde_core PROPERTIES COMPILE_OPTIONS "-Wno-deprecated-declarations")

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   add_pc_lint(${PROJECT_NAME} ${APPLICATION_SOURCES})
endif(COMMAND add_pc_lint)

# --- INSTALL RESULTS --------------------------------------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
