cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(010_uart_gnss)

zephyr_include_directories(external/librid/include)

zephyr_library_sources(
    src/main.c
    external/librid/src/json.c
    external/librid/src/location.c
    external/librid/src/message.c
)
