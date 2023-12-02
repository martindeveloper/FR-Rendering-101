#pragma once

#ifndef VERTEX_H
#define VERTEX_H

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

#endif // VERTEX_H
