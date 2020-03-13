#include "pch.h"
#include "Mesh.h"
#include "Renderer.h"

namespace bdr
{
    size_t getAttributeOrder(MeshAttribute attrBit)
    {
        if (attrBit & bdr::POSITION) {
            return 0;
        }
        else if (attrBit & bdr::NORMAL) {
            return 1;
        }
        else if (attrBit & bdr::TEXCOORD) {
            return 2;
        }
        else if (attrBit & bdr::BLENDINDICES) {
            return 3;
        }
        else if (attrBit & bdr::BLENDWEIGHT) {
            return 4;
        }
        else if (attrBit & bdr::COLOR) {
            return 5;
        }
        else {
            HALT("Invalid Attribute");
            return UINT64_MAX;
        }
    }

    void reset(Mesh& mesh)
    {
        for (auto& vertexBuffer : mesh.vertexBuffers) {
            reset(vertexBuffer);
        }
        reset(mesh.indexBuffer);
        mesh.numIndices = 0;
        mesh.numVertices = 0;
    }

    void collectBuffers(const Mesh& mesh, ID3D11Buffer* outputBuffers[])
    {
        size_t counter = 0;
        for (size_t i = 0; i < mesh.numPresentAttr; i++) {
            outputBuffers[counter++] = mesh.vertexBuffers[i].buffer;
        }
    }

    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11Buffer* outputBuffers[])
    {
        if ((mesh.presentAttributesMask & attrsToSelect) != attrsToSelect) {
            HALT("Mesh does not have requested attributes");
        }
        size_t counter = 0;
        for (size_t i = 0; i < mesh.numPresentAttr; i++) {
            if (mesh.attributes[i] & attrsToSelect) {
                outputBuffers[counter++] = mesh.vertexBuffers[i].buffer;
            }
        }
    }

    void collectViews(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11ShaderResourceView* outputSRVs[])
    {
        if ((mesh.presentAttributesMask & attrsToSelect) != attrsToSelect) {
            HALT("Mesh does not have requested attributes");
        }
        size_t counter = 0;
        for (size_t i = 0; i < mesh.maxAttrCount; i++) {
            if (mesh.attributes[i] & attrsToSelect) {
                outputSRVs[counter++] = mesh.vertexBuffers[i].srv;
            }
        }
    }

    void collectViews(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11UnorderedAccessView* outputUAVs[])
    {
        if ((mesh.presentAttributesMask & attrsToSelect) != attrsToSelect) {
            HALT("Mesh does not have requested attributes");
        }
        size_t counter = 0;
        for (size_t i = 0; i < mesh.maxAttrCount; i++) {
            if (mesh.attributes[i] & attrsToSelect) {
                outputUAVs[counter++] = mesh.vertexBuffers[i].uav;
            }
        }
    }

    template<typename T>
    void shiftSlice(T inArray[], T outArray[], size_t sliceStartIdx, size_t sliceEndIdx)
    {
        size_t diff = sliceStartIdx - sliceEndIdx;
        memcpy(outArray + sliceStartIdx + 1, inArray + sliceStartIdx, sizeof(T) * sliceEndIdx);
    }

    void addAttribute(MeshCreationInfo& meshCreationInfo, const void* data, const BufferFormat format, const MeshAttribute attrFlag)
    {
        ASSERT(attrFlag != MeshAttribute::INVALID, "Cannot add an invalid mesh attribute");
        ASSERT(format != BufferFormat::INVALID, "Cannot add an invalid mesh attribute");
        size_t minPosition = getAttributeOrder(attrFlag);
        size_t attrIdx = meshCreationInfo.numAttributes;
        if (attrIdx > minPosition) {
            ASSERT(meshCreationInfo.numAttributes < Mesh::maxAttrCount);
            MeshCreationInfo newMeshCreationInfo = meshCreationInfo;
            size_t sliceStartIdx = minPosition;
            size_t sliceEndIdx = meshCreationInfo.numAttributes;
            shiftSlice(meshCreationInfo.data, newMeshCreationInfo.data, sliceStartIdx, sliceEndIdx);
            shiftSlice(meshCreationInfo.bufferFormats, newMeshCreationInfo.bufferFormats, sliceStartIdx, sliceEndIdx);
            shiftSlice(meshCreationInfo.attributes, newMeshCreationInfo.attributes, sliceStartIdx, sliceEndIdx);
            shiftSlice(meshCreationInfo.strides, newMeshCreationInfo.strides, sliceStartIdx, sliceEndIdx);
            shiftSlice(meshCreationInfo.bufferUsages, newMeshCreationInfo.bufferUsages, sliceStartIdx, sliceEndIdx);
            attrIdx = minPosition;
            meshCreationInfo = std::move(newMeshCreationInfo);
        }
        meshCreationInfo.data[attrIdx] = (uint8_t*)(data);
        meshCreationInfo.bufferFormats[attrIdx] = format;
        meshCreationInfo.attributes[attrIdx] = attrFlag;
        meshCreationInfo.bufferUsages[attrIdx] = BufferUsage::VERTEX;
        meshCreationInfo.strides[attrIdx] = getByteSize(format);
        meshCreationInfo.presentAttributesMask |= attrFlag;
        ++meshCreationInfo.numAttributes;
    }

    uint32_t createMesh(Renderer& renderer, const MeshCreationInfo& meshCreateInfo)
    {
        uint32_t meshId = static_cast<uint32_t>(renderer.meshes.create());
        Mesh& mesh = renderer.meshes[meshId];

        mesh.numIndices = meshCreateInfo.numIndices;
        mesh.numVertices = meshCreateInfo.numVertices;

        BufferCreationInfo indexCreateInfo{};
        indexCreateInfo.numElements = meshCreateInfo.numIndices;
        indexCreateInfo.usage = BufferUsage::INDEX;
        indexCreateInfo.format = meshCreateInfo.indexFormat;

        ID3D11Device* device = renderer.getDevice();
        mesh.indexBuffer = createBuffer(device, meshCreateInfo.indexData, indexCreateInfo);

        for (size_t i = 0; i < meshCreateInfo.numAttributes; ++i) {
            BufferCreationInfo createInfo{};
            createInfo.numElements = meshCreateInfo.numVertices;
            createInfo.usage = meshCreateInfo.bufferUsages[i];
            createInfo.format = meshCreateInfo.bufferFormats[i];
            createInfo.type = BufferType::Default;

            if (createInfo.usage == BufferUsage::UNUSED) {
                continue;
            }

            mesh.vertexBuffers[i] = createBuffer(device, meshCreateInfo.data[i], createInfo);
            mesh.attributes[i] = meshCreateInfo.attributes[i];
            mesh.presentAttributesMask |= meshCreateInfo.attributes[i];
            mesh.strides[i] = meshCreateInfo.strides[i];
        }
        mesh.numPresentAttr = meshCreateInfo.numAttributes;
        mesh.inputLayoutHandle = renderer.inputLayoutManager.getOrCreateInputLayout(meshCreateInfo);
        return meshId;
    }


}