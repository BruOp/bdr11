#pragma once
#include "pch.h"

#include "DeviceResources.h"
#include "Mesh.h"
#include "InputLayoutManager.h"
#include "Material.h"
#include "GPUBuffer.h"
#include "Texture.h"
#include "Game/View.h"
#include "ResourceManager.h"

namespace bdr
{
    GPUBuffer createJointBuffer(ID3D11Device* device, const Skin& skin);

    uint32_t createMesh(Renderer& renderer, const MeshCreationInfo& meshCreationInfo);
    uint32_t getOrCreateBasicMaterial(Renderer& renderer);

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
            viewCB.reset();
            inputLayoutManager.reset();
            materials.reset();
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
            viewCB.init(deviceResources->GetD3DDevice(), false);
        };

        inline void createWindowSizeDependentResources()
        {
            deviceResources->CreateWindowSizeDependentResources();
        };

        inline uint32_t createTextureFromFile(const std::string& filePath, const TextureCreationInfo& createInfo)
        {
            uint32_t idx = static_cast<uint32_t>(textures.create());
            textures[idx] = Texture::createFromFile(deviceResources->GetD3DDevice(), filePath, createInfo);
            return idx;
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
        std::unique_ptr<DX::DeviceResources> deviceResources;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader = nullptr;

        InputLayoutManager inputLayoutManager;
        ResourceManager<Mesh> meshes;
        ResourceManager<GPUBuffer> jointBuffers;
        ResourceManager<Texture> textures;
        MaterialManager materials;
        ConstantBuffer<ViewConstants> viewCB;
    };
}

// Callbacks for window resize:
void onWindowResize(bdr::Renderer& renderer, int width, int height);
void onWindowMove(bdr::Renderer& renderer);