#include "pch.h"

#include <iostream>

#include "Scene.h"
#include "DeviceResources.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

using namespace DirectX::SimpleMath;

namespace bdr
{
    bool loadImageDataCallback(
        tinygltf::Image* image,
        const int image_idx,
        std::string* err,
        std::string* warn,
        int req_width,
        int req_height,
        const unsigned char* bytes,
        int size,
        void* user_data)
    {
        return true;
    };

    DXGI_FORMAT getFormat(size_t attrInfoIdx, int32_t componentType)
    {
        switch (attrInfoIdx) {
        case 0: // Position
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case 1: // Normal
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case 2: // TexCoord_0
            if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                return DXGI_FORMAT_R32G32_FLOAT;
            }
            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                return DXGI_FORMAT_R8G8_UNORM;
            }
            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                return DXGI_FORMAT_R16G16_UNORM;
            }
        case 3: // Tangent
            return DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
        throw std::runtime_error("Could not determine format");
    }

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
        { "TANGENT", "TANGENT", true },
    };

    uint32_t getByteStride(const tinygltf::Accessor& accessor)
    {
        switch (accessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return 1u;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return 2u;
        case TINYGLTF_COMPONENT_TYPE_INT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        case TINYGLTF_COMPONENT_TYPE_FLOAT:
            return 4u;
        case TINYGLTF_COMPONENT_TYPE_DOUBLE:
            return 8u;
        default:
            throw std::runtime_error("Don't know what the stride of this component type is!");
        }
    }

    uint32_t getByteSize(const tinygltf::Accessor& accessor)
    {
        uint32_t byteSize = getByteStride(accessor);

        switch (accessor.type) {
        case TINYGLTF_TYPE_SCALAR:
            return byteSize;
        case TINYGLTF_TYPE_VEC2:
            return byteSize * 2;
        case TINYGLTF_TYPE_VEC3:
            return byteSize * 3;
        case TINYGLTF_TYPE_VEC4:
        case TINYGLTF_TYPE_MAT2:
            return byteSize * 4;
        case TINYGLTF_TYPE_MAT3:
            return byteSize * 9;
        case TINYGLTF_TYPE_MAT4:
            return byteSize * 16;
        }

        return 0;
    }

    Matrix processLocalTransform(const tinygltf::Node& node)
    {
        Matrix localTransform = Matrix::Identity;
        if (node.scale.size() == 3) {
            Matrix scale = Matrix::CreateScale(
                static_cast<float>(node.scale[0]),
                static_cast<float>(node.scale[1]),
                static_cast<float>(node.scale[2])
            );
            localTransform *= scale;
        }

        if (node.rotation.size() == 4) {
            Matrix rotation = Matrix::Transform(localTransform,
                Quaternion{
                    static_cast<float>(node.rotation[0]),
                    static_cast<float>(node.rotation[1]),
                    static_cast<float>(node.rotation[2]),
                    static_cast<float>(node.rotation[3]),
                }
            );
        }

        if (node.translation.size() == 3) {
            localTransform.Translation(
                Vector3{
                    static_cast<float>(node.translation[0]),
                    static_cast<float>(node.translation[1]),
                    static_cast<float>(node.translation[2]),
                }
            );
        }
        return localTransform;
    };

    void updateNodes(NodeList& nodeList)
    {
        for (size_t i = 0; i < nodeList.size(); i++) {
            const int32_t parent = nodeList.parents[i];
            if (parent == -1) {
                nodeList.globalTransforms[i] = nodeList.localTransforms[i];
            }
            else {
                ASSERT(parent < nodeList.globalTransforms.size());
                ASSERT(parent < i);
                nodeList.globalTransforms[i] = nodeList.localTransforms[i] * nodeList.globalTransforms[parent];
            }
        }
    };

    void SceneLoader::loadGLTFModel(Scene& scene, const std::string& gltfFolder, const std::string& gltfFileName)
    {
        tinygltf::TinyGLTF loader;
        loader.SetImageLoader(loadImageDataCallback, nullptr);
        std::string err, warn;
        tinygltf::Model gltfModel;
        bool res = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, gltfFolder + gltfFileName);
        inputModel = &gltfModel;

        if (!warn.empty()) {
            std::cout << warn << std::endl;
        }

        if (!err.empty()) {
            std::cout << err << std::endl;
        }

        if (!res) {
            throw std::runtime_error("Failed to load GLTF Model");
        }

        if (inputModel->scenes.size() != 1) {
            throw std::runtime_error("Cannot handle a GLTF with more than one scene at the moment!");
        }

        const tinygltf::Scene& inputScene = inputModel->scenes[inputModel->defaultScene];
        scene.nodeList.resize(inputModel->nodes.size());
        int32_t nodeIdx = 0;
        for (const int inputNodeIdx : inputScene.nodes) {
            nodeIdx = processNode(scene, inputNodeIdx, nodeIdx, -1);
        }

        updateNodes(scene.nodeList);
    }

    /*
    Process the local transform and the parent relationship of our node, returning the next nodeIdx to index into our scene
    */
    int32_t SceneLoader::processNode(Scene& scene, int32_t inputNodeIdx, int32_t nodeIdx, int32_t parentIdx) const
    {
        ASSERT(nodeIdx < int32_t(scene.nodeList.size()));
        ASSERT(parentIdx < int32_t(scene.nodeList.size()));
        const tinygltf::Node& node = inputModel->nodes[inputNodeIdx];

        scene.nodeList.localTransforms[nodeIdx] = processLocalTransform(node);
        scene.nodeList.parents[nodeIdx] = parentIdx;

        if (node.mesh != -1) {
            const tinygltf::Mesh& inputMesh = inputModel->meshes[node.mesh];
            for (const tinygltf::Primitive& primitive : inputMesh.primitives) {
                Mesh mesh = processPrimitive(scene, primitive);
                mesh.nodeIdx = nodeIdx;
                scene.meshes.push_back(mesh);
            }
        }

        // If there are children, add the reference to the current node
        if (node.children.size() > 0) {
            int32_t childIdx = nodeIdx + 1;
            for (const int inputChildIdx : node.children) {
                childIdx = processNode(scene, inputChildIdx, childIdx, nodeIdx);
            }
            return childIdx;
        }
        else {
            return ++nodeIdx;
        }
    }


    Mesh SceneLoader::processPrimitive(Scene& scene, const tinygltf::Primitive& inputPrimitive) const
    {
        Mesh mesh{};
        // Index buffer
        const tinygltf::Accessor& indexAccessor = inputModel->accessors[inputPrimitive.indices];
        mesh.indexCount = uint32_t(indexAccessor.count);
        switch (indexAccessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            mesh.indexFormat = DXGI_FORMAT_R16_UINT;
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            mesh.indexFormat = DXGI_FORMAT_R32_UINT;
            break;
        default:
            throw std::runtime_error("Cannot support indices of this type!");
        }

        createBuffer(&mesh.indexBuffer, indexAccessor, D3D11_BIND_INDEX_BUFFER);
        
        D3D11_INPUT_ELEMENT_DESC inputLayouts[_countof(ATTR_INFO)];
        uint32_t vertBufferIdx = 0;
        for (size_t i = 0; i < _countof(ATTR_INFO); i++) {
            const AttributeInfo& attrInfo = ATTR_INFO[i];
            const std::string& attrName = attrInfo.name;
            
            if (inputPrimitive.attributes.count(attrName) == 0 && attrInfo.required) {
                throw std::runtime_error("Cannot handle meshes without " + attrName);
            }
            
            int accessorIndex = inputPrimitive.attributes.at(attrName);
            const tinygltf::Accessor& accessor = inputModel->accessors[accessorIndex];
            createBuffer(&mesh.vertexBuffers[vertBufferIdx], accessor, D3D11_BIND_VERTEX_BUFFER);
            mesh.strides[vertBufferIdx] = getByteSize(accessor);
            inputLayouts[vertBufferIdx] = { attrInfo.semanticName.c_str(), 0, getFormat(i, accessor.componentType), vertBufferIdx, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
            ++vertBufferIdx;
        }

        if (scene.pInputLayout == nullptr) {
            DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateInputLayout(
                inputLayouts,
                vertBufferIdx,
                materialInfo.shaderBytecode,
                materialInfo.byteCodeLength,
                scene.pInputLayout.ReleaseAndGetAddressOf()
            ));
        }
        return mesh;
    }

    void SceneLoader::createBuffer(ID3D11Buffer** dxBuffer, const tinygltf::Accessor& accessor, const uint32_t usageFlag) const
    {
        const tinygltf::BufferView& bufferView = inputModel->bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = inputModel->buffers[bufferView.buffer];
        D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(getByteSize(accessor) * accessor.count, usageFlag);

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &buffer.data.at(accessor.byteOffset + bufferView.byteOffset);
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;
        
        // Create the buffer with the device.
        DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &initData, dxBuffer));
    }
    
}
