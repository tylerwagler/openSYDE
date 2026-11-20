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
# AGL (the legacy Apple Graphics Library) no longer exists in the macOS 26 SDK, and
# Qt <= 6.8 still links it on Apple: its FindWrapOpenGL.cmake adds AGL to the
# WrapOpenGL::WrapOpenGL target, which Qt6::Gui pulls in. Every GUI link on a macOS 26
# runner therefore dies with "ld: framework 'AGL' not found" before one object of ours
# is considered. Qt does not use AGL; on older SDKs the flag was inert, so removing it
# changes nothing there.
#
# The item can appear in several shapes -- "-framework AGL" as one string, "-framework"
# followed by "AGL" as two list items, or an absolute /System/Library/Frameworks path --
# so match on AGL anywhere and drop a dangling -framework left behind.
#
# Call after find_package(Qt6 ...) in each GUI tool:
#
#   osy_strip_qt_removed_macos_frameworks()
function(osy_strip_qt_removed_macos_frameworks)
   if(NOT APPLE)
      return()
   endif()
   foreach(OSY_QT_TARGET
           Qt6::Gui Qt6::Widgets Qt6::PrintSupport Qt6::Svg Qt6::SvgWidgets
           Qt6::OpenGL Qt6::OpenGLWidgets WrapOpenGL::WrapOpenGL)
      if(NOT TARGET ${OSY_QT_TARGET})
         continue()
      endif()
      foreach(_osy_prop INTERFACE_LINK_LIBRARIES INTERFACE_LINK_OPTIONS)
         get_target_property(_osy_items ${OSY_QT_TARGET} ${_osy_prop})
         if(NOT _osy_items)
            continue()
         endif()
         set(_osy_kept "")
         set(_osy_pending_framework FALSE)
         foreach(_osy_item IN LISTS _osy_items)
            if(_osy_item MATCHES "AGL")
               # Drop the item, and a "-framework" that was only there to introduce it.
               if(_osy_pending_framework)
                  list(REMOVE_AT _osy_kept -1)
                  set(_osy_pending_framework FALSE)
               endif()
            elseif(_osy_item STREQUAL "-framework")
               set(_osy_pending_framework TRUE)
               list(APPEND _osy_kept "${_osy_item}")
            else()
               set(_osy_pending_framework FALSE)
               list(APPEND _osy_kept "${_osy_item}")
            endif()
         endforeach()
         if(NOT _osy_kept STREQUAL _osy_items)
            set_target_properties(${OSY_QT_TARGET} PROPERTIES ${_osy_prop} "${_osy_kept}")
         endif()
      endforeach()
   endforeach()
endfunction()
