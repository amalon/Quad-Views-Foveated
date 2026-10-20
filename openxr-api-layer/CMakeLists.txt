# Dependencies
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
#find_package(OpenXR 1.0.34 REQUIRED)

#   sed -i "s/FILEVERSION .*$/FILEVERSION %major%,%minor%,%patch%,0/g" $(ProjectDir)\resource.rc
#   sed -i "s/PRODUCTVERSION .*$/PRODUCTVERSION %major%,%minor%,%patch%,0/g" $(ProjectDir)\resource.rc
#   sed -i "s/VALUE \"FileVersion\", \".*\"$/VALUE \"FileVersion\", \"%major%.%minor%.%patch%.0\"/g" $(ProjectDir)\resource.rc
#   sed -i "s/VALUE \"ProductVersion\", \".*\"$/VALUE \"ProductVersion\", \"%major%.%minor%.%patch%.0\"/g" $(ProjectDir)\resource.rc

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


set(LAYER_NAME "XR_APILAYER_MBUCCHIA_quad_views_foveated")
add_compile_definitions(LAYER_NAME="${LAYER_NAME}")

# Generate API layer json file
file(READ "${CMAKE_CURRENT_LIST_DIR}/openxr-api-layer.json" FILE_CONTENTS)
string(REPLACE ".\\\\XR_APILAYER_name.dll" "../../../../../lib/$<TARGET_FILE_NAME:XrApiLayer_quad_views_foveated>" FILE_CONTENTS "${FILE_CONTENTS}")
string(REPLACE "XR_APILAYER_name" "@LAYER_NAME@" FILE_CONTENTS "${FILE_CONTENTS}")
# Expand variables
file(CONFIGURE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${LAYER_NAME}.json.in" CONTENT "${FILE_CONTENTS}")
# Expand generator expressions 
file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${LAYER_NAME}.json" INPUT "${CMAKE_CURRENT_BINARY_DIR}/${LAYER_NAME}.json.in")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/openxr-api-layer.json")

install(FILES       "${CMAKE_CURRENT_BINARY_DIR}/${LAYER_NAME}.json"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/openxr/1/api_layers/implicit.d"
)

# Shaders
#    <FxCompile Include="ProjectionPS.hlsl">
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Pixel</ShaderType>
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">Pixel</ShaderType>
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">Pixel</ShaderType>
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Release|x64'">Pixel</ShaderType>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">5.0</ShaderModel>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">5.0</ShaderModel>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">5.0</ShaderModel>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Release|x64'">5.0</ShaderModel>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
#      </ObjectFileOutput>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
#      </ObjectFileOutput>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
#      </ObjectFileOutput>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Release|x64'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|x64'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
#      </ObjectFileOutput>
#    </FxCompile>
#    <FxCompile Include="ProjectionVS.hlsl">
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Vertex</ShaderType>
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">Vertex</ShaderType>
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">Vertex</ShaderType>
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Release|x64'">Vertex</ShaderType>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">5.0</ShaderModel>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">5.0</ShaderModel>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">5.0</ShaderModel>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Release|x64'">5.0</ShaderModel>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
#      </ObjectFileOutput>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
#      </ObjectFileOutput>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
#      </ObjectFileOutput>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Release|x64'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|x64'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
#      </ObjectFileOutput>
#    </FxCompile>
#    <FxCompile Include="SharpeningCS.hlsl">
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">Compute</ShaderType>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">5.0</ShaderModel>
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">Compute</ShaderType>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">5.0</ShaderModel>
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">Compute</ShaderType>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">5.0</ShaderModel>
#      <ShaderType Condition="'$(Configuration)|$(Platform)'=='Release|x64'">Compute</ShaderType>
#      <ShaderModel Condition="'$(Configuration)|$(Platform)'=='Release|x64'">5.0</ShaderModel>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
#      </ObjectFileOutput>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">
#      </ObjectFileOutput>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
#      </ObjectFileOutput>
#      <VariableName Condition="'$(Configuration)|$(Platform)'=='Release|x64'">g_%(Filename)</VariableName>
#      <HeaderFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|x64'">$(IntDir)%(Filename).h</HeaderFileOutput>
#      <ObjectFileOutput Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
#      </ObjectFileOutput>
#      <AdditionalIncludeDirectories Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">$(SolutionDir)\external\FidelityFX-CAS\ffx-cas</AdditionalIncludeDirectories>
#      <AdditionalIncludeDirectories Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">$(SolutionDir)\external\FidelityFX-CAS\ffx-cas</AdditionalIncludeDirectories>
#      <AdditionalIncludeDirectories Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">$(SolutionDir)\external\FidelityFX-CAS\ffx-cas</AdditionalIncludeDirectories>
#      <AdditionalIncludeDirectories Condition="'$(Configuration)|$(Platform)'=='Release|x64'">$(SolutionDir)\external\FidelityFX-CAS\ffx-cas</AdditionalIncludeDirectories>
#      <PreprocessorDefinitions Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">CAS_SAMPLE_FP16=0;CAS_SAMPLE_SHARPEN_ONLY=1;WIDTH=64;HEIGHT=1;DEPTH=1</PreprocessorDefinitions>
#      <PreprocessorDefinitions Condition="'$(Configuration)|$(Platform)'=='Release|Win32'">CAS_SAMPLE_FP16=0;CAS_SAMPLE_SHARPEN_ONLY=1;WIDTH=64;HEIGHT=1;DEPTH=1</PreprocessorDefinitions>
#      <PreprocessorDefinitions Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">CAS_SAMPLE_FP16=0;CAS_SAMPLE_SHARPEN_ONLY=1;WIDTH=64;HEIGHT=1;DEPTH=1</PreprocessorDefinitions>
#      <PreprocessorDefinitions Condition="'$(Configuration)|$(Platform)'=='Release|x64'">CAS_SAMPLE_FP16=0;CAS_SAMPLE_SHARPEN_ONLY=1;WIDTH=64;HEIGHT=1;DEPTH=1</PreprocessorDefinitions>

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
    utils/d3d11.cpp
    utils/d3d12.cpp
    utils/general.cpp
    utils/input.cpp
    utils/opengl.cpp
)

# OpenGL graphics bindings
find_package(OpenGL COMPONENTS EGL GLX OpenGL)
if(OpenGL_OpenGL_FOUND)
    add_compile_definitions(USE_OPENGL)
endif()
if(OpenGL_GLX_FOUND)
    add_compile_definitions(USE_GLX)
endif()
if(OpenGL_EGL_FOUND)
    add_compile_definitions(USE_EGL)
endif()
find_package(X11 COMPONENTS xcb)
if (X11_xcb_FOUND)
    add_compile_definitions(USE_XCB)
endif()

# Vulkan graphics bindings
find_package(Vulkan)
if(Vulkan_Found)
    add_compile_definitions(USE_VULKAN)
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
        #${PROJECT_SOURCE_DIR}/external/OpenXR-MixedReality/shared/SampleShared
        #${PROJECT_SOURCE_DIR}/external/OpenXR-MixedReality/shared/ext/DirectXMath/Inc
        ${PROJECT_SOURCE_DIR}/external/FidelityFX-CAS/ffx-cas
)

target_link_libraries(XrApiLayer_quad_views_foveated
    PRIVATE
        ${OPENGL_LIBRARIES}
        glm::glm
)

set_target_properties(XrApiLayer_quad_views_foveated
    PROPERTIES
        VERSION         ${PROJECT_VERSION}
)
