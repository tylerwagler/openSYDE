#-----------------------------------------------------------------------------------------------------------------------
# Vendored QCustomPlot 2.1.1 (GPL) built as a static library for every platform.
# See README.md for provenance and why it is vendored rather than swapped for Qt Charts.
#-----------------------------------------------------------------------------------------------------------------------

# QCustomPlot uses bare Qt keywords (signals/slots/foreach). The consuming project
# builds with QT_NO_KEYWORDS (set via add_definitions in the tool CMake), and that
# define is inherited here. It has to be dropped for BOTH the compiler and moc:
# AUTOMOC reads the directory's COMPILE_DEFINITIONS, so a per-target -UQT_NO_KEYWORDS
# is not enough -- moc would still misparse the class and emit references to enum
# types (ScaleType, SelectableParts, ...) that then fail to compile. remove_definitions
# strips it from this directory, which is why QCustomPlot lives in its own subdir.
remove_definitions(-DQT_NO_KEYWORDS)

add_library(qcustomplot STATIC ${CMAKE_CURRENT_SOURCE_DIR}/qcustomplot.cpp)

target_include_directories(qcustomplot PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qcustomplot PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PrintSupport)

# QCustomPlot classes use Q_OBJECT, so moc is required; it has no .ui or .qrc.
set_target_properties(qcustomplot PROPERTIES AUTOMOC ON AUTOUIC OFF AUTORCC OFF)

# Third-party GPL code, held to its own standard rather than ours -- like miniz.
target_compile_options(qcustomplot PRIVATE -w)
