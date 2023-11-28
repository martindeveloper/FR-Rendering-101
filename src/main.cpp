#include "Platform/Windows/Windows.h"
#include "Application/Application.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
    // Entry point payload
    EntrypointPayload entrypointPayload = {};
    entrypointPayload.hInstance = hInstance;
    entrypointPayload.hPrevInstance = NULL;
    entrypointPayload.pCmdLine = pCmdLine;
    entrypointPayload.nCmdShow = nCmdShow;

    // Create application
    Application application(entrypointPayload);
    application.Run();

    return 0;
}
