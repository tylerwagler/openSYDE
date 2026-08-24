# minimum 3.24 for FetchContent, FILE_SET, and modern CMake policies
cmake_minimum_required(VERSION 3.24)

project("syde_x_gen")

#include script to generate PC-lint configuration files
include(lint_config.cmake)

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)

# --- PROJECT INCLUDE PATHS --------------------------------------------------------------------------------------------
set(SYDE_X_GEN_COMMON_INCLUDE_DIRECTORIES
   ${PROJECT_ROOT}/src
   ${PROJECT_ROOT}/src/x_gen_exporter
   )

if (WIN32)
   set(SYDE_X_GEN_COMMON_INCLUDE_DIRECTORIES
      ${SYDE_X_GEN_COMMON_INCLUDE_DIRECTORIES}
      )
endif()


include_directories(${SYDE_X_GEN_COMMON_INCLUDE_DIRECTORIES})

# --- PROJECT SOURCE FILE PATHS ----------------------------------------------------------------------------------------
set(SYDE_X_GEN_COMMON_SOURCES
   ${PROJECT_ROOT}/src/C_XconfigGenExport.cpp
   ${PROJECT_ROOT}/src/x_gen_exporter/C_XconfigGenExportBase.cpp
   ${PROJECT_ROOT}/src/x_gen_exporter/C_XconfigGenExportBase.hpp
)

set(SYDE_X_GEN_WINDOWS_SOURCES
   ${PROJECT_ROOT}/src/resources.rc
   )

# --- COMPILER FLAGS AND LIBRARIES -------------------------------------------------------------------------------------

# OPENSYDE CORE
# only include relevant modules (include directories and source files)
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
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         0)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     0)
set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS             1)
set(OPENSYDE_CORE_SKIP_LINUX_DRIVERS               1)
set(OPENSYDE_CORE_SKIP_SECURITY                    1)

if(WIN32)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET   0)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET   1)
endif()

add_subdirectory(${PROJECT_ROOT}/../libraries/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)

# SET LIBRARIES
if(WIN32)
   set(SYDE_X_GEN_LINK_LIBRARIES
      version
      opensyde_core
   )
else()
   set(SYDE_X_GEN_LINK_LIBRARIES
      opensyde_core
   )
endif()

# COMPILER SWITCHES
# use same compiler switches that gcc uses when compiling openSYDE core from osc_moduletest.pro file
add_compile_options(-Wno-deprecated-declarations -Wall -Wextra)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# do not warn about unused parameters in miniz.c; this library is not owned by us
set_source_files_properties(../../libraries/opensyde_core/miniz/miniz.c PROPERTIES COMPILE_FLAGS -Wno-unused-parameter)

# --- BUILD EXECUTABLE -------------------------------------------------------------------------------------------------
if(WIN32)
   add_executable(syde_x_gen ${SYDE_X_GEN_COMMON_SOURCES} ${SYDE_X_GEN_WINDOWS_SOURCES})
else()
   add_executable(syde_x_gen ${SYDE_X_GEN_COMMON_SOURCES})
endif()
target_link_libraries(syde_x_gen ${SYDE_X_GEN_LINK_LIBRARIES})

# common include directories
target_include_directories(syde_x_gen PRIVATE ${SYDE_X_GEN_COMMON_INCLUDE_DIRECTORIES})

# precompiled headers for core and application
target_precompile_headers(syde_x_gen PRIVATE ${PROJECT_ROOT}/src/precomp_headers.hpp)

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   if(WIN32)
      add_pc_lint(syde_x_gen ${SYDE_X_GEN_COMMON_SOURCES} ${SYDE_X_GEN_WINDOWS_SOURCES})
   endif(WIN32)
endif(COMMAND add_pc_lint)

# --- INSTALL RESULTS --------------------------------------------------------------------------------------------------
set(REPO_ROOT ${PROJECT_ROOT}/..)
install(TARGETS syde_x_gen DESTINATION ${REPO_ROOT}/result/${CMAKE_BUILD_TYPE}/syde_x_gen)
