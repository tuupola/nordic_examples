cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(009_segger_rtt)

zephyr_library_sources(src/main.c)
