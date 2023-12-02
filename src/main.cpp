#include "Platform/Generic.h"
#include "Platform/Windows/Windows.h"
#include "Core/Application.h"

#include "Scene/DefaultSceneGraphFactory.h"

/**
 * Windows entry point
 */
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR pCmdLine, _In_ int nCmdShow)
{
    // Entry point payload
    Platform::Windows::EntrypointPayload entrypointPayload{hInstance, NULL, pCmdLine, nCmdShow};

    // Create scene graph
    DefaultSceneGraphFactory sceneGraphFactory;
    Scene::SceneGraph *sceneGraph = sceneGraphFactory.Make();

    // Create and run application
    Core::Application application(entrypointPayload);
    application.SetInitialSceneGraph(sceneGraph);

    bool isApplicationInitialized = application.Initialize();

    if (!isApplicationInitialized)
    {
        // Press F
        return -1;
    }

    application.Run();

    // Delete scene graph
    delete sceneGraph;

    return 0;
}
