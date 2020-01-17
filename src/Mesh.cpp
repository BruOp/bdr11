#include "pch.h"
#include "Mesh.h"

namespace bdr
{
    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11Buffer* outputBuffers[])
    {
        ASSERT((mesh.presentAttributesMask & attrsToSelect) == attrsToSelect, "Mesh does not have requested attributes");
        size_t counter = 0;
        for (size_t i = 0; i < mesh.maxAttrCount; i++) {
            if (attrsToSelect & (1 << i)) {
                outputBuffers[counter++] = mesh.vertexBuffers[i].buffer;
            }
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
}