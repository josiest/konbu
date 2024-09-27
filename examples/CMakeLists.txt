add_executable(sketch sketch.cpp)
target_include_directories(sketch PRIVATE ../include)

set_target_properties(sketch PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE)

target_link_libraries(sketch
        INTERFACE ion::konbu
        PRIVATE yaml-cpp)
