#pragma once

#ifndef GRAPHICS_DIRECTX12_TOOLS_H
#define GRAPHICS_DIRECTX12_TOOLS_H

#include <d3d12.h>
#include <d3dcompiler.h>

#include "../../Platform/Platform.h"
#include "../../Core/CoreObject.h"

namespace Graphics::DirectX12
{
    /**
     * @brief The ShaderByteCodeBlob class, alternative to ID3DBlob
     */
    class ShaderByteCodeBlob : public Core::CoreObject
    {
    private:
        size_t Size;
        uint8_t *Data;

    public:
        ShaderByteCodeBlob(size_t size) : Size(size), Data(new uint8_t[size]) {}

        ~ShaderByteCodeBlob()
        {
            delete[] Data;
        }

        void *GetBufferPointer() const
        {
            return Data;
        }

        size_t GetBufferSize() const
        {
            return Size;
        }
    };

    /**
     * @brief The DirectX12Tools class
     */
    class Tools : public Core::CoreObject
    {
    public:
        /**
         * @brief Load shader byte code from .dxil file
         * @param path Path to .dxil file
         * @param byteCode Pointer to pointer to ShaderByteCodeBlob
         */
        static void LoadShaderByteCode(const wchar_t *path, ShaderByteCodeBlob **byteCode)
        {
            // Check if file exists
            if (Platform::FileExist(path))
            {
                Platform::GetLogger()->Fatal("LoadShaderByteCode: File %s does not exist", path);
                Platform::TriggerCrash();
            }

            // Read .dxil file into buffer
            char *buffer = nullptr;
            size_t fileSize = 0;

            Platform::LoadFileIntoBuffer(path, &buffer, &fileSize);

            // Create blob
            *byteCode = new ShaderByteCodeBlob(fileSize);

            // Copy buffer to blob
            memcpy((*byteCode)->GetBufferPointer(), buffer, fileSize);

            // Free buffer
            delete buffer;
        };

        /**
         * @brief CheckShaderError
         * @param result HRESULT
         * @param blob Pointer to ID3DBlob
         * @param message Message to log
         * @param shouldCrash Should crash if failed
         */
        static void CheckShaderError(HRESULT result, ID3DBlob *blob, const char *message, bool shouldCrash)
        {
            if (FAILED(result))
            {
                Platform::GetLogger()->Fatal(message);

                if (blob != nullptr)
                {
                    Platform::GetLogger()->Fatal(reinterpret_cast<const char *>(blob->GetBufferPointer()));
                }

                Platform::TriggerBreakpoint();

                if (shouldCrash)
                {
                    Platform::TriggerCrash();
                }
            }
        };
    };
}

#endif // GRAPHICS_DIRECTX12_TOOLS_H
