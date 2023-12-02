#pragma once

#ifndef SCENE_IENTITY_H
#define SCENE_IENTITY_H

#include <d3d12.h>
#include "../Platform/Windows/Windows.h"

namespace Scene
{
    class IEntity
    {
    public:
        virtual ~IEntity() = default;

        virtual void OnResourceCreate(Microsoft::WRL::ComPtr<ID3D12Device> device) = 0;
        virtual void OnRender(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList) = 0;
        virtual void OnUpdate(uint64_t frame) = 0;
    };
}

#endif // SCENE_IENTITY_H
