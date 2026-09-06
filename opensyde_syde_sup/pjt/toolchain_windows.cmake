# This toolchain file has been consolidated.
# The shared definition is at: ../../pjt/toolchains/windows_x64.cmake
# Kept for backward compatibility — new builds should reference the shared file directly.
include(${CMAKE_CURRENT_LIST_DIR}/../../pjt/toolchains/windows_x64.cmake)

# Tool-specific: install path
set(CMAKE_INSTALL_BINDIR ${PROJECT_SOURCE_DIR}/../result/${INSTALL_SUBDIR} CACHE PATH "Installing path")
