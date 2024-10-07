# Print a status message indicating that the lx200 library is being used.
message(STATUS "Using lx200")

# Define a new Zephyr library. This initializes a library target within the Zephyr build system.
zephyr_library()

# Add the source files to the previously defined Zephyr library.
zephyr_library_sources(
    src/MeadeLX200Parser.cpp
)

# Include the specified directories in the current Zephyr library.
zephyr_include_directories(include)

# Set the LX200_INCLUDE_DIRS variable to the current source directory.
# This is needed to make the include directories available to other parts of the build system.
# set(LX200_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)

# Print the value of LX200_INCLUDE_DIRS to the console.
# message(STATUS "LX200_INCLUDE_DIRS: ${LX200_INCLUDE_DIRS}")

zephyr_library_include_directories(include)

# Conditionally link the current Zephyr library to other libraries if the CONFIG_LX200 configuration option is enabled.
# If CONFIG_LX200 is defined, this links the current library (${ZEPHYR_CURRENT_LIBRARY}) to the specified libraries.
zephyr_link_libraries(${ZEPHYR_CURRENT_LIBRARY})
