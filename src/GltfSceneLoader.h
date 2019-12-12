#pragma once
#include "pch.h"

#include "Mesh.h"
#include "Scene.h"
#include "Renderer.h"

namespace tinygltf
{
    class Model;
}


namespace bdr
{
    namespace gltf
    {
        struct AttributeInfo
        {
            enum Flags : uint8_t
            {
                REQUIRED = 1,
                USED_FOR_SKINNING = 2,
                PRESKIN_ONLY = 4,
            };

            std::string name;
            std::string semanticName;
            MeshAttributes attrBit;
            uint8_t flags;
        };

        const AttributeInfo ATTR_INFO[]{
            {
                "POSITION",
                "SV_Position",
                MeshAttributes::POSITION,
                AttributeInfo::REQUIRED | AttributeInfo::USED_FOR_SKINNING
            },
            {
                "NORMAL",
                "NORMAL",
                MeshAttributes::NORMAL,
                AttributeInfo::REQUIRED | AttributeInfo::USED_FOR_SKINNING
            },
            {
                "TEXCOORD_0",
                "TEXCOORD",
                MeshAttributes::TEXCOORD,
                AttributeInfo::REQUIRED
            },
            {
                "TANGENT",
                "TANGENT",
                MeshAttributes::TANGENT,
                AttributeInfo::USED_FOR_SKINNING
            },
            {
                "JOINTS_0",
                "BLENDINDICES",
                MeshAttributes::BLENDINDICES,
                AttributeInfo::USED_FOR_SKINNING | AttributeInfo::PRESKIN_ONLY
            },
            {
                "WEIGHTS_0",
                "BLENDWEIGHT",
                MeshAttributes::BLENDWEIGHT,
                AttributeInfo::USED_FOR_SKINNING | AttributeInfo::PRESKIN_ONLY
            },
        };

        struct EntityMapping
        {
            int32_t gltfNodeIdx;
            uint32_t entity;
        };

        struct SceneData
        {
            Scene* pScene = nullptr;
            Renderer* pRenderer = nullptr;
            const std::string fileFolder;
            const std::string fileName;
            std::vector<uint32_t> nodeMap;
            std::vector<uint32_t> meshMap;
            std::vector<uint32_t> textureMap;
            tinygltf::Model* inputModel;

            SceneData(
                Scene* scene,
                Renderer* pRenderer,
                const std::string& folder,
                const std::string& file
            ) :
                pScene{ scene },
                pRenderer{ pRenderer },
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
