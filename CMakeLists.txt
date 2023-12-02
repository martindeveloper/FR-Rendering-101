cmake_minimum_required(VERSION 3.5.0)

project(FlyingRatRendering101 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_ROOT_DIR ${CMAKE_SOURCE_DIR})

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Zi /Od -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 -DNDEBUG")

    # Disable RTTI and exceptions
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /EHsc- /EHc-")
endif()

# Use latest Windows 11 SDK
set(CMAKE_SYSTEM_VERSION 10.0.22621.0)

# Find dxc.exe compiler for shader compilation
find_program(DXC_COMPILER NAMES dxc.exe PATHS "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")

# Compile HLSL shaders to DXIL
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")

# Set shader model to 6.0
set(SHADER_MODEL "6_0")

# Compile every .hlsl file in shaders directory
file(GLOB HLSL_SOURCE_FILES "${SHADER_DIR}/*.hlsl")

foreach(HLSL_FILE ${HLSL_SOURCE_FILES})
    get_filename_component(SHADER_FILE_NAME ${HLSL_FILE} NAME)
    set(DXIL_FILE_NAME "${SHADER_FILE_NAME}.dxil")
    set(DXIL_FILE "${SHADER_OUTPUT_DIR}/${DXIL_FILE_NAME}")

    # Shader type
    if(${SHADER_FILE_NAME} MATCHES ".*VertexShader.hlsl")
        set(SHADER_TYPE "vs")
    elseif(${SHADER_FILE_NAME} MATCHES ".*PixelShader.hlsl")
        set(SHADER_TYPE "ps")
    else()
        message(FATAL_ERROR "Shaders: Unknown shader type for ${SHADER_FILE_NAME}")
    endif()

    # Add custom command to compile HLSL to DXIL
    add_custom_command(
        OUTPUT ${DXIL_FILE}
        COMMAND ${DXC_COMPILER} -T ${SHADER_TYPE}_${SHADER_MODEL} -E main -Fo ${DXIL_FILE} ${HLSL_FILE}
        DEPENDS ${HLSL_FILE}
        COMMENT "Shaders: Compiling ${SHADER_FILE_NAME} to ${DXIL_FILE_NAME}"
        VERBATIM
    )

    list(APPEND DXIL_FILES ${DXIL_FILE})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${DXIL_FILES})

message(STATUS "WINDOWS_TARGET_PLATFORM_VERSION: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

# Include every .cpp file in src and its subdirectories
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Include every .h file in include and its subdirectories
file(GLOB_RECURSE HEADERS "include/*.h")

add_definitions(-DPROJECT_ROOT_DIRECTORY="${PROJECT_ROOT_DIR}")
add_definitions(-DPROJECT_SHADER_BYTECODE_DIRECTORY="${SHADER_OUTPUT_DIR}")
add_definitions(-DPROJECT_SHADER_MODEL="${SHADER_MODEL}")

# Add executable or library
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

# Set Windows subsystem to use WinMain instead of main
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

# Link against windows libraries
target_link_libraries(${PROJECT_NAME} PRIVATE user32 gdi32 d3d12 dxgi d3dcompiler)
