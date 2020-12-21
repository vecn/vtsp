cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_COMPILER "gcc")
# set(CMAKE_C_COMPILER "clang")

project(VTSP)

# Build Library
file(GLOB_RECURSE NB_SOURCES "sources/*.c")

include_directories("headers")

add_library(objlib OBJECT ${NB_SOURCES})
target_compile_options(objlib PUBLIC -std=c99 -Wall)
set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE 1) # -fPIC
add_library(vtsp SHARED $<TARGET_OBJECTS:objlib>)
add_library(vtsp_static STATIC $<TARGET_OBJECTS:objlib>)
set_target_properties(vtsp_static PROPERTIES OUTPUT_NAME vtsp)

# Build Unit Tests
file(GLOB_RECURSE sources_tests "tests/*.c")
add_executable(tests ${sources_tests} tests/main.c)
target_compile_options(utests PUBLIC -std=c99 -Wall)
target_link_libraries(utests cunit nbots m)
