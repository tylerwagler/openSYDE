#-----------------------------------------------------------------------------------------------------------------------
# Stamp osy_build_info.hpp from the git checkout. Runs in script mode (cmake -P), once at
# configure time so the header exists, and again as a custom target before every build of
# opensyde_core so it follows the checkout rather than the last configure.
#
# Inputs:  SOURCE_DIR      any directory inside the checkout
#          TEMPLATE        osy_build_info.hpp.in
#          OUTPUT          where to write the header
#          GIT_EXECUTABLE  optional; without it, or outside a checkout, both values are "unknown"
#
# The two values are deliberately ones that change only with the commit, so an incremental
# build does not re-link every tool: `git describe --tags --always --dirty` identifies the
# source, and the committer date says when it was made. A wall-clock build date would have
# rewritten the header on every build. configure_file() only touches the output when the
# content differs, which is what keeps the stamp cheap.
#
# Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
# Copyright 2026 Elytron Defense. All rights reserved.
#-----------------------------------------------------------------------------------------------------------------------
set(OSY_BUILD_GIT_DESCRIBE "unknown")
set(OSY_BUILD_SOURCE_DATE "unknown")

if(GIT_EXECUTABLE)
   execute_process(
      COMMAND "${GIT_EXECUTABLE}" -C "${SOURCE_DIR}" describe --tags --always --dirty
      RESULT_VARIABLE describe_result
      OUTPUT_VARIABLE describe_output
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
   if(describe_result EQUAL 0 AND NOT describe_output STREQUAL "")
      set(OSY_BUILD_GIT_DESCRIBE "${describe_output}")
   endif()

   execute_process(
      COMMAND "${GIT_EXECUTABLE}" -C "${SOURCE_DIR}" log -1 --format=%cI
      RESULT_VARIABLE date_result
      OUTPUT_VARIABLE date_output
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET)
   if(date_result EQUAL 0 AND NOT date_output STREQUAL "")
      set(OSY_BUILD_SOURCE_DATE "${date_output}")
   endif()
endif()

configure_file("${TEMPLATE}" "${OUTPUT}" @ONLY)
