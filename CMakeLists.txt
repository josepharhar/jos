# This file is just here for youcompleteme, for actual compiling use makefile

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/kernel)
include_directories(${CMAKE_SOURCE_DIR}/src/user)
include_directories(${CMAKE_SOURCE_DIR}/src/shared)

file(GLOB_RECURSE CXX_SOURCES "src/*/*.cc")
file(GLOB_RECURSE C_SOURCES "src/*/*.c")

add_executable("cmake_executable" ${CXX_SOURCES} ${C_SOURCES})
