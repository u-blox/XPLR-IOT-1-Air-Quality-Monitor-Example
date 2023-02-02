# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/scd4x_oot_driver)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(scd4x_broadcaster)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/scd4x_oot_driver/drivers/sensor/scd4x)

FILE(GLOB app_sources src/*.c)
FILE(GLOB scd4x_drv scd4x_oot_driver/drivers/sensor/scd4x/*.c)

target_sources(app PRIVATE ${scd4x_drv})
target_sources(app PRIVATE ${app_sources})
