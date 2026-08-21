cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(esp32_drivers)

include_directories(include)
include_directories(drivers/uart)

target_sources(app PRIVATE src/main.c src/uart_log_backend.c drivers/uart/uart.c)