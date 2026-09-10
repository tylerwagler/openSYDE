# CMake toolchain file for a native macOS build.
# Selected automatically by build.sh when `uname -s` reports Darwin.
#
# Deliberately does NOT set CMAKE_SYSTEM_NAME. Setting it -- as the Linux
# toolchain does -- tells CMake it is cross-compiling, which on a native build
# only disables host detection and breaks find_package. Let CMake see the host.

# Apple ships g++ as an alias for clang; say what we mean.
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "")

# --- Homebrew ---------------------------------------------------------------------------------------------------------
# Several dependencies come from Homebrew and are keg-only, so they are neither
# on PATH nor in CMake's default search paths. Intel Macs use /usr/local, Apple
# Silicon uses /opt/homebrew; ask brew rather than guessing, with a fallback.
execute_process(
   COMMAND brew --prefix
   OUTPUT_VARIABLE OSY_BREW_PREFIX
   OUTPUT_STRIP_TRAILING_WHITESPACE
   ERROR_QUIET
   RESULT_VARIABLE OSY_BREW_RC
)
if(NOT OSY_BREW_RC EQUAL 0 OR OSY_BREW_PREFIX STREQUAL "")
   if(EXISTS /opt/homebrew)
      set(OSY_BREW_PREFIX /opt/homebrew)
   else()
      set(OSY_BREW_PREFIX /usr/local)
   endif()
endif()

# bison: Apple ships 2.3 and Vector_DBC requires >= 3.3.
#   brew install bison
if(EXISTS "${OSY_BREW_PREFIX}/opt/bison/bin/bison")
   set(BISON_EXECUTABLE "${OSY_BREW_PREFIX}/opt/bison/bin/bison" CACHE FILEPATH "" FORCE)
endif()

# flex: Apple's /usr/bin/flex works, but FindFLEX looks for FlexLexer.h next to
# the executable's prefix, and Apple keeps it in CommandLineTools instead -- so
# FLEX_INCLUDE_DIR comes back NOTFOUND and Vector_DBC cannot generate its scanner.
# This is the macOS twin of the libfl-dev gotcha on Ubuntu. brew's flex ships the
# binary and header together.
#   brew install flex
if(EXISTS "${OSY_BREW_PREFIX}/opt/flex/bin/flex")
   set(FLEX_EXECUTABLE  "${OSY_BREW_PREFIX}/opt/flex/bin/flex"  CACHE FILEPATH "" FORCE)
   set(FLEX_INCLUDE_DIR "${OSY_BREW_PREFIX}/opt/flex/include"   CACHE PATH     "" FORCE)
endif()

# OpenSSL and Qt are keg-only too; put their CMake configs where find_package looks.
#   brew install openssl@3 qt
foreach(_pkg openssl@3 qt)
   if(EXISTS "${OSY_BREW_PREFIX}/opt/${_pkg}")
      list(APPEND CMAKE_PREFIX_PATH "${OSY_BREW_PREFIX}/opt/${_pkg}")
   endif()
endforeach()
