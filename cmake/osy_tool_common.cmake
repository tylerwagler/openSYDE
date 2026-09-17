# Shared scaffolding for every openSYDE tool. Included by each <tool>/pjt/CMakeLists.txt
# right after project(). Holds only what is genuinely identical across the eight tools;
# source lists, include directories and OPENSYDE_CORE_SKIP_* selection stay per tool,
# because those are what make each tool a different tool.
#
# Before this file existed each tool carried its own copy of the blocks below, and they
# had drifted: two tools had an older lint_config.cmake missing a fix the other six had,
# three used a different build-directory name for opensyde_core, two did not pin the C++
# standard at all, and the -Werror block existed in three slightly different spellings.

# C++23 tree-wide. The toolchain files set this too, so the unified build.sh build never
# depends on it -- it is here so a standalone configure of one tool without a toolchain
# file gets the same standard as everything else.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Turn warnings into errors on the targets we own.
#
# opensyde_core has built with -Wall -Wextra -Werror and zero diagnostics since 2026-09-06,
# and every line the tools own is warning-free, so this only pins that state. It is target
# scoped rather than a global flag because the vendored third-party code we build alongside
# (miniz, tinyxml2, the CAN libraries, QCustomPlot) is not held to our warning bar and we
# neither own nor can fix it.
#
#   osy_tool_werror(openSYDE)            # one target
#   osy_tool_werror(sydesuplib SYDEsup)  # a library and the executable that links it
function(osy_tool_werror)
   if(NOT MSVC)
      foreach(OSY_WERROR_TARGET IN LISTS ARGV)
         if(TARGET ${OSY_WERROR_TARGET})
            target_compile_options(${OSY_WERROR_TARGET} PRIVATE -Werror)
         endif()
      endforeach()
   endif()
endfunction()
