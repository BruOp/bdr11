#include "pch.h"

#include "GltfSceneLoader.h"
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

        DXGI_FORMAT getFormat(const AttributeInfo& attrInfo, int32_t componentType)
        {
            switch (attrInfo.attrBit) {
            case MeshAttributes::POSITION:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case MeshAttributes::NORMAL:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            case MeshAttributes::TEXCOORD:
                if (componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    return DXGI_FORMAT_R32G32_FLOAT;
                }
                else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    return DXGI_FORMAT_R8G8_UNORM;
                }
                else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    return DXGI_FORMAT_R16G16_UNORM;
                }
            case MeshAttributes::TANGENT:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            case MeshAttributes::BLENDINDICES:
                if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    return DXGI_FORMAT_R8G8B8A8_UINT;
                }
                else if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    return DXGI_FORMAT_R16G16B16A16_UINT;
                }
            case MeshAttributes::BLENDWEIGHT: // WEIGHTS_0
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


        InputLayoutDetail getInputLayoutDetails(const tinygltf::Accessor& accessor, const AttributeInfo& attrInfo)
        {
            InputLayoutDetail detail{};
            detail.attrMask = attrInfo.attrBit;
            detail.format = getFormat(attrInfo, accessor.componentType);
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

        Matrix getMatrixFromTransform(const Transform& transform)
        {
            Matrix localTransform = Matrix::Identity;
            if (transform.mask & TransformType::Scale) {
                localTransform *= Matrix::CreateScale(transform.scale);
            }
            if (transform.mask & TransformType::Rotation) {
                localTransform = Matrix::Transform(localTransform, transform.rotation);
            }
            if (transform.mask & TransformType::Translation) {
                localTransform *= Matrix::CreateTranslation(transform.translation);
            }
            return localTransform;
        };


        // Allocates a new entity and adds an entry to the mapping for each node, recursively.
        void processNode(SceneData& sceneData, int32_t inputNodeIdx, int32_t parentNodeIdx = -1)
        {
            const tinygltf::Node& inputNode = sceneData.inputModel->nodes[inputNodeIdx];
            auto& registry = sceneData.pScene->registry;

            const uint32_t entity = getNewEntity(registry);
            sceneData.nodeMap[inputNodeIdx] = entity;

            if (parentNodeIdx > -1) {
                // This is safe only because we process the tree depth first
                registry.parents[entity] = sceneData.nodeMap[parentNodeIdx];
                registry.cmpMasks[entity] |= CmpMasks::PARENT;
            }
            else {
                registry.parents[entity] = entity;
            }

            if (inputNode.mesh > -1) {
                const tinygltf::Mesh& inputMesh = sceneData.inputModel->meshes[inputNode.mesh];

                if (inputMesh.primitives.size() > 1) {
                    // If we have multiple primitves, we create a new entity for each one, assigning the
                    // current entity as their mutual parent.
                    for (size_t i = 0; i < inputMesh.primitives.size(); ++i) {
                        const auto& primitive = inputMesh.primitives[i];
                        const uint32_t childEntity = getNewEntity(registry);
                        registry.parents[childEntity] = entity;
                        // Our mesh map guarentees that primitives are stored adjacent to one another.
                        registry.meshes[childEntity] = sceneData.meshMap[inputNode.mesh] + i;
                        registry.materials[childEntity] = sceneData.pRenderer->materials[0];
                        registry.localMatrices[childEntity] = Matrix::Identity;
                        registry.cmpMasks[childEntity] |= (CmpMasks::MESH | CmpMasks::PARENT | CmpMasks::MATERIAL);
                    }
                }
                else {
                    // Otherwise, we just assign the existing mesh to our entity
                    registry.meshes[entity] = sceneData.meshMap[inputNode.mesh];
                    registry.materials[entity] = sceneData.pRenderer->materials[0];
                    registry.cmpMasks[entity] |= (CmpMasks::MESH | CmpMasks::MATERIAL);
                }
            }

            // If there are children, process them and reference the current entity as their parents
            if (inputNode.children.size() > 0) {
                for (const int inputChildIdx : inputNode.children) {
                    processNode(sceneData, inputChildIdx, inputNodeIdx);
                }
            }
        }

        /*
            Create entities for each Node and it's primitives in the scene
            While creating entities, fill mapping between gltf Scene nodes and our entities.
            Nodes are produced _depth first_, ensuring that parents appear before their children in the entity list.
        */
        void processNodeTree(SceneData& sceneData)
        {
            sceneData.nodeMap.resize(sceneData.inputModel->nodes.size());
            for (const tinygltf::Scene& inputScene : sceneData.inputModel->scenes) {
                for (const int32_t node : inputScene.nodes) {
                    processNode(sceneData, node);
                }
            }
        }

        void createBuffer(SceneData& sceneData, ID3D11Buffer** dxBuffer, const tinygltf::Accessor& accessor, const uint32_t usageFlag)
        {
            const tinygltf::Model& inputModel = *sceneData.inputModel;
            const tinygltf::BufferView& bufferView = inputModel.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];

            D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(getByteSize(accessor) * accessor.count, usageFlag);
            D3D11_SUBRESOURCE_DATA initData;
            initData.pSysMem = &buffer.data.at(accessor.byteOffset + bufferView.byteOffset);
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;

            // Create the buffer with the device.
            DX::ThrowIfFailed(sceneData.pRenderer->getDevice()->CreateBuffer(&bufferDesc, &initData, dxBuffer));
        }

        uint32_t processPrimitive(SceneData& sceneData, const tinygltf::Primitive& inputPrimitive)
        {
            const tinygltf::Model& inputModel = *sceneData.inputModel;
            Mesh mesh{};
            Mesh preskin{};
            // Index buffer
            const tinygltf::Accessor& indexAccessor = inputModel.accessors[inputPrimitive.indices];
            mesh.numIndices = uint32_t(indexAccessor.count);
            switch (indexAccessor.componentType) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            {
                mesh.indexFormat = DXGI_FORMAT_R16_UINT;
                // Need to recast our indices
                std::vector<uint16_t> indices(indexAccessor.count);
                // Copy the data from the buffer, casting each element
                const tinygltf::BufferView& bufferView = inputModel.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& buffer = inputModel.buffers[bufferView.buffer];
                for (size_t i = 0; i < indexAccessor.count; ++i) {
                    size_t offset = indexAccessor.byteOffset + bufferView.byteOffset + i;
                    indices[i] = uint16_t(buffer.data.at(offset));
                }
                // Create our buffer
                D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC(sizeof(uint16_t) * indices.size(), D3D11_BIND_INDEX_BUFFER);
                D3D11_SUBRESOURCE_DATA initData;
                initData.pSysMem = indices.data();
                initData.SysMemPitch = 0;
                initData.SysMemSlicePitch = 0;

                // Create the buffer with the device.
                DX::ThrowIfFailed(sceneData.pRenderer->getDevice()->CreateBuffer(&bufferDesc, &initData, &mesh.indexBuffer));
            }
            break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                mesh.indexFormat = DXGI_FORMAT_R16_UINT;
                createBuffer(sceneData, &mesh.indexBuffer, indexAccessor, D3D11_BIND_INDEX_BUFFER);
                break;
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                mesh.indexFormat = DXGI_FORMAT_R32_UINT;
                createBuffer(sceneData, &mesh.indexBuffer, indexAccessor, D3D11_BIND_INDEX_BUFFER);
                break;
            default:
                throw std::runtime_error("Cannot support indices of this type!");
            }

            bool isSkinned = inputPrimitive.attributes.count("JOINTS_0") > 0 && inputPrimitive.attributes.count("WEIGHTS_0") > 0;
            
            uint32_t vertBufferIdx = 0;
            uint32_t preskinBufferIdx = 0;
            InputLayoutDetail details[Mesh::maxAttrCount] = {};
            for (size_t i = 0; i < _countof(ATTR_INFO); i++) {
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

                if (!(attrInfo.flags & AttributeInfo::PRESKIN_ONLY)) {
                    createBuffer(sceneData, &mesh.vertexBuffers[vertBufferIdx], accessor, D3D11_BIND_VERTEX_BUFFER);
                    mesh.presentAttributesMask |= attrInfo.attrBit;
                    mesh.strides[vertBufferIdx] = getByteSize(accessor);

                    details[vertBufferIdx] = getInputLayoutDetails(accessor, attrInfo);
                    
                    // If the mesh is skinned and it's a relevant attribute, create UAVs
                    if (isSkinned && (attrInfo.flags & AttributeInfo::USED_FOR_SKINNING)) {
                        DX::ThrowIfFailed(sceneData.pRenderer->getDevice()->CreateUnorderedAccessView(mesh.vertexBuffers[vertBufferIdx], nullptr, &mesh.uavs[vertBufferIdx]));
                    }

                    ++vertBufferIdx;
                }
                
                if (attrInfo.attrBit & MeshAttributes::POSITION) {
                    mesh.numVertices = accessor.count;
                    preskin.numVertices = accessor.count;
                }

                if (isSkinned && (attrInfo.flags & AttributeInfo::USED_FOR_SKINNING)) {
                    createBuffer(sceneData, &preskin.vertexBuffers[preskinBufferIdx], accessor, D3D11_BIND_UNORDERED_ACCESS);
                    preskin.presentAttributesMask |= attrInfo.attrBit;
                    preskin.strides[preskinBufferIdx] = getByteSize(accessor);
                    DX::ThrowIfFailed(sceneData.pRenderer->getDevice()->CreateUnorderedAccessView(preskin.vertexBuffers[preskinBufferIdx], nullptr, &preskin.uavs[preskinBufferIdx]));
                    ++preskinBufferIdx;
                }                
            }
            mesh.numPresentAttr = uint8_t(vertBufferIdx);
            preskin.numPresentAttr = uint8_t(preskinBufferIdx);

            if (isSkinned) {
                sceneData.pRenderer->addPreskinMesh(preskin);
            }
            return sceneData.pRenderer->addMesh(mesh, details);
        }

        void processMeshes(SceneData& sceneData)
        {
            const auto& inputModel = *sceneData.inputModel;
            sceneData.meshMap.resize(inputModel.meshes.size());
            for (size_t i = 0; i < inputModel.meshes.size(); ++i) {
                const tinygltf::Mesh& inputMesh = inputModel.meshes[i];

                const auto& primitive = inputMesh.primitives[0];
                uint32_t parentMeshIdx = processPrimitive(sceneData, primitive);
                sceneData.meshMap[i] = parentMeshIdx;

                if (inputMesh.primitives.size() > 1) {
                    for (size_t j = 1; j < inputMesh.primitives.size(); ++j) {
                        const auto& childPrimitive = inputMesh.primitives[j];
                        uint32_t meshIdx = processPrimitive(sceneData, childPrimitive);
                        // We want to guarentee that our primitives are always tightly packed.
                        ASSERT(meshIdx == parentMeshIdx + j);
                    }
                }
            }
        }

        // Populates components for the entities associated with the scene nodes
        // Notice that it does not touch the entities created for meshes with multiple
        // primitives, as those will have a common parent that this does update.
        void processTransforms(SceneData& sceneData)
        {
            auto& registry = sceneData.pScene->registry;
            const auto& inputNodes = sceneData.inputModel->nodes;
            for (size_t i = 0; i < inputNodes.size(); ++i) {
                const tinygltf::Node& node = inputNodes[i];
                uint32_t entity = sceneData.nodeMap[i];
                const Transform transform = processTransform(node);
                registry.transforms[entity] = transform;
                registry.cmpMasks[entity] |= CmpMasks::TRANSFORM;
                const Matrix local = getMatrixFromTransform(transform);
                registry.localMatrices[entity] = local;

                uint32_t parent = registry.parents[entity];
                if (parent == entity) {
                    registry.globalMatrices[entity] = local;
                }
                else {
                    registry.globalMatrices[entity] = local * registry.globalMatrices[parent];
                }
            }
        }

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

            processMeshes(sceneData);
            processNodeTree(sceneData);
            processTransforms(sceneData);

            sceneData.inputModel = nullptr;
        }

        /*Skin processSkin(SceneData& sceneData, const tinygltf::Skin& inputSkin)
        {
            Skin skin{
                std::vector<int32_t>(inputSkin.joints.size()),
                std::vector<Matrix>(inputSkin.joints.size()),
            };

            for (size_t i = 0; i < skin.jointIndices.size(); ++i) {
                skin.jointIndices[i] = sceneData.nodeMap[inputSkin.joints[i]];
            }

            const tinygltf::Accessor& accessor = sceneData.inputModel->accessors[inputSkin.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = inputModel->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = inputModel->buffers[bufferView.buffer];
            memcpy(skin.inverseBindMatrices.data(), &buffer.data.at(accessor.byteOffset + bufferView.byteOffset), getByteSize(accessor) * accessor.count);
            return skin;
        }*/

        //Animation processAnimation(SceneData& sceneData, const tinygltf::Animation& animation)
        //{
        //    Animation output{};

        //    output.channels.reserve(animation.channels.size());
        //    for (size_t i = 0; i < animation.channels.size(); ++i) {
        //        const tinygltf::AnimationChannel& inputChannel = animation.channels[i];
        //        const tinygltf::AnimationSampler& inputSampler = animation.samplers[inputChannel.sampler];

        //        uint32_t numElements = 3;
        //        TransformType channelType;
        //        if (inputChannel.target_path.compare("scale") == 0) {
        //            channelType = TransformType::Scale;
        //        }
        //        else if (inputChannel.target_path.compare("rotation") == 0) {
        //            channelType = TransformType::Rotation;
        //            numElements = 4;
        //        }
        //        else if (inputChannel.target_path.compare("translation") == 0) {
        //            channelType = TransformType::Translation;
        //        }
        //        else {
        //            Utility::Print("Don't support weights yet!");
        //            continue;
        //        }

        //        Animation::InterpolationType interpolationType = Animation::InterpolationType::CubicSpline;
        //        if (inputSampler.interpolation.compare("LINEAR")) {
        //            interpolationType = Animation::InterpolationType::Linear;
        //        }
        //        else if (inputSampler.interpolation.compare("STEP")) {
        //            interpolationType = Animation::InterpolationType::Step;
        //        }

        //        const tinygltf::Accessor& inputAccessor = sceneData.inputModel->accessors[inputSampler.input];

        //        if (inputAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
        //            throw std::runtime_error("Don't support non float samplers just yet");
        //        }

        //        const tinygltf::Accessor& outputAccessor = sceneData.inputModel->accessors[inputSampler.input];

        //        if (outputAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
        //            throw std::runtime_error("Don't support non float samplers just yet");
        //        }

        //        Animation::Channel animationChannel{
        //            sceneData.nodeMap[inputChannel.target_node],
        //            inputAccessor.maxValues[0],
        //            channelType,
        //            interpolationType,
        //        };

        //        copyAccessorDataToVector<float>(sceneData.inputModel, inputAccessor, animationChannel.input);

        //        std::vector<float> data{};
        //        animationChannel.output.reserve(outputAccessor.count);
        //        if (animationChannel.targetType == TransformType::Rotation) {

        //            copyAccessorDataToVector<float>(sceneData.inputModel, inputAccessor, data, 4);

        //            for (size_t j = 0; j < outputAccessor.count; j++) {
        //                animationChannel.output.emplace_back(
        //                    data[j * 4 + 0],
        //                    data[j * 4 + 1],
        //                    data[j * 4 + 2],
        //                    data[j * 4 + 3]
        //                );
        //            }
        //        }
        //        else {
        //            copyAccessorDataToVector<float>(sceneData.inputModel, inputAccessor, data, 3);
        //            for (size_t j = 0; j < outputAccessor.count; j++) {
        //                animationChannel.output.emplace_back(
        //                    data[j * 3 + 0],
        //                    data[j * 3 + 1],
        //                    data[j * 3 + 2],
        //                    0.0f
        //                );
        //            }
        //        }

        //        output.channels.push_back(animationChannel);
        //    }
        //    return output;
        //}
    }
}