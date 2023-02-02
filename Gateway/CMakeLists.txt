# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/ubxlib)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nina_w15_wifi_mqtt)

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
FILE(GLOB app_sources src/*.c)

target_sources(app PRIVATE ${app_sources})
