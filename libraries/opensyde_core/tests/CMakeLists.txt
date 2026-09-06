# CMakeLists.txt for opensyde_core unit tests
#
# Usage:
#   cmake -S libraries/opensyde_core/tests -B build/tests -G Ninja
#   ninja -C build/tests
#   ctest --test-dir build/tests
#
# This project pulls in the full opensyde_core library via add_subdirectory()
# and builds test executables against it.  A stub CanLibraries::can target is
# provided so that the can-libraries submodule is never required when only
# core infrastructure needs to be compiled.
#
# Dependencies:
#   - CMake >= 3.16
#   - C++17 compiler
#   - Google Test (fetched automatically via FetchContent)

cmake_minimum_required(VERSION 3.16)
project(opensyde_core_tests LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Provide a stub CanLibraries::can target so opensyde_core's CMakeLists.txt
# does not attempt to add_subdirectory() the can-libraries submodule.
# The opensyde_core public link to CanLibraries::can only happens when
# PROTOCOL_DRIVERS_COMMON is enabled; with it skipped the stub is harmless.
# ---------------------------------------------------------------------------
if(NOT TARGET CanLibraries::can)
   add_library(CanLibraries_stub INTERFACE)
   add_library(CanLibraries::can ALIAS CanLibraries_stub)
endif()

# ---------------------------------------------------------------------------
# Skip modules that require external libraries not needed for basic testing.
# Comment out a SKIP line to enable that module (and satisfy its deps).
# ---------------------------------------------------------------------------
set(OPENSYDE_CORE_SKIP_ZIPPING                     0)
set(OPENSYDE_CORE_SKIP_IMPORTS                     0)
set(OPENSYDE_CORE_SKIP_CODE_GENERATION             1)  # needs protocol_drivers headers
set(OPENSYDE_CORE_SKIP_PROJECT_HANDLING            0)
set(OPENSYDE_CORE_SKIP_PARAM_SET                   0)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_BASIC      1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_SYSTEM     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_COMMON     1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_DRIVERS_MONITOR    1)
set(OPENSYDE_CORE_SKIP_PROTOCOL_LOGGING            1)
set(OPENSYDE_CORE_SKIP_SECURITY                    1)
set(OPENSYDE_CORE_SKIP_X_CONFIG_GENERATION         1)
set(OPENSYDE_CORE_SKIP_X_CERTIFICATES_GENERATION   1)
set(OPENSYDE_CORE_SKIP_SYSTEM_PACKAGE_HANDLING     1)
set(OPENSYDE_CORE_SKIP_WINDOWS_DRIVERS             1)
set(OPENSYDE_CORE_SKIP_WINDOWS_TARGET              1)
set(OPENSYDE_CORE_SKIP_LINUX_DRIVERS               1)  # needs ip_dispatcher headers from protocol_drivers_common

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_BINARY_DIR}/opensyde_core)

# ---------------------------------------------------------------------------
# Google Test
# ---------------------------------------------------------------------------
option(OPENSYDE_CORE_ENABLE_TESTS "Build tests for opensyde_core" ON)

if(OPENSYDE_CORE_ENABLE_TESTS)
   include(FetchContent)
   FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG        v1.14.0
   )
   # Prevent gtest from installing itself into our build
   set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
   set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
   FetchContent_MakeAvailable(googletest)

   enable_testing()

   # ------------------------------------------------------------------------
   # Test executables
   # ------------------------------------------------------------------------

   # Starter test — exercises the common infrastructure (logging, XML parser,
   # INI file handler, string utilities, etc.)
   add_executable(test_opensyde_core_common
      test_common.cpp
   )
   target_link_libraries(test_opensyde_core_common
      PRIVATE
         opensyde_core
         GTest::gtest_main
   )
   target_include_directories(test_opensyde_core_common
      PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..
   )
   gtest_discover_tests(test_opensyde_core_common)
endif()
