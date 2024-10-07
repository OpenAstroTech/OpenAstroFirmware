# Meade LX200 protocol library
add_subdirectory_ifdef(CONFIG_LX200 lx200)

# set(LIB_INCLUDE_DIRS "" PARENT_SCOPE)

# list_append_ifdef(CONFIG_LX200
#   LIB_INCLUDE_DIRS
#   ${LX200_INCLUDE_DIRS}
# )

# message(STATUS "LX200_INCLUDE_DIRS: ${LX200_INCLUDE_DIRS}")