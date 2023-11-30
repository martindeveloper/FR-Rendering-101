#pragma once

#ifndef PLATFORM_GENERIC_H
#define PLATFORM_GENERIC_H

// Check if we are in debug mode
#if defined(DEBUG) || defined(_DEBUG)
#define BUILD_TYPE "Debug"
#define BUILD_TYPE_DEBUG

#pragma message("Platform.Generic: Build type is Debug")
#else
#define BUILD_TYPE "Release"
#define BUILD_TYPE_RELEASE

#pragma message("Platform.Generic: Build type is Release")
#endif

// Detect platform
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#define PLATFORM_NAME "Windows"
#else
#pragma error("Platform.Generic: Unsupported platform")
#endif

#ifndef PROJECT_ROOT_DIR
#define PROJECT_ROOT_DIR L"" // Should be set by CMake
#endif

#define SHADER_PATH(shader) PROJECT_ROOT_DIR L"\\shaders\\" L##shader

#endif // PLATFORM_GENERIC_H
