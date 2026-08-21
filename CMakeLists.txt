cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(esp32_drivers)

include_directories(include)
include_directories(drivers/uart)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources} drivers/uart/uart.c)