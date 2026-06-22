cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(010_uart_gnss)

zephyr_compile_definitions(LWGPS_IGNORE_USER_OPTS)
zephyr_include_directories(external/lwgps/lwgps/src/include)
zephyr_library_sources(src/main.c external/lwgps/lwgps/src/lwgps/lwgps.c)
