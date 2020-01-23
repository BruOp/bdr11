
#include "pch.h"

#include "GltfSceneLoader.h"
#include "GPUBuffer.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tinygltf/tiny_gltf.h"


using namespace DirectX::SimpleMath;
namespace bdr
{
    namespace gltf
    {
        struct MeshData
        {
            uint8_t const* p_indices = nullptr;
            BufferFormat indexFormat = BufferFormat::INVALID;
            // Different handle for each "stream" of vertex attributes
            // 0 - Position
            // 1 - Normal
            // 2 - TexCoord0
            // 3 - Weights
            // 4 - BlendIndices
            // 5 - Tangent
            uint8_t* data[Mesh::maxAttrCount] = { nullptr };
            BufferFormat bufferFormats[Mesh::maxAttrCount] = { BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID };
            uint8_t presentAttributesMask = 0;
            size_t strides[Mesh::maxAttrCount] = { 0 };
            size_t numIndices = 0;
            size_t numVertices = 0;
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

        constexpr uint32_t TANGENT_IDX = 5;

        AttributeInfo genericAttrInfo[] = {
            ATTR_INFO[0], ATTR_INFO[1], ATTR_INFO[2],
        };
        AttributeInfo preskinAttrInfo[] = {
            ATTR_INFO[0], ATTR_INFO[1], ATTR_INFO[3], ATTR_INFO[4]
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

        void processMeshData(SceneData& sceneData, Mesh& mesh, MeshData& meshData, const uint8_t usage, const bool isPreskin = false)
        {
            uint8_t numPresentAttr = 0;
            for (size_t i = 0; i < _countof(ATTR_INFO); ++i) {
                const AttributeInfo& attrInfo = ATTR_INFO[i];

                BufferCreationInfo createInfo{};
                createInfo.numElements = meshData.numVertices;
                if (meshData.data[i] == nullptr) {
                    continue;
                }

                if (!isPreskin && (attrInfo.flags & AttributeInfo::PRESKIN_ONLY)) {
                    continue;
                }

                if (!(attrInfo.flags & AttributeInfo::USED_FOR_SKINNING)) {
                    // If it's not used for skinning, then don't create views for it.
                    createInfo.usage = usage & ~(BufferUsage::ShaderReadable | BufferUsage::ComputeWritable);
                }
                else {
                    createInfo.usage = usage;
                }
                createInfo.format = meshData.bufferFormats[i];

                GPUBuffer gpuBuffer = createBuffer(sceneData.pRenderer->getDevice(), meshData.data[i], createInfo);
                mesh.vertexBuffers[i] = gpuBuffer;
                mesh.presentAttributesMask |= attrInfo.attrBit;
                mesh.strides[i] = meshData.strides[i];
                ++numPresentAttr;
            }
            mesh.numPresentAttr = numPresentAttr;
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
            MeshData meshData;

            // Index buffer
            const tinygltf::Accessor& indexAccessor = inputModel.accessors[inputPrimitive.indices];
            meshData.numIndices = uint32_t(indexAccessor.count);

            // Process indices
            std::vector<uint16_t> indices;
            {
                const tinygltf::BufferView& bufferView = inputModel.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];
                size_t offset = indexAccessor.byteOffset + bufferView.byteOffset;
                meshData.p_indices = &buffer.data.at(offset);
                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    meshData.indexFormat = BufferFormat::UINT16;

                    // Need to recast our indices
                    indices.resize(meshData.numIndices);
                    // Copy the data from the buffer, casting each element
                    for (size_t i = 0; i < indexAccessor.count; ++i) {
                        size_t elementOffset = indexAccessor.byteOffset + bufferView.byteOffset + i;
                        indices[i] = uint16_t(buffer.data.at(elementOffset));
                    }
                    meshData.p_indices = (uint8_t*)(indices.data());
                }
                else {
                    meshData.p_indices = &buffer.data.at(offset);
                    if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        meshData.indexFormat = BufferFormat::UINT16;
                    }
                    else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        meshData.indexFormat = BufferFormat::UINT32;
                    }
                    else {
                        throw std::runtime_error("Cannot support indices of this type!");
                    }
                }
            }

            bool isSkinned = inputPrimitive.attributes.count("JOINTS_0") > 0 && inputPrimitive.attributes.count("WEIGHTS_0") > 0;

            for (size_t i = 0; i < _countof(ATTR_INFO); ++i) {
                const AttributeInfo& attrInfo = ATTR_INFO[i];
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
                    meshData.numVertices = accessor.count;
                }

