#pragma once
#include "pch.h"
#include <vector>

namespace tinygltf
{
    class Accessor;
    class Primitive;
    class Model;
}

namespace bdr
{
    class DX::DeviceResources;

    struct NodeList
    {
        std::vector<DirectX::SimpleMath::Matrix> localTransforms;
        std::vector<DirectX::SimpleMath::Matrix> globalTransforms;
        std::vector<int32_t> parents;
        
        size_t size() const
        {
            ASSERT(localTransforms.size() == globalTransforms.size());
            ASSERT(localTransforms.size() == parents.size());
            return localTransforms.size();
        };

        void resize(size_t newSize)
        {
            localTransforms.resize(newSize);
            globalTransforms.resize(newSize);
            parents.resize(newSize);
        };
    };

    void updateNodes(NodeList& nodeList);

    struct Mesh
    {
        std::vector<ID3D11Buffer*> vertexBuffers;
        ID3D11Buffer* indexBuffer;

        void destroy()
        {
            for (auto buffer : vertexBuffers) {
                buffer->Release();
            }
            indexBuffer->Release();
        }
    };

    struct Scene
    {
        NodeList nodeList;
        // Need a list of Models
    };

    struct SceneLoader
    {
        DX::DeviceResources* m_deviceResources;
        tinygltf::Model inputModel;
        
        void loadGLTFModel(Scene& scene, const std::string& gltfFolder, const std::string& gltfFileName);

    private:
        int32_t processNode(Scene& scene, int32_t inputNodeIdx, int32_t nodeIdx, int32_t parentIdx) const;
        Mesh processPrimitive(const tinygltf::Primitive& inputPrimitive) const;
        void createBuffer(ID3D11Buffer** buffer, const tinygltf::Accessor& accessor) const;
    };

}
