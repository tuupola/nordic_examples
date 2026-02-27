cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(007_uart_mavlink)

zephyr_library_sources(src/main.c)

target_include_directories(app PRIVATE external/mavlink)
