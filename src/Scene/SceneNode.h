#pragma once

#ifndef SCENE_SCENENODE_H
#define SCENE_SCENENODE_H

#include "../Platform/Windows/Std.h"
#include "../Core/BaseObject.h"
#include "./IEntity.h"

namespace Scene
{
    class SceneNode : public Core::BaseObject
    {
    private:
        IEntity *Entity;
        SceneNode *Parent;
        std::vector<SceneNode *> Children;

    public:
        SceneNode(IEntity *entity = nullptr) : Entity(entity), Parent(nullptr) {}

        ~SceneNode()
        {
            delete this->Entity;

            for (Scene::SceneNode *child : this->Children)
            {
                delete child;
            }
        }

        /**
         * Update the scene node.
         * @param frame The current frame.
         */
        void OnUpdate(uint64_t frame)
        {
            if (this->Entity)
            {
                this->Entity->OnUpdate(frame);
            }

            for (Scene::SceneNode *child : this->Children)
            {
                child->OnUpdate(frame);
            }
        }

        /**
         * Render the scene node.
         * @param commandList The command list to use for rendering.
         */
        void OnRender(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList)
        {
            if (this->Entity)
            {
                this->Entity->OnRender(commandList);
            }

            for (Scene::SceneNode *child : this->Children)
            {
                child->OnRender(commandList);
            }
        }

        /**
         * Create the resource for the scene node.
         * @param device The device to use for resource creation.
         */
        void OnResourceCreate(Microsoft::WRL::ComPtr<ID3D12Device> device)
        {
            if (this->Entity)
            {
                this->Entity->OnResourceCreate(device);
            }

            for (Scene::SceneNode *child : this->Children)
            {
                child->OnResourceCreate(device);
            }
        }

        /**
         * Set the parent of the scene node.
         * @param inParent The parent to set.
         */
        void SetParent(SceneNode *inParent)
        {
            // Don't allow to set parent to nullptr
            if (inParent == nullptr)
            {
                return;
            }

            // Don't allow to set parent to itself
            if (inParent == this->Parent)
            {
                return;
            }

            // Don't allow to set parent to one of its children
            if (inParent == this)
            {
                return;
            }

            // Detach from current parent
            if (this->Parent)
            {
                this->Parent->RemoveChild(this);
            }

            this->Parent = inParent;
            this->Parent->AddChild(this);
        }

        /**
         * Get the parent of the scene node.
         * @return The parent of the scene node.
         */
        SceneNode *GetParent() const
        {
            return this->Parent;
        }

        /**
         * Add a child to the scene node.
         * @param child The child to add.
         */
        void AddChild(SceneNode *child)
        {
            this->Children.push_back(child);
        }

        /**
         * Remove a child from the scene node.
         * @param child The child to remove.
         */
        void RemoveChild(SceneNode *child)
        {
            this->Children.erase(std::remove(this->Children.begin(), this->Children.end(), child), this->Children.end());
        }

        /**
         * Get the children of the scene node.
         * @return The children of the scene node.
         */
        const std::vector<SceneNode *> &GetChildren() const
        {
            return this->Children;
        }

        /**
         * Get the entity of the scene node.
         * @return The entity of the scene node.
         */
        IEntity *GetEntity() const
        {
            return this->Entity;
        }
    };
}

#endif // SCENE_SCENENODE_H
