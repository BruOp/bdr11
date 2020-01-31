#include "pch.h"
#include "Mesh.h"

namespace bdr
{
    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11Buffer* outputBuffers[])
    {
        ASSERT((mesh.presentAttributesMask & attrsToSelect) == attrsToSelect, "Mesh does not have requested attributes");
        size_t counter = 0;
        for (size_t i = 0; i < mesh.numPresentAttr; i++) {
            outputBuffers[counter++] = mesh.vertexBuffers[i].buffer;
        }
    }

    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11ShaderResourceView* outputSRVs[])
    {
        ASSERT((mesh.presentAttributesMask & attrsToSelect) == attrsToSelect, "Mesh does not have requested attributes");
        size_t counter = 0;
        for (size_t i = 0; i < mesh.maxAttrCount; i++) {
            if (attrsToSelect & (1 << i)) {
                outputSRVs[counter++] = mesh.vertexBuffers[i].srv;
            }
        }
    }

    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11UnorderedAccessView* outputUAVs[])
    {
        ASSERT((mesh.presentAttributesMask & attrsToSelect) == attrsToSelect, "Mesh does not have requested attributes");
        size_t counter = 0;
        for (size_t i = 0; i < mesh.maxAttrCount; i++) {
            if (attrsToSelect & (1 << i)) {
                outputUAVs[counter++] = mesh.vertexBuffers[i].uav;
            }
        }
    }

    void addAttribute(MeshCreationInfo& meshCreationInfo, const void* data, const BufferFormat format, const MeshAttribute attrFlag)
    {
        size_t attrIdx = meshCreationInfo.numAttributes;
        meshCreationInfo.data[attrIdx] = (uint8_t*)(data);
        meshCreationInfo.bufferFormats[attrIdx] = format;
        meshCreationInfo.attributes[attrIdx] = attrFlag;
        meshCreationInfo.bufferUsages[attrIdx] = BufferUsage::Vertex;
        meshCreationInfo.strides[attrIdx] = getByteSize(format);
        meshCreationInfo.presentAttributesMask |= attrFlag;
        ++meshCreationInfo.numAttributes;
    }
}