#pragma once
#include "pch.h"
#include "Scene.h"
#include "RenderPass.h"

namespace bdr
{
    struct GltfSceneLoader
    {
        GltfSceneLoader(DX::DeviceResources* deviceResources, BasicRenderPass* renderPass) :
            m_deviceResources{deviceResources },
            renderPass{ renderPass }
        {};

        DX::DeviceResources* m_deviceResources = nullptr;
        BasicRenderPass* renderPass = nullptr;
        tinygltf::Model const* inputModel = nullptr;

        void loadGLTFModel(Scene& scene, const std::string& gltfFolder, const std::string& gltfFileName);

    private:
        int32_t processNode(Scene& scene, std::vector<int32_t>& idxMap, int32_t inputNodeIdx, int32_t nodeIdx, int32_t parentIdx) const;
        Skin processSkin(std::vector<int32_t>& idxMap, const tinygltf::Skin& inputSkin);
        Animation processAnimation(const std::vector<int32_t>& idxMap, const tinygltf::Animation& animation);
        Mesh processPrimitive(const tinygltf::Primitive& inputPrimitive) const;
        std::array<D3D11_INPUT_ELEMENT_DESC, _countof(ATTR_INFO)> getInputElementDescs(const Mesh& mesh, const tinygltf::Primitive& inputPrimitive) const;
        void createBuffer(ID3D11Buffer** dxBuffer, const tinygltf::Accessor& accessor, const uint32_t usageFlag) const;
    };
}
