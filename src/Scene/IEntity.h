#pragma once

#ifndef SCENE_IENTITY_H
#define SCENE_IENTITY_H

#include "../Graphics/DirectX12/FrameMetadata.h"
#include "../Graphics/DirectX12/ResourcesInitializationMetadata.h"
#include "../Platform/Windows/Windows.h"
#include "../Core/BaseObject.h"

namespace Scene
{
    class IEntity : public Core::BaseObject
    {
    public:
        virtual ~IEntity() = default;

        virtual void OnResourceCreate(Graphics::DirectX12::ResourcesInitializationMetadata *resourceMetadata) = 0;
        virtual void OnRender(Graphics::DirectX12::FrameMetadata *frameMetadata) = 0;
        virtual void OnUpdate(uint64_t frame) = 0;
    };
}

#endif // SCENE_IENTITY_H
