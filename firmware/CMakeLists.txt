# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

list(APPEND EXTRA_CONF_FILE 
    "logging.conf"
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(OpenAstroFirmware)

add_subdirectory(lib)

zephyr_syscall_include_directories(include)
zephyr_include_directories(include)

# print the value of LIB_INCLUDE_DIRS
# message(STATUS "LIB_INCLUDE_DIRS: ${LIB_INCLUDE_DIRS}")

include(CMakePrintHelpers)
cmake_print_variables(PROJECT_SOURCE_DIR)

# Add the source files to the target
file(GLOB_RECURSE APP_SOURCES "src/*.cpp" "src/*.c")
target_sources(app PRIVATE ${APP_SOURCES})
