#-----------------------------------------------------------------------------------------------------------------------
# Whole-repository build: one opensyde_core, all eight tools.
#
# Historically each tool configured and compiled its own copy of opensyde_core with
# its own set of OPENSYDE_CORE_SKIP_* options. A clean eight-tool build produced
# 1,139 core object files across eight separate core archives and 5.8 GB in build/.
# This file builds core once: 220 objects, one archive, 4.5 GB, and 2m23s instead
# of roughly three minutes on a 48-core host.
#
# (Three tools name their core build directory osy_core rather than opensyde_core,
# which is why a directory-name count suggests five copies. There are eight.)
#
# WHY THIS IS SAFE
#
# The SKIP options are pure source selection: they add or omit entries in the
# source list and nothing else. There are no OPENSYDE_CORE_SKIP_* preprocessor
# defines and no #ifdef on them anywhere in the sources, so a core built with
# everything enabled compiles identically to one built with a subset -- it simply
# contains more objects.
#
# opensyde_core is a static library, so a tool links only the objects it actually
# references. The archive being larger does not enlarge the binaries or add
# dependencies. That holds specifically because opensyde_core does not link
# OpenSSL itself -- applications provide it (see the note in its CMakeLists) -- so
# enabling SKIP_SECURITY's sources here does not give OpenSSL to a tool that had
# none.
#
# That last point is the one to re-check if this ever changes: openSYDE_CAN_Monitor
# links no libcrypto today, and must not start.
#
# The per-tool builds still work exactly as before. Each tool guards its own
# add_subdirectory with if(NOT TARGET opensyde_core), so building a tool alone
# takes the old path with the old options. build.sh is unaffected.
#-----------------------------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.25)

project(openSYDE_All LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# One core has to satisfy its strictest consumer. opensyde_syde_sup builds
# sydesuplib as a SHARED library, which cannot link a non-PIC archive:
#
#   relocation R_X86_64_PC32 against symbol ... can not be used when making a
#   shared object; recompile with -fPIC
#
# The per-tool builds already differ on this -- sydesup compiles core with -fPIC
# and the openSYDE tool does not -- so a shared core takes the stricter setting.
# On x86-64 the cost for a static library linked into executables is negligible.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# zlib on Windows. Vector_BLF and CAN Monitor need zlib, but the LLVM-MinGW clang
# toolchain ships none (MinGW-GCC did; borrowing it would drag libstdc++ headers
# into a libc++ build). Build it from source -- source, not a vendored binary --
# and expose it under the names consumers expect: the classic FindZLIB result
# variables (Vector_BLF uses ${ZLIB_INCLUDE_DIRS}/${ZLIB_LIBRARIES}) and the
# ZLIB::ZLIB imported target (CAN Monitor). OVERRIDE_FIND_PACKAGE makes the
# find_package(ZLIB) calls in those subprojects resolve here. Windows-only:
# Linux and macOS resolve zlib from the system as before.
if(WIN32)
   include(FetchContent)
   FetchContent_Declare(ZLIB
      GIT_REPOSITORY https://github.com/madler/zlib.git
      GIT_TAG        v1.3.1
      OVERRIDE_FIND_PACKAGE
   )
   set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
   FetchContent_MakeAvailable(ZLIB)
   if(TARGET zlibstatic)
      if(NOT TARGET ZLIB::ZLIB)
         add_library(ZLIB::ZLIB ALIAS zlibstatic)
      endif()
      set(ZLIB_INCLUDE_DIRS "${zlib_SOURCE_DIR}" "${zlib_BINARY_DIR}" CACHE INTERNAL "source-built zlib includes")
      set(ZLIB_LIBRARIES    zlibstatic                                CACHE INTERNAL "source-built zlib library")
      set(ZLIB_FOUND        TRUE                                      CACHE INTERNAL "source-built zlib present")
   endif()
endif()

# Everything on: this core serves every tool, so it must be the union of what they need.
set(OPENSYDE_CORE_SKIP_ZIPPING                     0)
set(OPENSYDE_CORE_SKIP_IMPORTS                     0)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             0)
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            0)
set(OPENSYDE_CORE_SKIP_PARAM_SET                   0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            0)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     0)
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         0)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   0)
set(OPENSYDE_CORE_SKIP_SECURITY                    0)
set(OPENSYDE_CORE_SKIP_LINUX_DRIVERS               0)

# Windows-only subsystems stay out on Linux; they do not compile here.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          0)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           0)
else()
   set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS          1)
   set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET           1)
endif()

add_subdirectory(libraries/opensyde_core ${CMAKE_BINARY_DIR}/opensyde_core)

# Tools. Each calls project() itself, so PROJECT_SOURCE_DIR and the PROJECT_ROOT
# derived from it still resolve to that tool's own directory.
option(OPENSYDE_BUILD_GUI_TOOLS "Build the Qt GUI tools (openSYDE, CAN Monitor, SYDEflash)" ON)

if(OPENSYDE_BUILD_GUI_TOOLS)
   add_subdirectory(opensyde_tool/pjt/openSYDE        ${CMAKE_BINARY_DIR}/opensyde)
   add_subdirectory(opensyde_can_monitor/pjt          ${CMAKE_BINARY_DIR}/canmonitor)
   add_subdirectory(opensyde_syde_flash/pjt           ${CMAKE_BINARY_DIR}/sydeflash)
endif()

add_subdirectory(opensyde_syde_sup/pjt                ${CMAKE_BINARY_DIR}/sydesup)
add_subdirectory(opensyde_syde_x_gen/pjt              ${CMAKE_BINARY_DIR}/syde_x_gen)
add_subdirectory(opensyde_syde_coder_c/pjt            ${CMAKE_BINARY_DIR}/syde_coder_c)
add_subdirectory(opensyde_cmd_line_flash_tool/pjt     ${CMAKE_BINARY_DIR}/flash_tool)
add_subdirectory(opensyde_tsp_convert/pjt             ${CMAKE_BINARY_DIR}/tsp_convert)
