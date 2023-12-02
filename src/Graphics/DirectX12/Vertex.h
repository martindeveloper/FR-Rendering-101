#pragma once

#ifndef GRAPHICS_DIRECTX12_VERTEX_H
#define GRAPHICS_DIRECTX12_VERTEX_H

#include <DirectXMath.h>

namespace Graphics::DirectX12
{
    typedef DirectX::XMFLOAT3 Float3;
    typedef DirectX::XMFLOAT4 Float4;

    struct Vertex
    {
        Float3 Position;
        Float4 Color;
    };
}

#endif // GRAPHICS_DIRECTX12_VERTEX_H
