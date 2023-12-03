#if defined(DX12_AGILITY_SDK_ENABLED) && !defined(DX12_AGILITY_SDK_EXPORTED)
#define DX12_AGILITY_SDK_EXPORTED

extern "C"
{
    __declspec(dllexport) extern const unsigned int D3D12SDKVersion = DX12_AGILITY_SDK_VERSION;
    __declspec(dllexport) extern const char *D3D12SDKPath = DX12_AGILITY_SDK_BUILD_DLL_PATH;
}

namespace Graphics::DirectX12
{
    /**
     * @brief The DX12AgilitySDK struct
     */
    struct AgilitySDK
    {
        const unsigned int Version = DX12_AGILITY_SDK_VERSION;
        const char *DllPath = DX12_AGILITY_SDK_BUILD_DLL_PATH;
    };
}
#endif
