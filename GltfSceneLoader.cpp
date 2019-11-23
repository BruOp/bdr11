#include "pch.h"

#include "GltfSceneLoader.h"
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
        case 4: // JOINTS_0
            if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                return DXGI_FORMAT_R8G8B8A8_UINT;
            }
            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                return DXGI_FORMAT_R16G16B16A16_UINT;
            }
        case 5: // WEIGHTS_0
            if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                return DXGI_FORMAT_R8G8B8A8_UNORM;
            }
            else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                return DXGI_FORMAT_R16G16B16A16_UNORM;
            }
        }
        throw std::runtime_error("Could not determine format");
    }

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

    void copyAccessorData(const tinygltf::Model& inputModel, const tinygltf::Accessor& accessor, void* dst)
    {
        const tinygltf::BufferView& bufferView = inputModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];

        memcpy(dst, &buffer.data.at(accessor.byteOffset + bufferView.byteOffset), accessor.count * getByteStride(accessor));
    }

    template<class T>
    void copyAccessorDataToVector(const tinygltf::Model& inputModel, const tinygltf::Accessor& accessor, std::vector<T>& dst, uint32_t numElements = 1)
    {
        // Num elements is useful for flattening Vec3 and Vec4 arrays
        const tinygltf::BufferView& bufferView = inputModel.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];

        dst.resize(accessor.count * numElements);
        memcpy(dst.data(), &buffer.data.at(accessor.byteOffset + bufferView.byteOffset), accessor.count * getByteStride(accessor));
    }

    void processLocalTransformAttributes(const tinygltf::Node& inputNode, Node& outputNode)
    {
        if (inputNode.scale.size() == 3) {
            outputNode.scale = Vector3{
                static_cast<float>(inputNode.scale[0]),
                static_cast<float>(inputNode.scale[1]),
                static_cast<float>(inputNode.scale[2]),
            };
            outputNode.transformMask |= TransformType::Scale;
        }

        if (inputNode.rotation.size() == 4) {
            outputNode.rotation = Quaternion{
                static_cast<float>(inputNode.rotation[0]),
                static_cast<float>(inputNode.rotation[1]),
                static_cast<float>(inputNode.rotation[2]),
                static_cast<float>(inputNode.rotation[3]),
            };
            outputNode.transformMask |= TransformType::Rotation;
        }

        if (inputNode.translation.size() == 3) {
            outputNode.translation = Vector3{
                static_cast<float>(inputNode.translation[0]),
                static_cast<float>(inputNode.translation[1]),
                static_cast<float>(inputNode.translation[2]),
            };
            outputNode.transformMask |= TransformType::Translation;
        }
    }

    void GltfSceneLoader::loadGLTFModel(Scene& scene, const std::string& gltfFolder, const std::string& gltfFileName)
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
        // This idx map just maps the input node indices to our nodeIdx
        // It will be used to create our skins
        std::vector<int32_t> idxMap(inputModel->nodes.size());
        int32_t nodeIdx = 0;
        for (const int inputNodeIdx : inputScene.nodes) {
            nodeIdx = processNode(scene, idxMap, inputNodeIdx, nodeIdx, -1);
        }

        for (const tinygltf::Skin& inputSkin : inputModel->skins) {
            scene.skins.push_back(processSkin(idxMap, inputSkin));
        }

        for (const tinygltf::Animation& inputAnimation : inputModel->animations) {
            scene.animations.push_back(processAnimation(idxMap, inputAnimation));
        }
    }

    Skin GltfSceneLoader::processSkin(std::vector<int32_t>& idxMap, const tinygltf::Skin& inputSkin)
    {
        Skin skin{
            std::vector<int32_t>(inputSkin.joints.size()),
            std::vector<Matrix>(inputSkin.joints.size()),
        };

        for (size_t i = 0; i < skin.jointIndices.size(); ++i) {
            skin.jointIndices[i] = idxMap[inputSkin.joints[i]];
        }

        const tinygltf::Accessor& accessor = inputModel->accessors[inputSkin.inverseBindMatrices];
        const tinygltf::BufferView& bufferView = inputModel->bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = inputModel->buffers[bufferView.buffer];
        memcpy(skin.inverseBindMatrices.data(), &buffer.data.at(accessor.byteOffset + bufferView.byteOffset), getByteSize(accessor) * accessor.count);
        return skin;
    }

    Animation GltfSceneLoader::processAnimation(const std::vector<int32_t>& idxMap, const tinygltf::Animation& animation)
    {
        Animation output{};

        output.channels.reserve(animation.channels.size());
        for (size_t i = 0; i < animation.channels.size(); ++i) {
            const tinygltf::AnimationChannel& inputChannel = animation.channels[i];
            const tinygltf::AnimationSampler& inputSampler = animation.samplers[inputChannel.sampler];

            uint32_t numElements = 3;
            TransformType channelType;
            if (inputChannel.target_path.compare("scale") == 0) {
                channelType = TransformType::Scale;
            }
            else if (inputChannel.target_path.compare("rotation") == 0) {
                channelType = TransformType::Rotation;
                numElements = 4;
            }
            else if (inputChannel.target_path.compare("translation") == 0) {
                channelType = TransformType::Translation;
            }
            else {
                Utility::Print("Don't support weights yet!");
                continue;
            }

            Animation::InterpolationType interpolationType = Animation::InterpolationType::CubicSpline;
            if (inputSampler.interpolation.compare("LINEAR")) {
                interpolationType = Animation::InterpolationType::Linear;
            }
            else if (inputSampler.interpolation.compare("STEP")) {
                interpolationType = Animation::InterpolationType::Step;
            }

            const tinygltf::Accessor& inputAccessor = inputModel->accessors[inputSampler.input];

            if (inputAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                throw std::runtime_error("Don't support non float samplers just yet");
            }

            const tinygltf::Accessor& outputAccessor = inputModel->accessors[inputSampler.input];

            if (outputAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                throw std::runtime_error("Don't support non float samplers just yet");
            }

            Animation::Channel animationChannel{
                idxMap[inputChannel.target_node],
                inputAccessor.maxValues[0],
                channelType,
                interpolationType,
            };

            copyAccessorDataToVector<float>(*inputModel, inputAccessor, animationChannel.input);
            std::vector<float> data{};

            animationChannel.output.reserve(outputAccessor.count);
            if (animationChannel.targetType == TransformType::Rotation) {

                copyAccessorDataToVector<float>(*inputModel, inputAccessor, data, 4);

                for (size_t j = 0; j < outputAccessor.count; j++) {
                    animationChannel.output.emplace_back(
                        data[j * 4 + 0],
                        data[j * 4 + 1],
                        data[j * 4 + 2],
                        data[j * 4 + 3]
                    );
                }
            }
            else {
                copyAccessorDataToVector<float>(*inputModel, inputAccessor, data, 3);
                const tinygltf::BufferView& bufferView = inputModel->bufferViews[inputAccessor.bufferView];
                const tinygltf::Buffer& buffer = inputModel->buffers[bufferView.buffer];
                for (size_t j = 0; j < outputAccessor.count; j++) {
                    animationChannel.output.emplace_back(
                        data[j * 3 + 0],
                        data[j * 3 + 1],
                        data[j * 3 + 2],
                        0.0f
                    );
                }
            }

            output.channels.push_back(animationChannel);
        }
        return output;
    }

    /*
    Process the local transform and the parent relationship of our node, returning the next nodeIdx to index into our scene
    */
    int32_t GltfSceneLoader::processNode(Scene& scene, std::vector<int32_t>& idxMap, int32_t inputNodeIdx, int32_t nodeIdx, int32_t parentIdx) const
    {
        ASSERT(nodeIdx < int32_t(scene.nodeList.size()));
        ASSERT(parentIdx < int32_t(scene.nodeList.size()));
        const tinygltf::Node& node = inputModel->nodes[inputNodeIdx];

        // Update our mapping
        idxMap[inputNodeIdx] = nodeIdx;
        // Update our node
        Node& outputNode = scene.nodeList.nodes[nodeIdx];
        outputNode.parent = parentIdx;
        processLocalTransformAttributes(node, outputNode);
        scene.nodeList.localTransforms[nodeIdx] = calcLocalTransform(outputNode);
        scene.nodeList.globalTransforms[nodeIdx] = processGlobalTransform(scene.nodeList, nodeIdx);

        if (node.mesh != -1) {
            const tinygltf::Mesh& inputMesh = inputModel->meshes[node.mesh];
            for (const tinygltf::Primitive& primitive : inputMesh.primitives) {
                RenderObject renderObject{
                    nodeIdx,
                    node.skin,
                    scene.nodeList.globalTransforms[nodeIdx],
                    processPrimitive(scene, primitive),
                };

                scene.renderObjects.push_back(renderObject);
            }
        }


        // If there are children, add the reference to the current node
        if (node.children.size() > 0) {
            int32_t childIdx = nodeIdx + 1;
            for (const int inputChildIdx : node.children) {
                childIdx = processNode(scene, idxMap, inputChildIdx, childIdx, nodeIdx);
            }
            return childIdx;
        }
        else {
            return ++nodeIdx;
        }
    }


    Mesh GltfSceneLoader::processPrimitive(Scene& scene, const tinygltf::Primitive& inputPrimitive) const
    {
        Mesh mesh{};
        // Index buffer
        const tinygltf::Accessor& indexAccessor = inputModel->accessors[inputPrimitive.indices];
        mesh.indexCount = uint32_t(indexAccessor.count);
        switch (indexAccessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        {
            mesh.indexFormat = DXGI_FORMAT_R16_UINT;
            // Need to recast our indices
            std::vector<uint16_t> indices(indexAccessor.count);
            // Copy the data from the buffer, casting each element
            const tinygltf::BufferView& bufferView = inputModel->bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& buffer = inputModel->buffers[bufferView.buffer];
            for (size_t i = 0; i < indexAccessor.count; ++i) {
                indices[i] = uint16_t(uint8_t(buffer.data.at(indexAccessor.byteOffset + bufferView.byteOffset + i)));
            }
            // Create our buffer
            D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(sizeof(uint16_t) * indices.size(), D3D11_BIND_INDEX_BUFFER);
            D3D11_SUBRESOURCE_DATA initData;
            initData.pSysMem = indices.data();
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;

            // Create the buffer with the device.
            DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateBuffer(&bufferDesc, &initData, &mesh.indexBuffer));
        }
        break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            mesh.indexFormat = DXGI_FORMAT_R16_UINT;
            createBuffer(&mesh.indexBuffer, indexAccessor, D3D11_BIND_INDEX_BUFFER);
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            mesh.indexFormat = DXGI_FORMAT_R32_UINT;
            createBuffer(&mesh.indexBuffer, indexAccessor, D3D11_BIND_INDEX_BUFFER);
            break;
        default:
            throw std::runtime_error("Cannot support indices of this type!");
        }


        D3D11_INPUT_ELEMENT_DESC inputLayouts[_countof(ATTR_INFO)];
        uint32_t vertBufferIdx = 0;
        for (size_t i = 0; i < _countof(ATTR_INFO); i++) {
            const GltfAttributeInfo& attrInfo = ATTR_INFO[i];
            const std::string& attrName = attrInfo.name;

            if (inputPrimitive.attributes.count(attrName) == 0) {
                if (attrInfo.required) {
                    throw std::runtime_error("Cannot handle meshes without " + attrName);
                }
                else {
                    continue;
                }
            }

            int accessorIndex = inputPrimitive.attributes.at(attrName);
            const tinygltf::Accessor& accessor = inputModel->accessors[accessorIndex];
            createBuffer(&mesh.vertexBuffers.vertexBuffers[vertBufferIdx], accessor, D3D11_BIND_VERTEX_BUFFER);
            mesh.vertexBuffers.strides[vertBufferIdx] = getByteSize(accessor);
            inputLayouts[vertBufferIdx] = { attrInfo.semanticName.c_str(), 0, getFormat(i, accessor.componentType), vertBufferIdx, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
            ++vertBufferIdx;
        }
        mesh.vertexBuffers.numPresentAttributes = vertBufferIdx;

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

    void GltfSceneLoader::createBuffer(ID3D11Buffer** dxBuffer, const tinygltf::Accessor& accessor, const uint32_t usageFlag) const
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