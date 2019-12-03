#pragma once
#include "pch.h"

#include "DeviceResources.h"
#include "Mesh.h"
#include "InputLayoutManager.h"

namespace bdr
{
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


        uint32_t addMesh(Mesh mesh, const InputLayoutDetail details[])
        {
            mesh.inputLayoutHandle = inputLayoutManager.getOrCreateInputLayout(details, mesh.numPresentAttr);
            meshes.push_back(std::move(mesh));
            return meshes.size() - 1;
        };

        uint32_t addMesh(Mesh&& mesh, const InputLayoutDetail details[])
        {
            mesh.inputLayoutHandle = inputLayoutManager.getOrCreateInputLayout(details, mesh.numPresentAttr);
            meshes.push_back(std::forward<Mesh>(mesh));
            return meshes.size() - 1;
        };

        ID3D11Device1* getDevice() const
        {
            return deviceResources->GetD3DDevice();
        }

        // Device resources.
        uint32_t width = 0;
        uint32_t height = 0;
        std::unique_ptr<DX::DeviceResources> deviceResources;
        InputLayoutManager inputLayoutManager;
        std::vector<Mesh> meshes;
    };
}
