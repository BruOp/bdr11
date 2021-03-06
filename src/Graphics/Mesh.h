#pragma once
#include "pch.h"
#include "Resources.h"
#include "GPUBuffer.h"

namespace bdr
{
    class Renderer;

    void reset(Mesh& mesh);

    void collectBuffers(const Mesh& mesh, ID3D11Buffer* outputBuffers[]);
    void collectBuffers(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11Buffer* outputBuffers[]);
    void collectViews(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11ShaderResourceView* outputSRVs[]);
    void collectViews(const Mesh& mesh, const uint8_t attrsToSelect, ID3D11UnorderedAccessView* outputUAVs[]);

    void addAttribute(MeshCreationInfo& meshCreationInfo, const void* data, const BufferFormat format, const MeshAttribute attrFlag);

    MeshHandle createMesh(Renderer& renderer, const MeshCreationInfo& meshCreateInfo);
}