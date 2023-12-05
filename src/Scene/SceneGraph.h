#pragma once

#ifndef SCENE_SCENEGRAPH_H
#define SCENE_SCENEGRAPH_H

#include "../Platform/Windows/Std.h"
#include "../Core/BaseObject.h"
#include "SceneNode.h"

namespace Scene
{
    class SceneGraph : public Core::BaseObject
    {
    private:
        std::vector<SceneNode *> Nodes; // Root nodes

    public:
        ~SceneGraph()
        {
            // Delete all root nodes
            for (Scene::SceneNode *sceneNode : this->Nodes)
            {
                delete sceneNode;
            }
        }

        /**
         * Add a node to the scene graph.
         * @param sceneNode The node to add.
         */
        void AddNode(SceneNode *sceneNode)
        {
            if (sceneNode && sceneNode->GetParent() == nullptr)
            {
                this->Nodes.push_back(sceneNode);
            }
        }

        /**
         * Remove a node from the scene graph.
         * @param sceneNode The node to remove.
         */
        void RemoveNode(SceneNode *sceneNode)
        {
            this->Nodes.erase(std::remove(this->Nodes.begin(), this->Nodes.end(), sceneNode), this->Nodes.end());

            // TODO: Delete sceneNode? Maybe not, because it's not owned by this class
            // delete sceneNode;
        }

        // Iterator support for root nodes
        using Iterator = std::vector<SceneNode *>::iterator;
        using ConstIterator = std::vector<SceneNode *>::const_iterator;

        Iterator begin() { return this->Nodes.begin(); }
        Iterator end() { return this->Nodes.end(); }
        ConstIterator begin() const { return this->Nodes.begin(); }
        ConstIterator end() const { return this->Nodes.end(); }
    };
}

#endif // SCENE_SCENEGRAPH_H
