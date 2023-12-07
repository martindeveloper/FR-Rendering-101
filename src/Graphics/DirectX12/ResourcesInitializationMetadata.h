#pragma once

#ifndef GRAPHICS_DIRECTX12_RESOURCESINITIALIZATIONMETADATA_H
#define GRAPHICS_DIRECTX12_RESOURCESINITIALIZATIONMETADATA_H

#include <d3d12.h>
#include <wrl.h>

namespace Graphics::DirectX12
{
    struct ResourcesInitializationMetadata
    {
        Microsoft::WRL::ComPtr<ID3D12Device> Device;
        UINT BackBufferCount;
    };
}

#endif // GRAPHICS_DIRECTX12_RESOURCESINITIALIZATIONMETADATA_H
