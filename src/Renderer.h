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
        std::unique_ptr<DX::DeviceResources> deviceResources;
        InputLayoutManager inputLayoutManager;
        std::vector<Mesh> meshes;
    };
}
