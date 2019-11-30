#pragma once
#include "pch.h"
#include "Scene.h"
#include "RenderPass.h"
#include "ECSRegistry.h"

namespace bdr
{
    namespace gltf
    {
        struct AttributeInfo
        {
            std::string name;
            std::string semanticName;
            bool required;
            MeshAttributes attrBit;
        };

        const AttributeInfo ATTR_INFO[]{
            { "POSITION", "SV_Position", true, MeshAttributes::POSITION },
            { "NORMAL", "NORMAL", true, MeshAttributes::NORMAL },
            { "TEXCOORD_0", "TEXCOORD", true, MeshAttributes::TEXCOORD },
            { "TANGENT", "TANGENT", false, MeshAttributes::TANGENT },
            { "JOINTS_0", "BLENDINDICES", false, MeshAttributes::BLENDINDICES },
            { "WEIGHTS_0", "BLENDWEIGHT", false, MeshAttributes::BLENDWEIGHT },
        };

        struct EntityMapping
        {
            int32_t gltfNodeIdx;
            uint32_t entity;
        };

        struct SceneData
        {
            DX::DeviceResources* pDeviceResources = nullptr;
            RenderPassManager* pRenderPassManager = nullptr;
            Scene* pScene = nullptr;
            const std::string fileFolder;
            const std::string fileName;
            std::vector<uint32_t> nodeMap;
            std::vector<uint32_t> meshMap;
            std::vector<uint32_t> textureMap;
            tinygltf::Model inputModel;

            SceneData(
                DX::DeviceResources* pDeviceResources,
                RenderPassManager* pRenderPassManager,
                Scene* scene,
                const std::string& folder,
                const std::string& file
            ) :
                pDeviceResources{ pDeviceResources },
                pRenderPassManager{ pRenderPassManager },
                pScene{ scene },
                fileFolder(folder),
                fileName(file),
                nodeMap{},
                meshMap{},
                textureMap{},
                inputModel{}
            { };
        };

        void loadModel(SceneData& sceneData);
    }
}
