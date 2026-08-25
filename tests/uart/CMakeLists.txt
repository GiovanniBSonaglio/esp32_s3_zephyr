cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(uart_test)

set(DRIVERS_DIR ${PROJECT_SOURCE_DIR}/../../drivers/)

include_directories(${DRIVERS_DIR}/uart/)

target_sources(app PRIVATE src/main.c ${DRIVERS_DIR}/uart.c)