# Dependencies
find_package(OpenGL REQUIRED)

find_package(Python3 3.2 REQUIRED)

# Generate dispatch.gen.cpp
# For compatibility with windows version we put this in the source directory
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_LIST_DIR}/framework/dispatch.gen.cpp"
    COMMAND Python3::Interpreter "${CMAKE_CURRENT_LIST_DIR}/framework/dispatch_generator.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/framework"
    DEPENDS "${CMAKE_CURRENT_LIST_DIR}/framework/dispatch_generator.py"
    COMMENT "Generate layer dispatcher"
)

# Modify the "version.h" header
set(XrApiLayer_quad_views_foveated_VERSION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/version.h")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
               "${XrApiLayer_quad_views_foveated_VERSION_HEADER}")


add_compile_definitions(LAYER_NAME="XR_APILAYER_MBUCCHIA_quad_views_foveated")

# Source files
set(XrApiLayer_quad_views_foveated_SRCS
    ../external/fmt/src/format.cc
    framework/dispatch.cpp
    framework/dispatch.gen.cpp # generated
    framework/entry.cpp
    framework/log.cpp
    layer.cpp
    pch.cpp
    utils/composition.cpp
    utils/general.cpp
    utils/input.cpp
)

# Win32
if(WIN32)
    list(APPEND XrApiLayer_quad_views_foveated_SRCS
        utils/d3d11.cpp
        utils/d3d12.cpp
    )
endif()

# Build XrApiLayer_quad_views_foveated as a library
add_library(XrApiLayer_quad_views_foveated MODULE ${XrApiLayer_quad_views_foveated_SRCS})

# Ensure required C++ standards are available
target_compile_features(XrApiLayer_quad_views_foveated
    PRIVATE
        cxx_std_17
)

target_include_directories(XrApiLayer_quad_views_foveated
    PRIVATE
        ${PROJECT_BINARY_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/framework
        ${PROJECT_SOURCE_DIR}/external/fmt/include
        ${PROJECT_SOURCE_DIR}/external/OpenXR-SDK/include
        ${PROJECT_SOURCE_DIR}/external/OpenXR-SDK/src/common
        ${PROJECT_SOURCE_DIR}/external/OpenXR-MixedReality/shared/XrUtility
        ${PROJECT_SOURCE_DIR}/external/FidelityFX-CAS/ffx-cas
)

target_link_libraries(XrApiLayer_quad_views_foveated
    PRIVATE
        ${OPENGL_LIBRARIES}
)

set_target_properties(XrApiLayer_quad_views_foveated
    PROPERTIES
        VERSION         ${PROJECT_VERSION}
)
