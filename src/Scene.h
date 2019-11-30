#pragma once
#include "pch.h"
#include <vector>
#include <array>
#include "DeviceResources.h"
#include "NodeList.h"
#include "Animation.h"
#include "ECSRegistry.h"


namespace tinygltf
{
    struct Accessor;
    struct Primitive;
    struct Skin;
    struct Animation;
    class Model;
}

namespace bdr
{

    enum MeshAttributes : uint8_t
    {
        POSITION = 1,
        NORMAL = 2,
        TEXCOORD = 4,
        TANGENT = 8,
        BLENDINDICES = 16,
        BLENDWEIGHT = 32,
    };

    /*template<typename T>
    struct ResourceManager
    {
        uint32_t sizeOfComponent = sizeof(T);
        T* data;

        inline T& operator[](const size_t idx) const
        {
            return data[idx];
        };
        inline const T& operator[](const size_t idx) const
        {
            return data[idx];
        };
    };*/

    struct Mesh
    {
        static constexpr size_t maxAttrCount = 6u;

        std::array<ID3D11Buffer*, maxAttrCount> vertexBuffers;
        ID3D11Buffer* indexBuffer;
        DXGI_FORMAT indexFormat;
        uint32_t indexCount = 0;
        uint32_t strides[maxAttrCount];
        uint8_t presentAttributesMask;
        uint8_t numPresentAttr;

        void destroy()
        {
            for (auto buffer : vertexBuffers) {
                buffer != nullptr && buffer->Release();
            }
            indexBuffer->Release();
        }
    };

    class Scene
    {
    public:
        ECSRegistry registry;
        std::vector<Skin> skins;
        std::vector<Animation> animations;
        std::vector<Mesh> meshes;

        void reset()
        {
            for (auto& mesh : meshes) {
                mesh.destroy();
            }

            registry.clearComponentData();
        }

        uint32_t addMesh(const Mesh& mesh) {
            meshes.push_back(mesh);
            return meshes.size() - 1;
        };
        uint32_t addMesh(Mesh&& mesh) {
            meshes.push_back(std::forward<Mesh>(mesh));
            return meshes.size() - 1;
        };
    };
}