                meshData.data[i] = (uint8_t*)(&buffer.data.at(accessor.byteOffset + bufferView.byteOffset));
                meshData.strides[i] = getByteSize(accessor);
                meshData.bufferFormats[i] = getFormat(attrInfo, accessor.componentType);
                meshData.presentAttributesMask |= attrInfo.attrBit;
            }

            const uint32_t meshIdx = sceneData.pRenderer->getNewMesh();
            Mesh& mesh = sceneData.pRenderer->meshes[meshIdx];
            mesh.numIndices = meshData.numIndices;
            mesh.numVertices = meshData.numVertices;

            BufferCreationInfo indexCreateInfo{};
            indexCreateInfo.numElements = meshData.numIndices;
            indexCreateInfo.usage = BufferUsage::Index;
            indexCreateInfo.format = meshData.indexFormat;

            GPUBuffer gpuBuffer;
            gpuBuffer = createBuffer(sceneData.pRenderer->getDevice(), meshData.p_indices, indexCreateInfo);
            mesh.indexBuffer = gpuBuffer;
            uint8_t usage = BufferUsage::Vertex | (isSkinned ? BufferUsage::ComputeWritable : 0);
            processMeshData(sceneData, mesh, meshData, usage, false);

            mesh.inputLayoutHandle = getInputLayout(sceneData, inputPrimitive, genericAttrInfo, mesh.numPresentAttr);

            if (isSkinned) {
                const uint32_t preskinIdx = sceneData.pRenderer->getNewMesh();
                Mesh& preskinMesh = sceneData.pRenderer->meshes[preskinIdx];
                usage = BufferUsage::ShaderReadable;
                processMeshData(sceneData, preskinMesh, meshData, usage, true);

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

        void processTextures(SceneData& sceneData)
        {
            const tinygltf::Model& inputModel = *sceneData.inputModel;
            std::vector<bool> creationLog(inputModel.images.size(), false);
            sceneData.textureMap.resize(inputModel.images.size());

            // While we loop through the textures, we only process individual images once.
            // We ignore the edge case where a GLTF file has different textures pointing to
            // the same image source but uses different samplers.
            for (size_t i = 0; i < inputModel.textures.size(); i++) {
                const tinygltf::Texture& texture = inputModel.textures[i];
                const tinygltf::Image image = inputModel.images[texture.source];
                const size_t outputTextureIdx = texture.source;

                // Early abort if we've already store the output as a function of the texture source
                if (creationLog[outputTextureIdx]) {
                    continue;
                }

                TextureCreationInfo createInfo{};
                createInfo.dims[0] = image.width;
                createInfo.dims[1] = image.height;
                createInfo.usage = BufferUsage::ShaderReadable;

                uint32_t textureIdx = sceneData.pRenderer->createTextureFromFile(sceneData.fileFolder + image.uri, createInfo);
                Texture& output = sceneData.pRenderer->textures[textureIdx];

                D3D11_SAMPLER_DESC samplerDesc{};
                samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
                samplerDesc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
                samplerDesc.MipLODBias = D3D11_DEFAULT_MIP_LOD_BIAS;
                samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
                samplerDesc.MinLOD = 0.0f;
                samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
                samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
                samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                if (texture.sampler != -1) {
                    const tinygltf::Sampler sampler = inputModel.samplers[texture.sampler];
                    switch (sampler.wrapS) {
                    case TINYGLTF_TEXTURE_WRAP_REPEAT:
                        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
                        break;
                    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
                        break;
                    }

                    switch (sampler.wrapT) {
                    case TINYGLTF_TEXTURE_WRAP_REPEAT:
                        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
                        break;
                    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
                        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
                        break;
                    }
                }

                DX::ThrowIfFailed(sceneData.pRenderer->getDevice()->CreateSamplerState(&samplerDesc, &output.sampler));
                creationLog[outputTextureIdx] = true;
                sceneData.textureMap[outputTextureIdx] = textureIdx;
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
                Utility::Print(warn.c_str());
            }

            if (!err.empty()) {
                Utility::Print(err.c_str());
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

            processTextures(sceneData);

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
            uint32_t N = sceneData.traversedNodes.size();
            for (size_t nodeIdx = 0; nodeIdx < N; nodeIdx++) {
                const SceneNode& node = sceneData.traversedNodes[nodeIdx];
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
                    registry.cmpMasks[entity] |= MESH;

                    TextureSet& textureSet = registry.textures[entity];
                    const tinygltf::Mesh& mesh = gltfModel.meshes[node.meshId];
                    const tinygltf::Primitive& primitive = mesh.primitives[node.primitiveId];
                    const tinygltf::Material& material = gltfModel.materials[primitive.material];

                    auto valuesEnd = material.values.end();
                    auto additionalValuesEnd = material.additionalValues.end();

                    auto p_gltfTexture = material.values.find("baseColorTexture");
                    if (p_gltfTexture != valuesEnd) {
                        size_t textureIdx = p_gltfTexture->second.TextureIndex();
                        size_t imgIdx = gltfModel.textures[textureIdx].source;
                        textureSet.textures[textureSet.numTextures++] = sceneData.textureMap[imgIdx];
                        textureSet.textureFlags |= TextureFlags::ALBEDO;
                    }

                    p_gltfTexture = material.values.find("metallicRoughnessTexture");
                    if (p_gltfTexture != valuesEnd) {
                        size_t textureIdx = p_gltfTexture->second.TextureIndex();
                        size_t imgIdx = gltfModel.textures[textureIdx].source;
                        textureSet.textures[textureSet.numTextures++] = sceneData.textureMap[imgIdx];
                        textureSet.textureFlags |= TextureFlags::METALLIC_ROUGHNESS;

                        // We only allow occlusion maps to be the R channel of the metallicRoughness map
                        p_gltfTexture = material.additionalValues.find("occlusionTexture");
                        if (p_gltfTexture != additionalValuesEnd) {
                            textureIdx = p_gltfTexture->second.TextureIndex();
                            size_t occlusionImgIdx = gltfModel.textures[textureIdx].source;
                            if (imgIdx == occlusionImgIdx) {
                                textureSet.textureFlags |= TextureFlags::OCCLUSION;
                            }
                            else {
                                Utility::Printf(L"Occlusion texutre at index %d is not using the same source as the metallic roughness texture", textureIdx);
                            }
                        }
                    }

                    p_gltfTexture = material.additionalValues.find("normalTexture");
                    if (p_gltfTexture != additionalValuesEnd) {
                        size_t textureIdx = p_gltfTexture->second.TextureIndex();
                        size_t imgIdx = gltfModel.textures[textureIdx].source;
                        textureSet.textures[textureSet.numTextures++] = sceneData.textureMap[imgIdx];
                        textureSet.textureFlags |= TextureFlags::NORMAL_MAP;
                    }

                    p_gltfTexture = material.additionalValues.find("emissiveTexture");
                    if (p_gltfTexture != additionalValuesEnd) {
                        size_t textureIdx = p_gltfTexture->second.TextureIndex();
                        size_t imgIdx = gltfModel.textures[textureIdx].source;
                        textureSet.textures[textureSet.numTextures++] = sceneData.textureMap[imgIdx];
                        textureSet.textureFlags |= TextureFlags::EMISSIVE;
                    }

                    PBRConstants factors{};
                    if (material.values.count("baseColorFactor")) {
                        auto data = material.values.at("baseColorFactor").ColorFactor();
                        for (size_t i = 0; i < 4u; i++) {
                            factors.baseColorFactor[i] = float(data[i]);
                        }
                    }

                    if (material.values.count("metallicFactor")) {
                        factors.metallicFactor = float(material.values.at("metallicFactor").Factor());
                    }

                    if (material.values.count("roughnessFactor")) {
                        factors.roughnessFactor = float(material.values.at("roughnessFactor").Factor());
                    }

                    if (material.additionalValues.count("emissiveFactor")) {
                        auto data = material.additionalValues.at("emissiveFactor").ColorFactor();
                        for (size_t i = 0; i < 3u; i++) { // Emissive values cannot have alpha
                            factors.emissiveFactor[i] = float(data[i]);
                        }
                    }
                    // Copy our material data
                    memcpy(&registry.materialData[entity], &factors, sizeof(factors));

                    registry.materials[entity] = getOrCreatePBRMaterial(sceneData.pRenderer->materials, textureSet.textureFlags);
                    registry.cmpMasks[entity] |= MATERIAL;

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
                GPUBuffer jointBuffer{ createJointBuffer(sceneData.pRenderer->getDevice(), skin) };
                sceneData.pRenderer->jointBuffers.add(jointBuffer);
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