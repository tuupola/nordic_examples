cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(005_lte_mqtt)

zephyr_library_sources(src/main.c)
