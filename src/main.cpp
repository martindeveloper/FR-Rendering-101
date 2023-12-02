#include "Platform/Generic.h"
#include "Platform/Windows/Windows.h"
#include "Application/Application.h"

using namespace Platform::Windows;

/**
 * Windows entry point
 */
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
    // Entry point payload
    EntrypointPayload entrypointPayload{hInstance, NULL, pCmdLine, nCmdShow};

    // Create application
    Application application(entrypointPayload);
    application.Run();

    return 0;
}
