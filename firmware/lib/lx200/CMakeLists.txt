# Print a status message indicating that the lx200 library is being used.
message(STATUS "Using lx200")

# Define a new Zephyr library. This initializes a library target within the Zephyr build system.
zephyr_library()

# Add the source files to the previously defined Zephyr library.
zephyr_library_sources(lx200.cpp)

# Conditionally link the current Zephyr library to other libraries if the CONFIG_LX200 configuration option is enabled.
# If CONFIG_LX200 is defined, this links the current library (${ZEPHYR_CURRENT_LIBRARY}) to the specified libraries.
zephyr_link_libraries(${ZEPHYR_CURRENT_LIBRARY})