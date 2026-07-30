# minimum 3.16 for precompiled headers
cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME "SYDEsup")
project(${PROJECT_NAME})

# As we have a nested project structure by default paths for cmake object storage can become quite deep
# Tell cmake to abbreviate paths to stay within a safe margin. Otherwise this could cause issues with e.g. gcovr.
set(CMAKE_OBJECT_PATH_MAX 200)

# Make installation directory variables known so we can use them with "install"
include(GNUInstallDirs)
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
      ${PROJECT_ROOT}/libs/openssl/openssl/include
      )
endif()

include_directories(${APPLICATION_INCLUDE_DIRECTORIES})

# --- PROJECT SOURCE FILE PATHS ----------------------------------------------------------------------------------------
set(APPLICATION_SOURCES
   ${PROJECT_ROOT}/src/C_SydeSup.cpp
   ${PROJECT_ROOT}/src/C_SupSuSequences.cpp
   ${PROJECT_ROOT}/src/C_SupCreatePackage.cpp
   ${PROJECT_ROOT}/src/C_SupConfig.cpp
)

if (WIN32)
set(APPLICATION_SOURCES
   ${APPLICATION_SOURCES}
   ${PROJECT_ROOT}/src/WindowsMain.cpp
   ${PROJECT_ROOT}/src/C_SydeSupWindows.cpp
   ${PROJECT_ROOT}/src/resources.rc
   )
else()
set(APPLICATION_SOURCES
   ${APPLICATION_SOURCES}
   ${PROJECT_ROOT}/src/LinuxMain.cpp
   ${PROJECT_ROOT}/src/C_SydeSupLinux.cpp
   )
endif()

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# SET LIBRARIES
# OPENSYDE CORE
# only include relevant modules (include directories and source files)
set(OPENSYDE_CORE_SKIP_ZIPPING                     0)
set(OPENSYDE_CORE_SKIP_IMPORTS                     1)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             1)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            0)
set(OPENSYDE_CORE_SKIP_PARAM_SET                   0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            1)
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         1)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     0)
set(OPENSYDE_CORE_SKIP_SECURITY                    0)
set(OPENSYDE_CORE_SKIP_SECURITY_CRYPTO_AGENT_UTILS 0)

if(WIN32)
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS  0)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET   0)
   set(OPENSYDE_CORE_SKIP_LINUX_DRIVERS    1)
   set(OPENSYDE_CORE_SKIP_LINUX_TARGET     1)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS  1)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET   1)
   set(OPENSYDE_CORE_SKIP_LINUX_DRIVERS    0)
   set(OPENSYDE_CORE_SKIP_LINUX_TARGET     0)
endif()

add_subdirectory(${PROJECT_ROOT}/libs/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)

if (WIN32)
set(APPLICATION_LINK_LIBRARIES
   version
   opensyde_core
   ws2_32   #WinSock
   Iphlpapi #IP helper API
   crypto   #OpenSSL
)
link_directories(
   ../libs/openssl/openssl
)
else()
set(APPLICATION_LINK_LIBRARIES
   rt
   opensyde_core
   OpenSSL::Crypto
)
endif()

# --- BUILD EXECUTABLE -------------------------------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${APPLICATION_SOURCES})
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
