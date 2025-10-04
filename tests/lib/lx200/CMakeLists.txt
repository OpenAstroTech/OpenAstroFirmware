# Copyright (c) 2025, OpenAstroTech
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lx200_lib_test)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C++20 test sources
target_sources(app PRIVATE 
    src/main.cpp
    src/test_parser.cpp
    src/test_coordinates.cpp
    src/test_commands.cpp
    src/test_integration.cpp
)

# Compiler flags for embedded C++20
target_compile_options(app PRIVATE
  -fno-exceptions
  -fno-rtti
  -fno-threadsafe-statics
)
