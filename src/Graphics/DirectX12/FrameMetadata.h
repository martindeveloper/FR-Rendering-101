#pragma once

#ifndef GRAPHICS_DIRECTX12_FRAMEMETADATA_H
#define GRAPHICS_DIRECTX12_FRAMEMETADATA_H

#include <d3d12.h>
#include <wrl.h>

namespace Graphics::DirectX12
{
    /**
     * @brief The FrameMetadata struct
     * @note Not a best way, as in real world scenario we would have multiple command lists per frame, and multiple frames in flight
     */
    struct FrameMetadata
    {
        UINT Frame;
        UINT CurrentBackBufferIndex;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandList;
    };
}

#endif // GRAPHICS_DIRECTX12_FRAMEMETADATA_H
