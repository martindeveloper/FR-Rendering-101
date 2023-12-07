#pragma once

#ifndef ENTITIES_TRIANGLE_ENTITY_H
#define ENTITIES_TRIANGLE_ENTITY_H

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "../IEntity.h"
#include "../../Platform/Platform.h"
#include "../../Graphics/DirectX12/Vertex.h"
#include "../../Graphics/DirectX12/Tools.h"

namespace Scene::Entities
{
    class TriangleEntity : public IEntity
    {
    private:
        Diagnostics::Logger *Logger = nullptr;

        uint64_t Frame = 0;

        struct ConstantBufferPayload
        {
            // "Time"
            float Time = 0.0f;
        };

        const Graphics::DirectX12::Vertex Vertices[3] = {
            {Graphics::DirectX12::Float3(0.0f, 0.25f, 0.0f), Graphics::DirectX12::Float4(1.0f, 0.0f, 0.0f, 1.0f)},
            {Graphics::DirectX12::Float3(0.25f, -0.25f, 0.0f), Graphics::DirectX12::Float4(0.0f, 1.0f, 0.0f, 1.0f)},
            {Graphics::DirectX12::Float3(-0.25f, -0.25f, 0.0f), Graphics::DirectX12::Float4(0.0f, 0.0f, 1.0f, 1.0f)}};

        Graphics::DirectX12::ShaderByteCodeBlob *VertexShaderBlob = nullptr;
        Graphics::DirectX12::ShaderByteCodeBlob *PixelShaderBlob = nullptr;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignature = nullptr;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> PipelineState = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuffer = nullptr;
        Microsoft::WRL::ComPtr<ID3D12Resource> ConstantBuffers[2] = {nullptr, nullptr};

    public:
        TriangleEntity();
        virtual ~TriangleEntity();

        void OnResourceCreate(Graphics::DirectX12::ResourcesInitializationMetadata *resourceMetadata);
        void OnRender(Graphics::DirectX12::FrameMetadata *frameMetadata);
        void OnUpdate(uint64_t frame);

    private:
        void CreateRootSignature(Microsoft::WRL::ComPtr<ID3D12Device> device);
        void CreatePipelineState(Microsoft::WRL::ComPtr<ID3D12Device> device);
        void CreateVertexBuffer(Microsoft::WRL::ComPtr<ID3D12Device> device);
        void CreateConstantBuffers(Microsoft::WRL::ComPtr<ID3D12Device> device, UINT backBufferCount);
        void CreateShaders();
    };
}

#endif // ENTITIES_TRIANGLE_ENTITY_H
