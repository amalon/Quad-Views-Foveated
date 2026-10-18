# Top level CMakeLists.txt
cmake_minimum_required(VERSION 3.18)

# Extract version numbers from version.info
file(READ "version.info" version)
string(REGEX MATCH "major=([0-9]*)" _ ${version})
set(XrApiLayer_quad_views_foveated_MAJOR_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "minor=([0-9]*)" _ ${version})
set(XrApiLayer_quad_views_foveated_MINOR_VERSION ${CMAKE_MATCH_1})
string(REGEX MATCH "patch=([0-9]*)" _ ${version})
set(XrApiLayer_quad_views_foveated_PATCH_VERSION ${CMAKE_MATCH_1})
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "version.info")

set(XrApiLayer_quad_views_foveated_VERSION "${XrApiLayer_quad_views_foveated_MAJOR_VERSION}.${XrApiLayer_quad_views_foveated_MINOR_VERSION}.${XrApiLayer_quad_views_foveated_PATCH_VERSION}")

project(XrApiLayer_quad_views_foveated
        VERSION     ${XrApiLayer_quad_views_foveated_VERSION}
        DESCRIPTION "OpenXR layer to add quad views foveated"
)

include(GNUInstallDirs)

# Source files in openxr-api-layer/
add_subdirectory(openxr-api-layer)

# Install library
install(TARGETS                   XrApiLayer_quad_views_foveated
        LIBRARY DESTINATION       "${CMAKE_INSTALL_LIBDIR}"
)
