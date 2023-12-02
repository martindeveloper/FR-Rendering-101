#pragma once

#ifndef DIRECTX12_TOOLS_H
#define DIRECTX12_TOOLS_H

#include <d3d12.h>
#include <d3dcompiler.h>

#include "../../Platform/Platform.h"

namespace Graphics::DirectX12
{
    /**
     * @brief The DirectX12Tools class
     */
    class Tools
    {
    public:
        /**
         * @brief Load shader byte code from .dxil file
         * @param path Path to .dxil file
         * @param byteCode Pointer to pointer to ID3DBlob
         */
        static void LoadShaderByteCode(const wchar_t *path, ID3DBlob **byteCode)
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
            HRESULT result = D3DCreateBlob(fileSize, byteCode);

            if (FAILED(result))
            {
                Platform::GetLogger()->Fatal("Failed to create blob for file %s", path);
                Platform::TriggerCrash();
            }

            // Copy buffer to blob
            memcpy((*byteCode)->GetBufferPointer(), buffer, fileSize);

            // Free buffer
            free(buffer);
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

#endif // DIRECTX12_TOOLS_H
