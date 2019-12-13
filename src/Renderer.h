#pragma once
#include "pch.h"

#include "Animation.h"
#include "DeviceResources.h"
#include "Mesh.h"
#include "InputLayoutManager.h"
#include "Material.h"

namespace bdr
{
    struct View
    {
        DirectX::SimpleMath::Matrix viewTransform;
        DirectX::SimpleMath::Matrix projection;
    };

    struct JointBuffer
    {
        ID3D11Buffer* buffer = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;

        void reset()
        {
            if (srv != nullptr) {
                srv->Release();
                srv = nullptr;
            }

            if (buffer != nullptr) {
                buffer->Release();
                buffer = nullptr;
            }
        }
    };

    JointBuffer createJointBuffer(ID3D11Device* device, const Skin& skin);

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
            for (JointBuffer& jointBuffer: jointBuffers) {
                jointBuffer.reset();
            }
            inputLayoutManager.reset();
            materials.reset();
        }

        void setWindow(HWND window, int newWidth, int newHeight)
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
        }

        inline void createWindowSizeDependentResources()
        {
            deviceResources->CreateWindowSizeDependentResources();
        }

        uint32_t getNewMesh()
        {
            meshes.emplace_back();
            return meshes.size() - 1;
        };

        uint32_t getInputLayout(const InputLayoutDetail details[], uint8_t numAttributes)
        {
            return inputLayoutManager.getOrCreateInputLayout(details, numAttributes);
        };

        ID3D11Device1* getDevice() const
        {
            return deviceResources->GetD3DDevice();
        }

        // Device resources.
        uint32_t width = 0;
        uint32_t height = 0;
        std::unique_ptr<DX::DeviceResources> deviceResources;
        Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader = nullptr;

        InputLayoutManager inputLayoutManager;

        std::vector<Mesh> meshes;
        std::vector<JointBuffer> jointBuffers;
        MaterialManager materials;
    };
}
