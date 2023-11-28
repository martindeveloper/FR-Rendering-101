#pragma once

#ifndef PLATFORM_WINDOWS_ENTRYPOINT_H
#define PLATFORM_WINDOWS_ENTRYPOINT_H

#include "./OS.h"

struct EntrypointPayload
{
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPSTR pCmdLine;
	int nCmdShow;
};

#endif // PLATFORM_WINDOWS_ENTRYPOINT_H
