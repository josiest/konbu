cmake_minimum_required(VERSION 3.23)
project(konbu)

add_library(konbu INTERFACE)
target_sources(konbu INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include/konbu
        include/konbu/konbu.h)

add_executable(sketch src/sketch.cpp)
target_include_directories(sketch PRIVATE include)

set_target_properties(sketch PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE)

find_package(yaml-cpp REQUIRED)
add_library(konbu::konbu ALIAS konbu)
target_link_libraries(sketch
        INTERFACE konbu::konbu
        PRIVATE yaml-cpp)