#pragma once
#include "pch.h"
#include <unordered_map>

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

        struct SceneNode
        {
            std::string name;
            uint32_t index = UINT32_MAX;
            int32_t parentId = -1;
            int32_t meshId = -1;
            int32_t primitiveId = -1;
            int32_t skinId = -1;
            bool isJoint = false;
        };

        struct SceneData
        {
            Scene* pScene = nullptr;
            Renderer* pRenderer = nullptr;
            const std::string fileFolder;
            const std::string fileName;
            std::vector<SceneNode> nodes;
            std::vector<SceneNode> traversedNodes;
            std::vector<uint32_t> nodeToEntityMap;
            std::unordered_map<uint64_t, uint32_t> meshMap;
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
                nodes{},
                traversedNodes{},
                nodeToEntityMap{},
                meshMap{},
                textureMap{},
                inputModel{}
            { };
        };

        void loadModel(SceneData& sceneData);
    }
}
