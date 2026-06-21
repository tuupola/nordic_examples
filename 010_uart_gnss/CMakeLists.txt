cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(010_uart_gnss)

configure_file(
    external/lwgps/lwgps/src/include/lwgps/lwgps_opts_template.h
    ${CMAKE_CURRENT_BINARY_DIR}/lwgps_opts.h
    COPYONLY
)
zephyr_include_directories(external/lwgps/lwgps/src/include ${CMAKE_CURRENT_BINARY_DIR})
zephyr_library_sources(src/main.c external/lwgps/lwgps/src/lwgps/lwgps.c)
