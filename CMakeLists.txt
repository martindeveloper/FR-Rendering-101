cmake_minimum_required(VERSION 3.5.0)

project(FlyingRatRendering101 VERSION 0.1.0 LANGUAGES CXX)

message(STATUS "[Executable] Windows SDK version: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

# Add executable or library
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

# Set properties or definitions based on configuration
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    "$<$<CONFIG:Debug>:DEBUG_BUILD>"
    "$<$<CONFIG:Release>:RELEASE_BUILD>"
)

include("cmake/DX12AgilitySDK.cmake")
include("cmake/DX12Shaders.cmake")
include("cmake/WindowsPix.cmake")

# Generic definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_ROOT_DIRECTORY="${CMAKE_SOURCE_DIR}"
)

if (MSVC)
    message(STATUS "[Executable] Using MSVC compiler")
    message(STATUS "[Executable] Will try to disable RTTI and exceptions in the CRT")

    # Add extra compilation flags specific to MSVC
    # /GR- disables RTTI
    # /EHs-c- disables exceptions
    # /MTd and /MT are used to statically link the CRT (/MDd and /MD are the default)
    # /Zi enables debugging information (in Debug mode)
    # /Od disables optimization (in Debug mode)
    # /O2 enables optimization in favor of speed (in Release mode)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Zi /Od -DDEBUG /GR- /EHs-c-")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 -DNDEBUG /GR- /EHs-c-")

    # Disable RTTI and exceptions flags in the CRT as CMake forces them on by default
	string(REGEX REPLACE "/GR" "/GR-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REGEX REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # Define _HAS_EXCEPTIONS to zero to disable exceptions in the CRT
    target_compile_definitions(${PROJECT_NAME} PRIVATE _HAS_EXCEPTIONS=0)

    # Link against Windows libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        user32.lib
        gdi32.lib
        d3d12.lib
        dxgi.lib
    )
endif()
