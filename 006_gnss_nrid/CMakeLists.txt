cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(006_gnss_nrid)

zephyr_include_directories(external/librid/include)

zephyr_library_sources(
    src/main.c
    external/librid/src/basic_id.c
    external/librid/src/location.c
    external/librid/src/message.c
    external/librid/src/system.c
    external/librid/src/message_pack.c
    external/librid/src/header.c
    external/librid/src/auth.c
    external/librid/src/auth_page.c
    external/librid/src/operator_id.c
    external/librid/src/self_id.c
    external/librid/src/transport.c
    external/librid/src/version.c
)
