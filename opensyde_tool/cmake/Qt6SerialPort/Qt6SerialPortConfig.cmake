set(Qt6SerialPort_VERSION 6.10.2)
add_library(Qt6::SerialPort SHARED IMPORTED)
set_target_properties(Qt6::SerialPort PROPERTIES
    IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libQt6SerialPort.so.6.10.2
)
