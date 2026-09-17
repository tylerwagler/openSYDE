# 3.25 tree-wide: the root CMakeLists requires it, and a standalone configure of one tool should not accept less
cmake_minimum_required(VERSION 3.25)

project(osy_syde_coder_c LANGUAGES CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/osy_tool_common.cmake)

set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)

# include script to generate PC-lint configuration files
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/lint_config.cmake)

# Set include directories before dragging in core; the core needs the "precomp_header.h" and files included by it
include_directories(
   ${PROJECT_ROOT}/src

   ${PROJECT_ROOT}/src/osy_code_exporter
   ${PROJECT_ROOT}/../libraries/opensyde_core/protocol_drivers   #just one header
)

# Drag in openSYDE core base (files, include paths)
# we don't need all packages
set(OPENSYDE_CORE_SKIP_ZIPPING                     0)
set(OPENSYDE_CORE_SKIP_IMPORTS                     1)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             0)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            0)
set(OPENSYDE_CORE_SKIP_PARAM_SET                   0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     1)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         1)
set(OPENSYDE_CORE_SKIP_SECURITY                    1)
set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS             1)
set(OPENSYDE_CORE_SKIP_LINUX_DRIVERS               1)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           1)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           0)
endif()
# Only pull in our own copy of opensyde_core when nothing has provided it already.
# Building this tool on its own (the historical path, and what build.sh does) takes
# this branch and is unchanged. The root CMakeLists.txt adds one core configured with
# every subsystem enabled before adding the tools, so a whole-repo build shares a
# single core archive instead of compiling it once per tool.
if(NOT TARGET opensyde_core)
   add_subdirectory(${PROJECT_ROOT}/../libraries/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)
endif()



# --- Project source file ----------------------------------------------------------------------------------------------
set(OPENSYDE_SYDE_CODER_C_SOURCES
   ${PROJECT_ROOT}/src/C_CodeExport.cpp
   ${PROJECT_ROOT}/src/osy_code_exporter/C_OsyCodeExportBase.cpp

   ${PROJECT_ROOT}/src/resources.rc

   ${PROJECT_ROOT}/src/precomp_headers.hpp
   ${PROJECT_ROOT}/src/osy_code_exporter/C_OsyCodeExportBase.hpp
)

# define binary name and sources
add_executable(osy_syde_coder_c ${OPENSYDE_SYDE_CODER_C_SOURCES})

# add required libraries
target_link_libraries(osy_syde_coder_c
   opensyde_core
)
if(WIN32)
   target_link_libraries(osy_syde_coder_c version)
endif()

# define precompiled headers for core and application
target_precompile_headers(opensyde_core PRIVATE ../src/precomp_headers.hpp)
target_precompile_headers(osy_syde_coder_c PRIVATE ../src/precomp_headers.hpp)

# create PC-lint configuration files
if(COMMAND add_pc_lint)
   add_pc_lint(osy_syde_coder_c ${OPENSYDE_SYDE_CODER_C_SOURCES})
endif(COMMAND add_pc_lint)

# warn a lot — C++17 to match the rest of openSYDE
# -Wmaybe-uninitialized and -Wlogical-op are GCC-only. clang has no such options,
# and under -Werror an unknown warning flag is itself an error, so they must not
# reach a clang command line. -Wuninitialized (kept unconditionally) is the
# portable subset of the first.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   set(OSY_GCC_ONLY_WARNINGS "-Wmaybe-uninitialized -Wlogical-op")
else()
   set(OSY_GCC_ONLY_WARNINGS "")
endif()
set(CMAKE_C_FLAGS "${OSY_GCC_ONLY_WARNINGS} -Wall -Wextra -Wc99-c11-compat -Wunused -Wuninitialized \
   -Wdeclaration-after-statement -Wsign-compare -Wstrict-prototypes -Wmissing-prototypes \
   -Wdouble-promotion -Wfloat-equal -Wmissing-include-dirs -Wshadow -Wundef \
   -Wunused-macros -Wcast-align -Wno-multichar -Wredundant-decls -Wnested-externs")
# For C++ do not add "-Wdouble-promotion" as it causes a warning in tinyxml.cpp which we do not want to modify
set(CMAKE_CXX_FLAGS "${OSY_GCC_ONLY_WARNINGS} -Wall -Wextra -Wunused -Wuninitialized -Wsign-compare  \
   -Wmissing-include-dirs -Wshadow -Wno-multichar -Wredundant-decls")

# link standard libraries statically (fully static only on Windows)
#
# -static-libgcc / -static-libstdc++ are GCC options. clang on macOS has neither a
# libgcc nor a libstdc++ -- the runtime is compiler-rt inside libSystem and the C++
# library is libc++ -- and it rejects the flags outright. A dynamically linked
# libSystem is the only supported configuration on macOS anyway.
if(WIN32)
   set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()

# Install results
set(REPO_ROOT ${PROJECT_SOURCE_DIR}/../..)
install(TARGETS osy_syde_coder_c DESTINATION ${REPO_ROOT}/result/${CMAKE_BUILD_TYPE}/syde_coder_c)

osy_tool_werror(osy_syde_coder_c)
