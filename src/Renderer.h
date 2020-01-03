#pragma once
#include "pch.h"

#include "Animation.h"
#include "DeviceResources.h"
#include "Mesh.h"
#include "InputLayoutManager.h"
#include "Material.h"
#include "GPUBuffer.h"
#include "View.h"


namespace bdr
{
    GPUBuffer createJointBuffer(ID3D11Device* device, const Skin& skin);

    class Renderer
    {
    public:
        Renderer()
        {
            deviceResources = std::make_unique<DX::DeviceResources>();
        }

        ~Renderer()
        {
            reset();
        }

        UNCOPIABLE(Renderer);
        UNMOVABLE(Renderer);

        void reset()
        {
            for (size_t i = 0; i < meshes.size(); i++) {
                meshes[i].destroy();
            }
            for (GPUBuffer& jointBuffer : jointBuffers) {
                bdr::reset(jointBuffer);
            }
            inputLayoutManager.reset();
            materials.reset();
        }

        inline void setWindow(HWND window, int newWidth, int newHeight)
        {
            width = uint32_t(newWidth);
            height = uint32_t(newHeight);
            deviceResources->SetWindow(window, newWidth, newHeight);
        }

        inline RECT getOutputSize() const
        {
            return deviceResources->GetOutputSize();
        }

        bool hasWindowSizeChanged(int newWidth, int newHeight)
        {
            const bool hasSizeChanged = deviceResources->WindowSizeChanged(newWidth, newHeight);
            if (hasSizeChanged) {
                width = uint32_t(newWidth);
                height = uint32_t(newHeight);
            }
            return hasSizeChanged;
        }

        inline void createDeviceResources()
        {
            deviceResources->CreateDeviceResources();
            inputLayoutManager.init(deviceResources->GetD3DDevice());
            viewCB.init(deviceResources->GetD3DDevice(), false);
        }

        inline void createWindowSizeDependentResources()
        {
            deviceResources->CreateWindowSizeDependentResources();
        }

        inline uint32_t getNewMesh()
        {
            meshes.emplace_back();
            return meshes.size() - 1;
        };

        inline uint32_t getInputLayout(const InputLayoutDetail details[], uint8_t numAttributes)
        {
            return inputLayoutManager.getOrCreateInputLayout(details, numAttributes);
        };

        inline ID3D11Device1* getDevice() const
        {
            return deviceResources->GetD3DDevice();
        }

        inline ID3D11DeviceContext1* getContext() const
        {
            return deviceResources->GetD3DDeviceContext();
        }

        // Device resources.
        uint32_t width = 0;
        uint32_t height = 0;
        std::unique_ptr<DX::DeviceResources> deviceResources;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader = nullptr;

        InputLayoutManager inputLayoutManager;
        std::vector<Mesh> meshes;
        std::vector<GPUBuffer> jointBuffers;
        MaterialManager materials;
        ConstantBuffer<ViewConstants> viewCB;
    };
}
