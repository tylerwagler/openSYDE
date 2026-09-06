set(Qt6Svg_VERSION 6.10.2)
set(SVG_LIBRARIES Qt6::Svg)
set(SVG_INCLUDE_DIRS "")

add_library(Qt6::Svg SHARED IMPORTED)
set_target_properties(Qt6::Svg PROPERTIES
    IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libQt6Svg.so.6.10.2
    INTERFACE_COMPILE_FEATURES cxx_std_17
)

add_library(Qt6::SvgWidgets SHARED IMPORTED)
set_target_properties(Qt6::SvgWidgets PROPERTIES
    IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libQt6Svg.so.6.10.2
    INTERFACE_COMPILE_FEATURES cxx_std_17
)
