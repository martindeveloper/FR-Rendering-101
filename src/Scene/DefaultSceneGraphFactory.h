#pragma once

#ifndef SCENE_DEFAULT_SCENE_GRAPH_FACTORY_H
#define SCENE_DEFAULT_SCENE_GRAPH_FACTORY_H

#include "../Platform/Platform.h"
#include "SceneGraph.h"
#include "SceneNode.h"
#include "Entities/TriangleEntity.h"

/**
 * @brief Default scene graph factory with hardcoded nodes
 */
class DefaultSceneGraphFactory
{
public:
    /**
     * @brief Make a scene graph, the caller is owner and responsible for deleting the scene graph.
     * @return The scene graph.
     */
    Scene::SceneGraph *Make()
    {
        // Scene::SceneGraph *sceneGraph = new Scene::SceneGraph();
        // TODO: Testing placement new with custom allocator, remove this and use base class new override
        Scene::SceneGraph *sceneGraph = static_cast<Scene::SceneGraph *>(Platform::Allocate(sizeof(Scene::SceneGraph)));
        new (sceneGraph) Scene::SceneGraph();

        // Create entities
        Scene::IEntity *triangleEntity = new Scene::Entities::TriangleEntity();

        // Create scene nodes
        Scene::SceneNode *rootNode = new Scene::SceneNode();
        Scene::SceneNode *triangleNode = new Scene::SceneNode(triangleEntity); // Transferring ownership of triangleEntity to triangleNode

        // Add nodes to scene graph
        sceneGraph->AddNode(rootNode);
        sceneGraph->AddNode(triangleNode);

        return sceneGraph;
    }
};

#endif // SCENE_DEFAULT_SCENE_GRAPH_FACTORY_H
