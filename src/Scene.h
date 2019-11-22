#pragma once
#include "pch.h"
#include <vector>
#include <array>
#include "DeviceResources.h"
#include "NodeList.h"
#include "Animation.h"


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
    struct MaterialInfo
    {
        void const* shaderBytecode;
        size_t byteCodeLength;
    };

    enum MeshAttributes : uint8_t
    {
        POSITION = 1,
        NORMAL = 2,
        TEXCOORD = 4,
        TANGENT = 8,
        BLENDINDICES = 16,
        BLENDWEIGHT = 32,
    };

    struct GltfAttributeInfo
    {
        std::string name;
        std::string semanticName;
        bool required;
        MeshAttributes attrBit;
    };

    const GltfAttributeInfo ATTR_INFO[]{
        { "POSITION", "SV_Position", true, MeshAttributes::POSITION },
        { "NORMAL", "NORMAL", true, MeshAttributes::NORMAL },
        { "TEXCOORD_0", "TEXCOORD", true, MeshAttributes::TEXCOORD },
        { "TANGENT", "TANGENT", false, MeshAttributes::TANGENT },
        { "JOINTS_0", "BLENDINDICES", false, MeshAttributes::BLENDINDICES },
        { "WEIGHTS_0", "BLENDWEIGHT", false, MeshAttributes::BLENDWEIGHT },
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

    struct RenderObject
    {
        int32_t SceneNodeIdx;
        int32_t skinIdx = -1;
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
