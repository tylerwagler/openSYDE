# minimum 3.16 for precompiled headers
cmake_minimum_required(VERSION 3.16)

project("osy_cmd_line_flash_tool")

#include script to generate PC-lint configuration files
include(lint_config.cmake)

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)

# --- PROJECT INCLUDE PATHS --------------------------------------------------------------------------------------------
# Set include directories before dragging in core; the core needs the "precomp_headers.hpp" and files included by it
include_directories(
   ${PROJECT_ROOT}/src
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   find_package(OpenSSL REQUIRED)
   include_directories(${OPENSSL_INCLUDE_DIR})
else()
   include_directories(${PROJECT_ROOT}/libs/openssl/openssl/include)
endif()

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# OPENSYDE CORE
# only include relevant modules (includes directories and source files)
set(OPENSYDE_CORE_SKIP_ZIPPING                     0)
set(OPENSYDE_CORE_SKIP_IMPORTS                     1)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             1)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            1)
set(OPENSYDE_CORE_SKIP_PARAM_SET                   1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     1)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         1)
set(OPENSYDE_CORE_SKIP_SECURITY                    1)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          1)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           1)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          0)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           0)
endif()
add_subdirectory(${PROJECT_ROOT}/../libraries/opensyde_core ${CMAKE_BINARY_DIR}/osy_core)

# warn a lot
# Unfortunately forcing C90 by using "-Wc90-c99-compat" or "-std=c89" would (correctly) also prohibit
#  C++ style comments and there's no switch to explicitly suppress this warning
set(CMAKE_C_FLAGS "-Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized \
   -Wdeclaration-after-statement -Wsign-compare -Wstrict-prototypes -Wmissing-prototypes \
   -Wdouble-promotion -Wfloat-equal -Wmissing-include-dirs -Wshadow -Wundef \
   -Wunused-macros -Wcast-align -Wlogical-op -Wno-multichar -Wredundant-decls -Wnested-externs \
   -Wno-deprecated-declarations")
# For C++ do not add "-Wdouble-promotion" as it causes a warning in tinyxml.cpp which we do not want to modify
# C++17 to match the other openSYDE apps and to satisfy libcan's INTERFACE cxx_std_14 minimum.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized -Wsign-compare  \
   -Wmissing-include-dirs -Wshadow -Wlogical-op -Wno-multichar -Wredundant-decls -Wno-deprecated-declarations")

# link standard libraries statically (fully static only on Windows)
if(WIN32)
   set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
else()
   set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

# --- Project source file ----------------------------------------------------------------------------------------------
set(OSY_CMD_LINE_FLASH_TOOL_SOURCES
   ${PROJECT_ROOT}/src/main.cpp
   ${PROJECT_ROOT}/src/C_BasicFlashTool.cpp
   ${PROJECT_ROOT}/src/C_BasicUpdateSequence.cpp

   ${PROJECT_ROOT}/src/C_BasicFlashTool.hpp
   ${PROJECT_ROOT}/src/C_BasicUpdateSequence.hpp

   ../src/resources.rc
)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
   link_directories(
       ../libs/openssl/openssl
   )
endif()

# --- BUILD EXECUTABLE -------------------------------------------------------------------------------------------------

# define binary name and sources
add_executable(osy_cmd_line_flash_tool ${OSY_CMD_LINE_FLASH_TOOL_SOURCES})

# add required libraries
target_link_libraries(osy_cmd_line_flash_tool
   opensyde_core
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   target_link_libraries(osy_cmd_line_flash_tool OpenSSL::SSL OpenSSL::Crypto)
else()
   target_link_libraries(osy_cmd_line_flash_tool version)
endif()

# define precompiled headers for core and application
target_precompile_headers(opensyde_core PRIVATE ${PROJECT_ROOT}/src/precomp_headers.hpp)
target_precompile_headers(osy_cmd_line_flash_tool PRIVATE ${PROJECT_ROOT}/src/precomp_headers.hpp)

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   add_pc_lint(osy_cmd_line_flash_tool ${OSY_CMD_LINE_FLASH_TOOL_SOURCES})
endif(COMMAND add_pc_lint)

# --- INSTALL RESULTS --------------------------------------------------------------------------------------------------
set(REPO_ROOT ${PROJECT_ROOT}/..)
install(TARGETS osy_cmd_line_flash_tool DESTINATION ${REPO_ROOT}/result/${CMAKE_BUILD_TYPE}/cmd_line_flash_tool)
