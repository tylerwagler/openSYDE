# Wrapper to build vector_blf as a STATIC library.
# The upstream CMakeLists.txt hardcodes SHARED, so we build it ourselves
# in a separate add_subdirectory scope.

cmake_minimum_required(VERSION 3.16)

set(VECTOR_BLF_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../libraries/vector_blf)
set(VECTOR_BLF_SRC_DIR ${VECTOR_BLF_ROOT}/src/Vector/BLF)

find_package(ZLIB REQUIRED)
include(GenerateExportHeader)

# Collect sources by globbing the upstream source list
file(GLOB VECTOR_BLF_SOURCES ${VECTOR_BLF_SRC_DIR}/*.cpp)
file(GLOB VECTOR_BLF_HEADERS ${VECTOR_BLF_SRC_DIR}/*.h)

add_library(Vector_BLF STATIC ${VECTOR_BLF_SOURCES} ${VECTOR_BLF_HEADERS})

# Generated headers go under Vector/BLF/ so that #include <Vector/BLF/config.h> works
set(VECTOR_BLF_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/Vector/BLF)
# Upstream config.h.in is empty; we add <cstdint> which the headers need but don't include
file(WRITE ${VECTOR_BLF_GEN_DIR}/config.h
   "#pragma once\n#include <cstdint>\n")
generate_export_header(Vector_BLF
   EXPORT_FILE_NAME ${VECTOR_BLF_GEN_DIR}/vector_blf_export.h)

target_include_directories(Vector_BLF PUBLIC
   ${VECTOR_BLF_ROOT}/src
   ${VECTOR_BLF_SRC_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}/generated
   ${VECTOR_BLF_GEN_DIR})

target_link_libraries(Vector_BLF PRIVATE ZLIB::ZLIB)
target_compile_features(Vector_BLF PRIVATE cxx_std_11)

# Mark as static for the export header macros
target_compile_definitions(Vector_BLF PUBLIC VECTOR_BLF_STATIC_DEFINE)
