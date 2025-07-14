# Copyright (c) 2025, OpenAstroTech
# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources_ifdef(CONFIG_LX200 lx200.c)
zephyr_library_include_directories(${CMAKE_CURRENT_SOURCE_DIR})