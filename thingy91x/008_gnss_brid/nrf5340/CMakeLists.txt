cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(008_gnss_brid_nrf5340)

zephyr_library_sources(src/main.c)
