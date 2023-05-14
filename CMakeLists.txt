cmake_minimum_required(VERSION 3.24)

project(cFS LANGUAGES C)

set(CMAKE_BUILD_TYPE "debug")
set(CMAKE_INSTALL_PREFIX "exe")

# use this if you want to compile for ARM and have the path to the compiler set up.
#set(SIMULATION "arm64-pi-linux-gnueabi")

add_subdirectory(cfe)

message ( dep for ${CMAKE_BINARY_DIR})

execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_BINARY_DIR}/cfe/tools ${CMAKE_BINARY_DIR}/tools )

add_custom_target(mission ALL)
add_dependencies(mission mission-all)
