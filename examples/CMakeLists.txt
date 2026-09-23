add_executable(sketch)
target_sources(sketch
    PRIVATE
        sketch.cpp
    PUBLIC FILE_SET HEADERS
    FILES
        gold.hpp)

set_target_properties(sketch PROPERTIES
        CXX_STANDARD 26
        CXX_STANDARD_REQUIRED TRUE)

target_link_libraries(sketch
        INTERFACE pi-konbu
        PRIVATE yaml-cpp::yaml-cpp)

target_compile_options(sketch PRIVATE "-freflection")
target_include_directories(sketch PUBLIC ../include)
