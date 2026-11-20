# Shared scaffolding for every openSYDE tool. Included by each <tool>/pjt/CMakeLists.txt
# right after project(). Holds only what is genuinely identical across the nine tools;
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

# Drop framework flags Qt still carries for libraries macOS 26 removed.
#
# Qt 6.8's macOS packages list "-framework AGL" in Qt6::Gui's link interface. AGL (the
# legacy Apple Graphics Library) no longer exists in the macOS 26 SDK, so every GUI link
# on a macOS 26 runner dies with "ld: framework 'AGL' not found" before a single object
# of ours is considered. Qt does not use AGL; on older SDKs the flag was merely inert,
# so stripping it changes nothing where it was harmless.
#
# Call after find_package(Qt6 ...) in each GUI tool:
#
#   osy_strip_qt_removed_macos_frameworks()
function(osy_strip_qt_removed_macos_frameworks)
   if(NOT APPLE)
      return()
   endif()
   foreach(OSY_QT_TARGET Qt6::Gui Qt6::Widgets Qt6::PrintSupport Qt6::Svg Qt6::SvgWidgets)
      if(TARGET ${OSY_QT_TARGET})
         get_target_property(_osy_libs ${OSY_QT_TARGET} INTERFACE_LINK_LIBRARIES)
         if(_osy_libs)
            list(REMOVE_ITEM _osy_libs "-framework AGL" "AGL")
            set_target_properties(${OSY_QT_TARGET} PROPERTIES INTERFACE_LINK_LIBRARIES "${_osy_libs}")
         endif()
         get_target_property(_osy_opts ${OSY_QT_TARGET} INTERFACE_LINK_OPTIONS)
         if(_osy_opts)
            list(REMOVE_ITEM _osy_opts "-framework AGL" "AGL")
            set_target_properties(${OSY_QT_TARGET} PROPERTIES INTERFACE_LINK_OPTIONS "${_osy_opts}")
         endif()
      endif()
   endforeach()
endfunction()
