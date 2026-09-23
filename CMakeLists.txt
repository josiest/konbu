cmake_minimum_required(VERSION 3.23)
project(pi-konbu VERSION 0.4.0)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_FLAGS "-freflection")

add_library(pi-konbu INTERFACE)
target_sources(pi-konbu
    INTERFACE FILE_SET HEADERS
    BASE_DIRS include
    FILES
        include/pi/konbu.hpp)
set_target_properties(pi-konbu PROPERTIES EXPORT_NAME konbu)

find_package(yaml-cpp REQUIRED)
target_link_libraries(pi-konbu INTERFACE yaml-cpp::yaml-cpp)

include(pi-cmake-library-helpers/PiLibraryHelpers.cmake)
install_pi_package(konbu)

option(EXAMPLES "Build examples" ON)
if (EXAMPLES)
    add_subdirectory(examples)
endif()