#pragma once
#include "pch.h"

#include "DeviceResources.h"
#include "Resources.h"
#include "GPUBuffer.h"
#include "InputLayoutManager.h"
#include "Texture.h"
#include "ResourceManager.h"
#include "PipelineState.h"


namespace bdr
{
    class Renderer
    {
    public:
        Renderer()
        {
            deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
        }

        ~Renderer()
        {
            reset();
        }

        UNCOPIABLE(Renderer);
        UNMOVABLE(Renderer);

        void reset()
        {
            inputLayoutManager.reset();
        }

        inline void setWindow(HWND window, int newWidth, int newHeight)
        {
            width = uint32_t(newWidth);
            height = uint32_t(newHeight);
            deviceResources->SetWindow(window, newWidth, newHeight);
        };

        inline RECT getOutputSize() const
        {
            return deviceResources->GetOutputSize();
        };

        bool hasWindowSizeChanged(int newWidth, int newHeight)
        {
            const bool hasSizeChanged = deviceResources->WindowSizeChanged(newWidth, newHeight);
            if (hasSizeChanged) {
                width = uint32_t(newWidth);
                height = uint32_t(newHeight);
            }
            return hasSizeChanged;
        };

        inline void createDeviceResources()
        {
            deviceResources->CreateDeviceResources();
            inputLayoutManager.init(deviceResources->GetD3DDevice());
        };

        inline void createWindowSizeDependentResources()
        {
            deviceResources->CreateWindowSizeDependentResources();
        };

        inline ID3D11Device1* getDevice() const
        {
            return deviceResources->GetD3DDevice();
        };

        inline ID3D11DeviceContext1* getContext() const
        {
            return deviceResources->GetD3DDeviceContext();
        };

        // Device resources.
        uint32_t width = 0;
        uint32_t height = 0;
        float totalTime = 0;
        float deltaTime = 0;
        std::unique_ptr<DX::DeviceResources> deviceResources;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader = nullptr;

        InputLayoutManager inputLayoutManager;
        ResourceManager<Mesh> meshes;
        ResourceManager<GPUBuffer> jointBuffers;
        ResourceManager<Texture> textures;
        ResourceManager<GPUBuffer> constantBuffers;
        SimpleMap32<PipelineStateDefinition> pipelineDefinitions;
        ResourceManager<PipelineState> pipelines;
        ResourceBindingHeap bindingHeap;
        std::vector<ResourceBinder> binders;
        std::vector<std::string> shaderCodeRegistry;

    };
}

// Callbacks for window resize:
void onWindowResize(bdr::Renderer& renderer, int width, int height);
void onWindowMove(bdr::Renderer& renderer);