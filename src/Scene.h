#pragma once
#include "pch.h"
#include <vector>
#include <array>
#include "DeviceResources.h"

namespace tinygltf
{
    struct Accessor;
    struct Primitive;
    class Model;
}

namespace bdr
{

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

    struct MaterialInfo
    {
        void const* shaderBytecode;
        size_t byteCodeLength;
    };

    enum class RenderFeature
    {

    };

    enum MeshAttributes : uint8_t
    {
        POSITION = 1,
        NORMAL = 2,
        TEXCOORD_0 = 4,
        TANGENT = 8,
    };

    struct Mesh
    {
        static constexpr size_t numSupportedAttributes = 4;
        std::array<ID3D11Buffer*, numSupportedAttributes> vertexBuffers;
        ID3D11Buffer* indexBuffer;
        DXGI_FORMAT indexFormat;
        uint32_t strides[numSupportedAttributes];
        uint32_t indexCount = 0;
        uint8_t supportedAttributes;

        void destroy()
        {
            for (auto buffer : vertexBuffers) {
                buffer->Release();
            }
            indexBuffer->Release();
        }
    };

    struct RenderObject
    {
        int32_t SceneNodeIdx;
        DirectX::SimpleMath::Matrix modelTransform;
        Mesh mesh;
    };

    class Scene
    {
    public:
        ~Scene()
        {
            for (auto& renderObject : renderObjects) {
                renderObject.mesh.destroy();
            }
        }

        NodeList nodeList;
        std::vector<RenderObject> renderObjects;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout = nullptr;
        // Need a list of Models
    };

    struct SceneLoader
    {
        DX::DeviceResources* m_deviceResources;
        MaterialInfo materialInfo;
        tinygltf::Model const* inputModel = nullptr;

        void loadGLTFModel(Scene& scene, const std::string& gltfFolder, const std::string& gltfFileName);

    private:
        int32_t processNode(Scene& scene, int32_t inputNodeIdx, int32_t nodeIdx, int32_t parentIdx) const;
        Mesh processPrimitive(Scene& scene, const tinygltf::Primitive& inputPrimitive) const;
        void createBuffer(ID3D11Buffer** dxBuffer, const tinygltf::Accessor& accessor, const uint32_t usageFlag) const;
    };

}
