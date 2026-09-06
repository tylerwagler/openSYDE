# Find Qt6Svg module
find_library(QT6_SVG_LIBRARY NAMES Qt6Svg libQt6Svg HINTS /usr/lib/x86_64-linux-gnu/)
find_library(QT6_SVGWIDGETS_LIBRARY NAMES Qt6SvgWidgets libQt6SvgWidgets HINTS /usr/lib/x86_64-linux-gnu/)

if(QT6_SVG_LIBRARY)
    add_library(Qt6::Svg UNKNOWN IMPORTED)
    set_target_properties(Qt6::Svg PROPERTIES IMPORTED_LOCATION ${QT6_SVG_LIBRARY})
endif()

if(QT6_SVGWIDGETS_LIBRARY)
    add_library(Qt6::SvgWidgets UNKNOWN IMPORTED)
    set_target_properties(Qt6::SvgWidgets PROPERTIES IMPORTED_LOCATION ${QT6_SVGWIDGETS_LIBRARY})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qt6Svg DEFAULT_MSG QT6_SVG_LIBRARY)
