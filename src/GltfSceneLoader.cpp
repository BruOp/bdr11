#include "pch.h"

#include "GltfSceneLoader.h"
#include "GPUBuffer.h"
#include <iostream>

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"

using namespace DirectX::SimpleMath;
namespace bdr
{
    namespace gltf
    {
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
                "JOINTS_0",
                "BLENDINDICES",
                MeshAttributes::BLENDINDICES,
                AttributeInfo::REQUIRED | AttributeInfo::USED_FOR_SKINNING | AttributeInfo::PRESKIN_ONLY
            },
            {
                "WEIGHTS_0",
                "BLENDWEIGHT",
                MeshAttributes::BLENDWEIGHT,
                AttributeInfo::REQUIRED | AttributeInfo::USED_FOR_SKINNING | AttributeInfo::PRESKIN_ONLY
            },
            {
                "TANGENT",
                "TANGENT",
                MeshAttributes::TANGENT,
                AttributeInfo::USED_FOR_SKINNING
            },
        };

        AttributeInfo genericAttrInfo[] = {
            ATTR_INFO[0], ATTR_INFO[1], ATTR_INFO[2], ATTR_INFO[5],
        };
        AttributeInfo preskinAttrInfo[] = {
            ATTR_INFO[0], ATTR_INFO[1], ATTR_INFO[3], ATTR_INFO[4], ATTR_INFO[5],
        };

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

        BufferFormat getFormat(const AttributeInfo& attrInfo, int32_t componentType)
        {
            switch (attrInfo.attrBit) {
            case MeshAttributes::POSITION:
            case MeshAttributes::NORMAL:
                return BufferFormat::FLOAT_3;

            case MeshAttributes::TEXCOORD:
                if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    return BufferFormat::FLOAT_2;
                }
                else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    return BufferFormat::UNORM8_2;
                }
                else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    return BufferFormat::UNORM16_2;
                }

            case MeshAttributes::TANGENT:
                return BufferFormat::FLOAT_4;

            case MeshAttributes::BLENDINDICES:
                if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    return BufferFormat::UINT8_4;
                }
                else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    return BufferFormat::UINT16_4;
                }

            case MeshAttributes::BLENDWEIGHT: // WEIGHTS_0
                if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    return BufferFormat::FLOAT_4;
                }
                else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    return BufferFormat::UNORM8_4;
                }
                else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    return BufferFormat::UNORM16_4;
                }
            }
            throw std::runtime_error("Could not determine format");
        }

        DXGI_FORMAT getDXFormat(const AttributeInfo& attrInfo, int32_t componentType)
        {
            return mapFormatToDXGI(getFormat(attrInfo, componentType));
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

        uint32_t getPerElementCount(const tinygltf::Accessor& accessor)
        {
            switch (accessor.type) {
            case TINYGLTF_TYPE_SCALAR:
                return 1u;
            case TINYGLTF_TYPE_VEC2:
                return 2u;
            case TINYGLTF_TYPE_VEC3:
                return 3u;
            case TINYGLTF_TYPE_VEC4:
            case TINYGLTF_TYPE_MAT2:
                return 4u;
            case TINYGLTF_TYPE_MAT3:
                return 9u;
            case TINYGLTF_TYPE_MAT4:
                return 16u;
            }
            return 0;
        }

        uint32_t getByteSize(const tinygltf::Accessor& accessor)
        {
            return getByteStride(accessor) * getPerElementCount(accessor);
        }

        uint64_t getMeshMapKey(const uint32_t meshIdx, const uint32_t primitiveIdx)
        {
            return (uint64_t(meshIdx) << 32) | uint64_t(primitiveIdx);
        }

        template<class T>
        void copyAccessorDataToVector(const tinygltf::Model* inputModel, const tinygltf::Accessor& accessor, std::vector<T>& dst)
        {
            // Num elements is useful for flattening Vec3 and Vec4 arrays
            const tinygltf::BufferView& bufferView = inputModel->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = inputModel->buffers[bufferView.buffer];
            ASSERT(bufferView.byteStride == 0);
            dst.resize(accessor.count);
            memcpy(dst.data(), &buffer.data.at(accessor.byteOffset + bufferView.byteOffset), accessor.count * getByteSize(accessor));
        }


        InputLayoutDetail getInputLayoutDetails(const tinygltf::Accessor& accessor, const AttributeInfo& attrInfo)
        {
            // TODO refactor to use new GPUBuffer formats and info
            InputLayoutDetail detail{};
            detail.attrMask = attrInfo.attrBit;
            detail.format = getDXFormat(attrInfo, accessor.componentType);
            detail.semanticName = attrInfo.semanticName;
            detail.vectorSize = getPerElementCount(accessor);

            switch (accessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_BYTE:
            case TINYGLTF_COMPONENT_TYPE_SHORT:
            case TINYGLTF_COMPONENT_TYPE_INT:
                detail.type = InputLayoutDetail::Type::INT;
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                if (attrInfo.attrBit & MeshAttributes::BLENDINDICES) {
                    detail.type = InputLayoutDetail::Type::UINT;
                }
                else {
                    detail.type = InputLayoutDetail::Type::FLOAT;
                }
                break;
            default:
                detail.type = InputLayoutDetail::Type::FLOAT;
            }

            return detail;
        };

        Transform processTransform(const tinygltf::Node& inputNode)
        {
            Transform transform{};
            if (inputNode.scale.size() == 3) {
                transform.scale = Vector3{
                    static_cast<float>(inputNode.scale[0]),
                    static_cast<float>(inputNode.scale[1]),
                    static_cast<float>(inputNode.scale[2]),
                };
                transform.mask |= TransformType::Scale;
            }

            if (inputNode.rotation.size() == 4) {
                transform.rotation = Quaternion{
                    static_cast<float>(inputNode.rotation[0]),
                    static_cast<float>(inputNode.rotation[1]),
                    static_cast<float>(inputNode.rotation[2]),
                    static_cast<float>(inputNode.rotation[3]),
                };
                transform.mask |= TransformType::Rotation;
            }

            if (inputNode.translation.size() == 3) {
                transform.translation = Vector3{
                    static_cast<float>(inputNode.translation[0]),
                    static_cast<float>(inputNode.translation[1]),
                    static_cast<float>(inputNode.translation[2]),
                };
                transform.mask |= TransformType::Translation;
            }

            return transform;
        }

        // Allocates a new entity and adds an entry to the mapping for each node, recursively.
        void traverseNode(SceneData& sceneData, int32_t inputNodeIdx, int32_t parentNodeIdx = -1)
        {
            const SceneNode& node = sceneData.nodes[inputNodeIdx];

            // Copy
            SceneNode output{ node };

            if (parentNodeIdx > -1) {
                output.parentId = parentNodeIdx;
            }

            if (output.meshId > -1) {
                const tinygltf::Mesh& inputMesh = sceneData.inputModel->meshes[output.meshId];
                int32_t parentId = parentNodeIdx;
                for (int32_t primitiveIdx = 0; primitiveIdx < inputMesh.primitives.size(); ++primitiveIdx) {
                    SceneNode newNode{ output };
                    newNode.primitiveId = primitiveIdx;
                    newNode.parentId = parentId;
                    sceneData.traversedNodes.push_back(newNode);
                    // We do this so that subsequent primitives actually have the node as their parent.
                    // Basically if our tree looks like A -> B, and B has three primitives, this flattens
                    // to A, B1(A), B2(B1), B3(B1). Not sure this is preferable to A, B, C1(B), C2(B), C3(B)
                    parentId = inputNodeIdx;
                }
            }
            else {
                sceneData.traversedNodes.push_back(output);
            }

            // If there are children, process them and reference the current entity as their parents
            const tinygltf::Node& inputNode = sceneData.inputModel->nodes[inputNodeIdx];
            for (const int inputChildIdx : inputNode.children) {
                traverseNode(sceneData, inputChildIdx, inputNodeIdx);
            }
        }

        //void createBuffer(SceneData& sceneData, ID3D11Buffer** dxBuffer, const tinygltf::Accessor& accessor, const uint32_t bindFlags)
        //{
        //    const tinygltf::Model& inputModel = *sceneData.inputModel;
        //    const tinygltf::BufferView& bufferView = inputModel.bufferViews[accessor.bufferView];
        //    const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];

        //    D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(getByteSize(accessor) * accessor.count, bindFlags);
        //    if (bindFlags & D3D11_BIND_UNORDERED_ACCESS) {
        //        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
        //    }

        //    D3D11_SUBRESOURCE_DATA initData;
        //    initData.pSysMem = &buffer.data.at(accessor.byteOffset + bufferView.byteOffset);
        //    initData.SysMemPitch = 0;
        //    initData.SysMemSlicePitch = 0;

        //    // Create the buffer with the device.
        //    DX::ThrowIfFailed(sceneData.pRenderer->getDevice()->CreateBuffer(&bufferDesc, &initData, dxBuffer));
        //}

        void processVertexBuffers(
            SceneData& sceneData,
            const tinygltf::Primitive& inputPrimitive,
            Mesh& mesh,
            const AttributeInfo attrInfos[],
            const size_t attrInfoCount,
            const uint8_t usage
        )
        {
            uint32_t vertBufferIdx = 0;
            for (size_t i = 0; i < attrInfoCount; ++i) {
                const AttributeInfo& attrInfo = attrInfos[i];
                const std::string& attrName = attrInfo.name;

                if (inputPrimitive.attributes.count(attrName) == 0) {
                    if (attrInfo.flags & AttributeInfo::REQUIRED) {
                        throw std::runtime_error("Cannot handle meshes without " + attrName);
                    }
                    else {
                        continue;
                    }
                }

                int accessorIndex = inputPrimitive.attributes.at(attrName);
                const tinygltf::Accessor& accessor = sceneData.inputModel->accessors[accessorIndex];
                const tinygltf::BufferView& bufferView = sceneData.inputModel->bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = sceneData.inputModel->buffers[bufferView.buffer];
                const size_t offset = accessor.byteOffset + bufferView.byteOffset;
                ASSERT(bufferView.byteStride == 0, "Cannot handle byte strides for our vertex attributes");

                if (attrInfo.attrBit & MeshAttributes::POSITION) {
                    mesh.numVertices = accessor.count;
                }

                BufferCreationInfo createInfo{};
                createInfo.numElements = accessor.count;
                if (!(attrInfo.flags & AttributeInfo::USED_FOR_SKINNING)) {
                    // If it's not used for skinning, then don't create views for it.
                    createInfo.usage = usage & ~(BufferUsage::ShaderReadable | BufferUsage::ComputeWritable);
                }
                else {
                    createInfo.usage = usage;
                }
                createInfo.format = getFormat(attrInfo, accessor.componentType);

                GPUBuffer gpuBuffer = createBuffer(sceneData.pRenderer->getDevice(), &buffer.data.at(offset), createInfo);
                mesh.vertexBuffers[vertBufferIdx] = gpuBuffer.buffer;
                mesh.srvs[vertBufferIdx] = gpuBuffer.srv;
                mesh.uavs[vertBufferIdx] = gpuBuffer.uav;
                mesh.presentAttributesMask |= attrInfo.attrBit;
                mesh.strides[vertBufferIdx] = getByteSize(accessor);
                ++vertBufferIdx;
            }
            mesh.numPresentAttr = uint8_t(vertBufferIdx);

        }

        uint32_t getInputLayout(SceneData& sceneData, const tinygltf::Primitive& inputPrimitive, const AttributeInfo attrInfos[], const size_t attrInfoCount)
        {
            InputLayoutDetail details[Mesh::maxAttrCount] = {};
            for (size_t i = 0; i < attrInfoCount; ++i) {
                const AttributeInfo& attrInfo = attrInfos[i];
                const std::string& attrName = attrInfo.name;

                if (inputPrimitive.attributes.count(attrName) == 0) {
                    if (attrInfo.flags & AttributeInfo::REQUIRED) {
                        throw std::runtime_error("Cannot handle meshes without " + attrName);
                    }
                    else {
                        continue;
                    }
                }

                int accessorIndex = inputPrimitive.attributes.at(attrName);
                const tinygltf::Accessor& accessor = sceneData.inputModel->accessors[accessorIndex];

                details[i] = getInputLayoutDetails(accessor, attrInfo);
            }
            return sceneData.pRenderer->getInputLayout(details, attrInfoCount);
        }

        uint32_t processPrimitive(SceneData& sceneData, const tinygltf::Primitive& inputPrimitive)
        {
            const tinygltf::Model& inputModel = *sceneData.inputModel;

            const uint32_t meshIdx = sceneData.pRenderer->getNewMesh();
            Mesh& mesh = sceneData.pRenderer->meshes[meshIdx];

            // Index buffer
            const tinygltf::Accessor& indexAccessor = inputModel.accessors[inputPrimitive.indices];
            mesh.numIndices = uint32_t(indexAccessor.count);

            BufferCreationInfo indexCreateInfo{};
            indexCreateInfo.numElements = indexAccessor.count;
            indexCreateInfo.usage = BufferUsage::Index;
            GPUBuffer gpuBuffer;
            {
                const tinygltf::BufferView& bufferView = inputModel.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];
                size_t offset = indexAccessor.byteOffset + bufferView.byteOffset;

                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    indexCreateInfo.format = BufferFormat::UINT16;
                    // Need to recast our indices
                    std::vector<uint16_t> indices(indexAccessor.count);
                    // Copy the data from the buffer, casting each element
                    for (size_t i = 0; i < indexAccessor.count; ++i) {
                        size_t elementOffset = indexAccessor.byteOffset + bufferView.byteOffset + i;
                        indices[i] = uint16_t(buffer.data.at(elementOffset));
                    }
                    gpuBuffer = createBuffer(sceneData.pRenderer->getDevice(), indices.data(), indexCreateInfo);
                }
                else {

                    if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        indexCreateInfo.format = BufferFormat::UINT16;
                    }
                    else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        indexCreateInfo.format = BufferFormat::UINT32;
                    }
                    else {
                        throw std::runtime_error("Cannot support indices of this type!");
                    }
                    gpuBuffer = createBuffer(sceneData.pRenderer->getDevice(), &buffer.data.at(offset), indexCreateInfo);
                }
            }
            mesh.indexBuffer = gpuBuffer.buffer;
            mesh.indexFormat = mapFormatToDXGI(indexCreateInfo.format);

            bool isSkinned = inputPrimitive.attributes.count("JOINTS_0") > 0 && inputPrimitive.attributes.count("WEIGHTS_0") > 0;

            uint8_t usage = BufferUsage::Vertex | (isSkinned ? BufferUsage::ComputeWritable : 0);
            processVertexBuffers(sceneData, inputPrimitive, mesh, genericAttrInfo, _countof(genericAttrInfo), usage);
            mesh.inputLayoutHandle = getInputLayout(sceneData, inputPrimitive, genericAttrInfo, mesh.numPresentAttr);

            if (isSkinned) {
                const uint32_t preskinIdx = sceneData.pRenderer->getNewMesh();
                Mesh& preskinMesh = sceneData.pRenderer->meshes[preskinIdx];
                processVertexBuffers(sceneData, inputPrimitive, preskinMesh, preskinAttrInfo, _countof(preskinAttrInfo), BufferUsage::ShaderReadable);

                sceneData.pRenderer->meshes[meshIdx].preskinMeshIdx = preskinIdx;
            }
            return meshIdx;
        }

        void processMeshes(SceneData& sceneData)
        {
            const auto& inputModel = *sceneData.inputModel;

            for (uint32_t inputMeshIdx = 0; inputMeshIdx < inputModel.meshes.size(); ++inputMeshIdx) {
                const tinygltf::Mesh& inputMesh = inputModel.meshes[inputMeshIdx];

                for (uint32_t primitiveIdx = 0; primitiveIdx < inputMesh.primitives.size(); ++primitiveIdx) {
                    const auto& primitive = inputMesh.primitives[primitiveIdx];
                    uint32_t meshIdx = processPrimitive(sceneData, primitive);

                    uint64_t key = getMeshMapKey(inputMeshIdx, primitiveIdx);
                    sceneData.meshMap[key] = meshIdx;
                }
            }
        }

        Skin processSkin(SceneData& sceneData, const tinygltf::Skin& inputSkin)
        {
            Skin skin{
                std::vector<uint32_t>(inputSkin.joints.size()),
                std::vector<Matrix>(inputSkin.joints.size()),
            };

            for (size_t i = 0; i < skin.jointEntities.size(); ++i) {
                skin.jointEntities[i] = sceneData.nodeToEntityMap[inputSkin.joints[i]];
            }

            const tinygltf::Accessor& accessor = sceneData.inputModel->accessors[inputSkin.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = sceneData.inputModel->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = sceneData.inputModel->buffers[bufferView.buffer];
            memcpy(skin.inverseBindMatrices.data(), &buffer.data.at(accessor.byteOffset + bufferView.byteOffset), getByteSize(accessor) * accessor.count);

            return skin;
        }

        template<typename ChannelT, typename OutputT>
        ChannelT processChannel(SceneData& sceneData, const tinygltf::AnimationChannel& inputChannel, const tinygltf::AnimationSampler& inputSampler)
        {
            Animation::InterpolationType interpolationType = Animation::InterpolationType::CubicSpline;
            if (inputSampler.interpolation.compare("LINEAR")) {
                interpolationType = Animation::InterpolationType::Linear;
            }
            else if (inputSampler.interpolation.compare("STEP")) {
                interpolationType = Animation::InterpolationType::Step;
            }

            const tinygltf::Accessor& inputAccessor = sceneData.inputModel->accessors[inputSampler.input];
            const tinygltf::Accessor& outputAccessor = sceneData.inputModel->accessors[inputSampler.output];

            if (inputAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || outputAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                throw std::runtime_error("Don't support non float samplers just yet");
            }

            ChannelT channel{};
            channel.targetEntity = sceneData.nodeToEntityMap[inputChannel.target_node];
            channel.maxInput = inputAccessor.maxValues[0];
            channel.interpolationType = interpolationType;

            copyAccessorDataToVector<float>(sceneData.inputModel, inputAccessor, channel.input);
            copyAccessorDataToVector<OutputT>(sceneData.inputModel, outputAccessor, channel.output);

            return channel;
        }

        Animation processAnimation(SceneData& sceneData, const tinygltf::Animation& animation)
        {
            Animation output{};

            for (size_t i = 0; i < animation.channels.size(); ++i) {
                const tinygltf::AnimationChannel& inputChannel = animation.channels[i];
                const tinygltf::AnimationSampler& inputSampler = animation.samplers[inputChannel.sampler];

                if (inputChannel.target_path.compare("scale") == 0) {
                    auto channel{ processChannel<Animation::ScaleChannel, Vector3>(sceneData, inputChannel, inputSampler) };
                    addScaleChannel(output, std::move(channel));
                }
                else if (inputChannel.target_path.compare("rotation") == 0) {
                    auto channel{ processChannel<Animation::RotationChannel, Vector4>(sceneData, inputChannel, inputSampler) };
                    addRotationChannel(output, std::move(channel));
                }
                else if (inputChannel.target_path.compare("translation") == 0) {
                    auto channel{ processChannel<Animation::TranslationChannel, Vector3>(sceneData, inputChannel, inputSampler) };
                    addTranslationChannel(output, std::move(channel));
                }
                else {
                    Utility::Print("Don't support weights yet!");
                    continue;
                }
            }

            return output;
        }

        // Load model takes an instance of sceneData and loads the entire GLTF scene into our ECS registry,
        // loading materials, meshes defined in the scene and adding them to the renderer.
        void loadModel(SceneData& sceneData)
        {
            tinygltf::TinyGLTF loader;
            loader.SetImageLoader(loadImageDataCallback, nullptr);
            std::string err, warn;
            tinygltf::Model gltfModel;
            sceneData.inputModel = &gltfModel;
            bool res = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, sceneData.fileFolder + sceneData.fileName);

            if (!warn.empty()) {
                std::cout << warn << std::endl;
            }

            if (!err.empty()) {
                std::cout << err << std::endl;
            }

            if (!res) {
                throw std::runtime_error("Failed to load GLTF Model");
            }

            // Copy Node List
            sceneData.nodes.resize(sceneData.inputModel->nodes.size());
            for (uint32_t i = 0; i < sceneData.nodes.size(); i++) {
                const tinygltf::Node& node = sceneData.inputModel->nodes[i];

                SceneNode output;
                output.name = node.name;
                output.index = i;
                output.meshId = node.mesh;
                output.skinId = node.skin;
                output.isJoint = false;
                sceneData.nodes[i] = output;
            }

            // Mark Joint Nodes
            for (size_t i = 0; i < sceneData.inputModel->skins.size(); i++) {
                const tinygltf::Skin& inputSkin = sceneData.inputModel->skins[i];
                for (const auto jointIdx : inputSkin.joints) {
                    sceneData.nodes[jointIdx].isJoint = true;
                }
            }

            // Traverse Scene, producing list of entities to create
            const auto& rootNodes = gltfModel.scenes[gltfModel.defaultScene].nodes;
            for (int32_t rootNode : rootNodes) {
                traverseNode(sceneData, rootNode);
            }

            // sceneData.traversedNodes is now full, in traversal order.
            processMeshes(sceneData);

            // Create entities
            sceneData.nodeToEntityMap.resize(sceneData.nodes.size());
            ECSRegistry& registry = sceneData.pScene->registry;
            for (size_t i = 0; i < sceneData.traversedNodes.size(); i++) {
                const SceneNode& node = sceneData.traversedNodes[i];
                uint32_t entity = getNewEntity(registry);
                // Don't map submeshes
                if (node.primitiveId < 1) {
                    sceneData.nodeToEntityMap[node.index] = entity;
                }

                if (node.parentId != -1) {
                    registry.parents[entity] = sceneData.nodeToEntityMap[node.parentId];
                    registry.cmpMasks[entity] |= PARENT;
                }

                if (node.meshId != -1) {
                    registry.meshes[entity] = sceneData.meshMap[getMeshMapKey(node.meshId, node.primitiveId)];
                    registry.materials[entity] = 0;
                    registry.cmpMasks[entity] |= MESH | MATERIAL;

                    if (node.skinId != -1) {
                        registry.skinIds[entity] = node.skinId;
                        registry.cmpMasks[entity] |= SKIN;
                    }
                }


                registry.transforms[entity] = processTransform(sceneData.inputModel->nodes[node.index]);
                registry.localMatrices[entity] = getMatrixFromTransform(registry.transforms[entity]);
                registry.cmpMasks[entity] |= TRANSFORM;
            }

            // Create Skins
            for (size_t i = 0; i < sceneData.inputModel->skins.size(); i++) {
                const tinygltf::Skin& inputSkin = sceneData.inputModel->skins[i];
                Skin skin = processSkin(sceneData, inputSkin);
                GPUBuffer jointBuffer = createJointBuffer(sceneData.pRenderer->getDevice(), skin);
                sceneData.pRenderer->jointBuffers.push_back(jointBuffer);
                sceneData.pScene->skins.push_back(std::move(skin));
            }

            // Create aniamtions
            for (size_t i = 0; i < sceneData.inputModel->animations.size(); i++) {
                const tinygltf::Animation& animation = sceneData.inputModel->animations[i];
                sceneData.pScene->animations.push_back(processAnimation(sceneData, animation));
            }

            sceneData.inputModel = nullptr;
        }

    }
}