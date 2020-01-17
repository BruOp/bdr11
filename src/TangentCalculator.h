#pragma once
#include "pch.h"

#include "mikktspace/mikktspace.h"
#include "GPUBuffer.h"

namespace bdr
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
        uint8_t* data[6] = { nullptr };
        BufferFormat bufferFormats[6] = { BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID, BufferFormat::INVALID };
        uint8_t presentAttributesMask = 0;
        size_t strides[6] = { 0 };
        size_t numIndices = 0;
        size_t numVertices = 0;
    };

    namespace MikktSpace
    {
        uint32_t getIndex(const MeshData* meshData, const int iface, const int ivert)
        {
            uint32_t i = iface * 3 + ivert;
            uint32_t indexByteSize = getByteSize(meshData->indexFormat);
            if (meshData->indexFormat == BufferFormat::UINT16) {
                return *(uint16_t*)(meshData->p_indices + (i * indexByteSize));
            }
            else {
                return *(uint32_t*)(meshData->p_indices + (i * indexByteSize));
            }
        }

        int getNumFaces(const SMikkTSpaceContext* ctx)
        {
            const MeshData* meshData = static_cast<const MeshData*>(ctx->m_pUserData);
            return meshData->numIndices / 3u;
        };

        int getNumVerticesOfFace(const SMikkTSpaceContext*, const int)
        {
            return 3;
        };

        void getNormal(const SMikkTSpaceContext* ctx, float normals[], const int iface, const int ivert)
        {
            constexpr size_t NORMAL_INDEX = 1;
            const MeshData* meshData = static_cast<const MeshData*>(ctx->m_pUserData);
            uint32_t index = getIndex(meshData, iface, ivert);
            memcpy(normals, meshData->data[NORMAL_INDEX] + (index * meshData->strides[NORMAL_INDEX]), meshData->strides[NORMAL_INDEX]);
        };

        void getTexCoord(const SMikkTSpaceContext* ctx, float texCoordOut[], const int iface, const int ivert)
        {
            constexpr size_t TEX_INDEX = 2;
            const MeshData* meshData = static_cast<const MeshData*>(ctx->m_pUserData);
            uint32_t index = getIndex(meshData, iface, ivert);
            if (meshData->bufferFormats[TEX_INDEX] == BufferFormat::FLOAT_2) {
                memcpy(texCoordOut, meshData->data[TEX_INDEX] + (index * meshData->strides[TEX_INDEX]), meshData->strides[TEX_INDEX]);
            }
            else if (meshData->bufferFormats[TEX_INDEX] == BufferFormat::UNORM8_2) {
                uint8_t texCoord[2] = { 0, 0 };
                memcpy(texCoord, meshData->data[TEX_INDEX] + (index * meshData->strides[TEX_INDEX]), meshData->strides[TEX_INDEX]);
                texCoordOut[0] = float(texCoord[0]) / float(UINT8_MAX);
                texCoordOut[1] = float(texCoord[1]) / float(UINT8_MAX);
            }
            else if (meshData->bufferFormats[TEX_INDEX] == BufferFormat::UNORM16_2) {
                uint16_t texCoord[2] = { 0, 0 };
                memcpy(texCoord, meshData->data[TEX_INDEX] + (index * meshData->strides[TEX_INDEX]), meshData->strides[TEX_INDEX]);
                texCoordOut[0] = float(texCoord[0]) / float(UINT16_MAX);
                texCoordOut[1] = float(texCoord[1]) / float(UINT16_MAX);
            }
        };

        void getPosition(const SMikkTSpaceContext* ctx, float positions[], const int iface, const int ivert)
        {
            constexpr size_t POSITION_INDEX = 0;
            const MeshData* meshData = static_cast<const MeshData*>(ctx->m_pUserData);
            uint32_t index = getIndex(meshData, iface, ivert);
            memcpy(positions, meshData->data[POSITION_INDEX] + (index * meshData->strides[POSITION_INDEX]), meshData->strides[POSITION_INDEX]);
        };

        void setTSpaceBasic(const SMikkTSpaceContext* ctx, const float tangent[], const float sign,
            const int iface, const int ivert)
        {
            const MeshData* meshData = static_cast<const MeshData*>(ctx->m_pUserData);
            uint32_t index = getIndex(meshData, iface, ivert);
            // Invert the sign because of glTF convention.
            float t[4]{ tangent[0], tangent[1], tangent[2], -sign };
            memcpy(meshData->data[5] + index * meshData->strides[5], t, meshData->strides[5]);
        };


        void calcTangents(const MeshData& meshData)
        {
            SMikkTSpaceInterface iface;
            iface.m_getNumFaces = getNumFaces;
            iface.m_getNumVerticesOfFace = getNumVerticesOfFace;
            iface.m_getPosition = getPosition;
            iface.m_getNormal = getNormal;
            iface.m_getTexCoord = getTexCoord;
            iface.m_setTSpaceBasic = setTSpaceBasic;
            iface.m_setTSpace = nullptr;

            SMikkTSpaceContext context{
                &iface,
                (void*)&meshData,
            };
            genTangSpaceDefault(&context);
        }
    }
}
