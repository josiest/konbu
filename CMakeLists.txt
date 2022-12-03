cmake_minimum_required(VERSION 3.23)
project(konbu)

add_executable(sketch src/sketch.cpp)
set_target_properties(sketch PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED TRUE)

find_package(yaml-cpp REQUIRED)
target_link_libraries(sketch PRIVATE yaml-cpp)