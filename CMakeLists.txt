cmake_minimum_required(VERSION 3.5.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Zi /Od -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /O2 -DNDEBUG")

# Use latest Windows 11 SDK
set(CMAKE_SYSTEM_VERSION 10.0.22621.0)

project(FlyingRatRendering101 VERSION 0.1.0 LANGUAGES CXX)
message(STATUS "WINDOWS_TARGET_PLATFORM_VERSION: ${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}")

# Include every .cpp file in src and its subdirectories
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Include every .h file in include and its subdirectories
file(GLOB_RECURSE HEADERS "include/*.h")

# Add executable or library
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

# Set Windows subsystem to use WinMain instead of main
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

# Link against windows libraries
target_link_libraries(${PROJECT_NAME} PRIVATE user32 gdi32)
