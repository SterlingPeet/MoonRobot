cmake_minimum_required(VERSION 3.24)

project(cFS LANGUAGES C)

set(CMAKE_BUILD_TYPE "debug")
set(CMAKE_INSTALL_PREFIX "/exe")

add_subdirectory(cfe)


function (_get_all_cmake_targets out_var current_dir)
    get_property(targets DIRECTORY ${current_dir} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(subdirs DIRECTORY ${current_dir} PROPERTY SUBDIRECTORIES)

    foreach(subdir ${subdirs})
        _get_all_cmake_targets(subdir_targets ${subdir})
        list(APPEND targets ${subdir_targets})
    endforeach()

    set(${out_var} ${targets} PARENT_SCOPE)
endfunction()

# Run at end of top-level CMakeLists
_get_all_cmake_targets(all_targets ${CMAKE_CURRENT_LIST_DIR})
message(" all targets ${all_targets}")
