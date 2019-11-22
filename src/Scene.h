#pragma once
#include "pch.h"
#include <vector>
#include <array>
#include "DeviceResources.h"

namespace tinygltf
{
    struct Accessor;
    struct Primitive;
    struct Skin;
    struct Animation;
    class Model;
}

namespace bdr
{

    enum TransformType : uint8_t
    {
        Rotation = 1,
        Translation = 2,
        Scale = 4,
        // Weights not supported
    };

    struct Node
    {
        DirectX::SimpleMath::Vector4 rotation;
        DirectX::SimpleMath::Vector3 scale;
        DirectX::SimpleMath::Vector3 translation;
        int32_t parent = -1;
        uint8_t transformMask = 0;
    };

    struct NodeList
    {
        std::vector<DirectX::SimpleMath::Matrix> localTransforms;
        std::vector<DirectX::SimpleMath::Matrix> globalTransforms;
        std::vector<Node> nodes;

        size_t size() const
        {
            ASSERT(localTransforms.size() == globalTransforms.size());
            ASSERT(localTransforms.size() == nodes.size());
            return localTransforms.size();
        };

        void resize(size_t newSize)
        {
            localTransforms.resize(newSize);
            globalTransforms.resize(newSize);
            nodes.resize(newSize);
        };
    };

    void updateNodes(NodeList& nodeList);

    struct MaterialInfo
    {
        void const* shaderBytecode;
        size_t byteCodeLength;
    };

    struct AttributeInfo
    {
        std::string name;
        std::string semanticName;
        bool required;
    };

    const AttributeInfo ATTR_INFO[]{
        { "POSITION", "SV_Position", true },
        { "NORMAL", "NORMAL", true },
        { "TEXCOORD_0", "TEXCOORD", true },
        { "TANGENT", "TANGENT", false },
        { "JOINTS_0", "BLENDINDICES", false },
        { "WEIGHTS_0", "BLENDWEIGHT", false },
    };

    enum MeshAttributes : uint8_t
    {
        POSITION = 1,
        NORMAL = 2,
        TEXCOORD_0 = 4,
        TANGENT = 8,
        BLENDINDICES = 16,
        BLENDWEIGHT = 32,
    };

    template<size_t attrCount>
    struct VertexBufferSet
    {
        static constexpr size_t numSupportedAttributes = attrCount;
        std::array<ID3D11Buffer*, attrCount> vertexBuffers;
        uint32_t strides[attrCount];
        uint8_t presentAttributesMask;
        uint8_t numPresentAttributes;
    };

    struct Mesh
    {
        VertexBufferSet<6u> vertexBuffers;
        ID3D11Buffer* indexBuffer;
        DXGI_FORMAT indexFormat;
        
        uint32_t indexCount = 0;


        void destroy()
        {
            for (auto buffer : vertexBuffers.vertexBuffers) {
                buffer != nullptr && buffer->Release();
            }
            indexBuffer->Release();
        }
    };

    enum AnimationInterpolationType : uint8_t
    {
        Linear = 1,
        Step = 2,
        CubicSpline = 4,
    };

    struct AnimationChannel {
        int32_t targetNodeIdx;
        float maxInput;
        TransformType targetType;
        AnimationInterpolationType interpolationType;
        std::vector<float> input;
        std::vector<DirectX::SimpleMath::Vector4> output;
    };

    struct Animation
    {
        std::vector<AnimationChannel> channels;
    };

    void updateAnimation(NodeList& nodeList, const Animation& animation, const float currentTime);

    struct RenderObject
    {
        int32_t SceneNodeIdx;
        int32_t skinIdx = -1;
        DirectX::SimpleMath::Matrix modelTransform;
        Mesh mesh;
    };

    struct Skin
    {
        std::vector<int32_t> jointIndices;
        std::vector<DirectX::SimpleMath::Matrix> inverseBindMatrices;
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
        std::vector<Skin> skins;
        std::vector<Animation> animations;
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
        int32_t processNode(Scene& scene, std::vector<int32_t>& idxMap, int32_t inputNodeIdx, int32_t nodeIdx, int32_t parentIdx) const;
        Skin processSkin(std::vector<int32_t>& idxMap, const tinygltf::Skin& inputSkin);
        Animation processAnimation(const std::vector<int32_t>& idxMap, const tinygltf::Animation& animation);
        Mesh processPrimitive(Scene& scene, const tinygltf::Primitive& inputPrimitive) const;
        void createBuffer(ID3D11Buffer** dxBuffer, const tinygltf::Accessor& accessor, const uint32_t usageFlag) const;
    };

}
