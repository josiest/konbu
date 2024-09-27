cmake_minimum_required(VERSION 3.23)
project(pi-konbu VERSION 0.3.0)

add_library(pi-konbu INTERFACE)
target_sources(pi-konbu INTERFACE
        FILE_SET HEADERS
        BASE_DIRS include
        FILES include/konbu/read.hpp
              include/konbu/expect.hpp)

set_target_properties(pi-konbu PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED True)

add_library(pi::konbu ALIAS pi-konbu)

find_package(yaml-cpp REQUIRED)
target_link_libraries(pi-konbu INTERFACE yaml-cpp)

add_subdirectory(examples)

#
# Export and install the libary
#

include(GNUInstallDirs)

# specify header file directories for building/installing
target_include_directories(pi-konbu INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# install the include files
install(TARGETS pi-konbu FILE_SET HEADERS
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# specify the library targets
install(TARGETS pi-konbu
        EXPORT pi-konbu-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/pi/konbu
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILE_SET HEADERS)

# install the export details
install(EXPORT pi-konbu-targets
        FILE pi-konbu-targets.cmake
        NAMESPACE pi::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pi/konbu)

#
# Create and install the package
#

include(CMakePackageConfigHelpers)

# generate the package configuration file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/pi-konbu-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/pi-konbu-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pi/konbu)

# generate the version file for the config file
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/pi-konbu-config-version.cmake
        COMPATIBILITY AnyNewerVersion)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/pi-konbu-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/pi-konbu-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/pi/konbu)

# allow exporting from build tree
export(EXPORT pi-konbu-targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/pi-konbu-targets.cmake NAMESPACE pi::)