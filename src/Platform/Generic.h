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

// ERROR_FATAL macro which calls ERROR_FATAL_<PLATFORM_NAME> macro
#ifdef PLATFORM_WINDOWS
#define ERROR_FATAL(error_message) ERROR_FATAL_WINDOWS(error_message)
#else
#pragma error("Platform.Generic: Unsupported platform")
#endif

#endif // PLATFORM_GENERIC_H
