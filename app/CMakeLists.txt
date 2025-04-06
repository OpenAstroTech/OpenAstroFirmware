cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(OpenAstroFirmware)

zephyr_syscall_include_directories(include)
zephyr_include_directories(include)

# Add the source files to the target
file(GLOB_RECURSE APP_SOURCES "src/*.cpp" "src/*.c")
target_sources(app PRIVATE ${APP_SOURCES})