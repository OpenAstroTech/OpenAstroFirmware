# Copyright (c) 2025, OpenAstroTech
# SPDX-License-Identifier: Apache-2.0

zephyr_library()

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C++20 source files for LX200 protocol implementation
zephyr_library_sources_ifdef(CONFIG_LX200
  lx200.cpp
  parser.cpp
  coordinates.cpp
)

# Public include directory
zephyr_library_include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
)

# Compiler flags for embedded C++20
zephyr_library_compile_options(
  -fno-exceptions
  -fno-rtti
  -fno-threadsafe-statics
)