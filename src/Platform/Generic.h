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

#endif // PLATFORM_GENERIC_H
