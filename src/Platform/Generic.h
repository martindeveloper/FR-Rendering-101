#pragma once

#ifndef PLATFORM_GENERIC_H
#define PLATFORM_GENERIC_H

// Check if we are in debug mode
#if defined(DEBUG) || defined(_DEBUG)
#define BUILD_TYPE "Debug"
#define BUILD_TYPE_DEBUG
#else
#define BUILD_TYPE "Release"
#define BUILD_TYPE_RELEASE
#endif

// Detect platform
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#define PLATFORM_NAME "Windows"
#else
#error "Platform.Generic: Unsupported platform"
#endif

#ifndef PROJECT_ROOT_DIRECTORY
#pragma message("Platform.Generic: PROJECT_ROOT_DIRECTORY is not defined")
#define PROJECT_ROOT_DIRECTORY L"\\" // For editor
#endif

#ifndef PROJECT_SHADER_BYTECODE_DIRECTORY
#pragma message("Platform.Generic: PROJECT_SHADER_BYTECODE_DIRECTORY is not defined")
#define PROJECT_SHADER_BYTECODE_DIRECTORY L"\\" // For editor
#endif

#define SHADER_PATH(shader) PROJECT_ROOT_DIRECTORY L"\\shaders\\" L##shader
#define SHADER_BYTECODE_PATH(shader, type) PROJECT_SHADER_BYTECODE_DIRECTORY L"\\" L##shader L##type L"Shader.hlsl.dxil"

#endif // PLATFORM_GENERIC_H
