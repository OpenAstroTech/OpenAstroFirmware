cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(OpenAstroFirmware)

add_subdirectory_ifdef(CONFIG_MOUNT src/mount)

target_sources(app PRIVATE src/main.cpp)
