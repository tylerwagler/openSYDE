# minimum 3.16 for precompiled headers
cmake_minimum_required(VERSION 3.16)

project("osy_tsp_convert")

#include script to generate PC-lint configuration files
include(../../pjt/lint_config.cmake)

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)

# --- PROJECT INCLUDE PATHS --------------------------------------------------------------------------------------------
# Set include directories before dragging in core; the core needs the "precomp_headers.hpp" and files included by it
include_directories(
   ${PROJECT_ROOT}/src
   ${PROJECT_ROOT}/src/v2_filer
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   find_package(OpenSSL REQUIRED)
   include_directories(${OPENSSL_INCLUDE_DIR})
else()
    include_directories(${PROJECT_ROOT}/../libraries/openssl/openssl/include)
endif()

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# OPENSYDE CORE
# Pull in zipping (needed for output bundle) and project handling (NodeFiler + V2 TSP filer + node-definition save).
# Skip everything network/protocol/code-generation related.
set(OPENSYDE_CORE_SKIP_ZIPPING                     0)
set(OPENSYDE_CORE_SKIP_IMPORTS                     1)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             1)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            0)
set(OPENSYDE_CORE_SKIP_PARAM_SET                   1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     1)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         1)
set(OPENSYDE_CORE_SKIP_SECURITY                    1)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          1)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           1)
   # No CAN/IP dispatchers: this is a pure file-format converter.
   set(OPENSYDE_CORE_SKIP_LINUX_DRIVERS            1)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          0)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           0)
endif()
add_subdirectory(${PROJECT_ROOT}/../libraries/opensyde_core ${CMAKE_BINARY_DIR}/osy_core)

# warn a lot
set(CMAKE_C_FLAGS "-Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized \
   -Wdeclaration-after-statement -Wsign-compare -Wstrict-prototypes -Wmissing-prototypes \
   -Wdouble-promotion -Wfloat-equal -Wmissing-include-dirs -Wshadow -Wundef \
   -Wunused-macros -Wcast-align -Wlogical-op -Wno-multichar -Wredundant-decls -Wnested-externs \
   -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "-std=c++98 -Wall -Wextra -Wunused -Wuninitialized -Wmaybe-uninitialized -Wsign-compare  \
   -Wmissing-include-dirs -Wshadow -Wlogical-op -Wno-multichar -Wredundant-decls -Wno-deprecated-declarations")

# link standard libraries statically (fully static only on Windows)
if(WIN32)
   set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
else()
   set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

# --- Project source file ----------------------------------------------------------------------------------------------
set(OSY_TSP_CONVERT_SOURCES
   ${PROJECT_ROOT}/src/main.cpp
   # V2 TSP filer lives here (moved out of opensyde_core when V3 became canonical) — only this
   # converter still reads V2 files, so keeping the legacy reader scoped to one binary.
   ${PROJECT_ROOT}/src/v2_filer/C_OscTargetSupportPackageV2.cpp
   ${PROJECT_ROOT}/src/v2_filer/C_OscTargetSupportPackageV2Filer.cpp
   ${PROJECT_ROOT}/src/v2_filer/C_OscTargetSupportPackageV2.hpp
   ${PROJECT_ROOT}/src/v2_filer/C_OscTargetSupportPackageV2Filer.hpp
)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
   link_directories(
        ../libraries/openssl/openssl
   )
endif()

# --- BUILD EXECUTABLE -------------------------------------------------------------------------------------------------

# define binary name and sources
add_executable(osy_tsp_convert ${OSY_TSP_CONVERT_SOURCES})

# add required libraries
target_link_libraries(osy_tsp_convert
   opensyde_core
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   target_link_libraries(osy_tsp_convert OpenSSL::SSL OpenSSL::Crypto)
else()
   target_link_libraries(osy_tsp_convert version)
endif()

# define precompiled headers for core and application
target_precompile_headers(opensyde_core PRIVATE ${PROJECT_ROOT}/src/precomp_headers.hpp)
target_precompile_headers(osy_tsp_convert PRIVATE ${PROJECT_ROOT}/src/precomp_headers.hpp)

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   add_pc_lint(osy_tsp_convert ${OSY_TSP_CONVERT_SOURCES})
endif(COMMAND add_pc_lint)

# --- INSTALL RESULTS --------------------------------------------------------------------------------------------------
set(REPO_ROOT ${PROJECT_ROOT}/..)
install(TARGETS osy_tsp_convert DESTINATION ${REPO_ROOT}/result/${CMAKE_BUILD_TYPE}/tsp_convert)
