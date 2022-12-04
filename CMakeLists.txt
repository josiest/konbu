cmake_minimum_required(VERSION 3.23)
project(konbu VERSION 0.2.1)

add_library(konbu INTERFACE)
target_sources(konbu INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES include/konbu/konbu.h)

find_package(yaml-cpp REQUIRED)
target_link_libraries(konbu INTERFACE yaml-cpp)

#
# Export and install the libary
#

include(GNUInstallDirs)

# specify header file directories for building/installing
target_include_directories(konbu INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# install the include files
install(TARGETS konbu FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# specify the library targets
install(TARGETS konbu
        EXPORT konbu-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/konbu
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILE_SET HEADERS)

# install the export details
install(EXPORT konbu-targets
        FILE konbu-targets.cmake
        NAMESPACE ion::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/konbu)

add_library(ion::konbu STATIC IMPORTED)
set_target_properties(ion::konbu PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include")

#
# Create and install the package
#

include(CMakePackageConfigHelpers)

# generate the package configuration file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/konbu-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/konbu-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/konbu)

# generate the version file for the config file
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/konbu-config-version.cmake
        COMPATIBILITY AnyNewerVersion)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/konbu-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/konbu-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/konbu)

# allow exporting from build tree
export(EXPORT konbu-targets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/konbu-targets.cmake NAMESPACE ion::)

add_executable(sketch examples/sketch.cpp)
target_include_directories(sketch PRIVATE include)

set_target_properties(sketch PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED TRUE)

target_link_libraries(sketch
        INTERFACE ion::konbu
        PRIVATE yaml-cpp)