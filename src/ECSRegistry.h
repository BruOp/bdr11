#pragma once
#include "pch.h"

#include "bdrMath.h"
#include "Animation.h"

namespace bdr
{
    enum CmpMasks : uint32_t
    {
        ALLOCATED = (1 << 0),
        PARENT = (1 << 1),
        SKIN = (1 << 2),
        MESH = (1 << 3),
        TRANSFORM = (1 << 4),
        MATERIAL = (1 << 5),
    };

    template<typename T>
    struct ComponentArray
    {
        const uint32_t sizeOfComponent = sizeof(T);
        T* data = nullptr;

        inline T& operator[](const size_t entityId)
        {
            return data[entityId];
        };
        inline const T& operator[](const size_t entityId) const
        {
            return data[entityId];
        };
    };

    struct GenericComponentArray
    {
        const uint32_t sizeOfComponent;
        void* data;

        void* operator[](const size_t index)
        {
            uint8_t* byteData = (uint8_t*)data;
            uint8_t* indexedByteData = &byteData[index * sizeOfComponent];
            return (void*)(indexedByteData);
        };
    };

    struct DrawConstants
    {
        DirectX::SimpleMath::Matrix model;
        DirectX::SimpleMath::Matrix invModel;
    };

    struct GenericMaterialData
    {
        float data[64];

        inline float& operator[](const size_t index)
        {
            return data[index];
        }
        inline const float& operator[](const size_t index) const
        {
            return data[index];
        }
    };


    // Used to retrieve the next "free" entity, one that has been allocated by unused.
    struct FreeEntityNode
    {
        uint32_t node;
        FreeEntityNode* next = nullptr;
        FreeEntityNode* prev = nullptr;
    };

    class ECSRegistry
    {
    public:
        ECSRegistry() : numComponents{ 0 }, numAllocatedEntities{ 0 }, pFreeNode{ nullptr }
        {
            numComponents = uint32_t((((size_t)&numComponents - (size_t)&cmpMasks)) / sizeof(GenericComponentArray));
        };
        ~ECSRegistry()
        {
            clearComponentData();
        };

        UNCOPIABLE(ECSRegistry);
        UNMOVABLE(ECSRegistry);

        // Component Data
        ComponentArray<uint32_t> cmpMasks;
        ComponentArray<uint32_t> parents;
        ComponentArray<uint32_t> skinIds;
        ComponentArray<uint32_t> meshes;
        ComponentArray<uint32_t> preskinMeshes;
        ComponentArray<uint32_t> jointBuffer;
        ComponentArray<Transform> transforms;
        ComponentArray<DirectX::SimpleMath::Matrix> localMatrices;
        ComponentArray<DirectX::SimpleMath::Matrix> globalMatrices;
        ComponentArray<FreeEntityNode> freeEntitiesNodes;
        ComponentArray<uint32_t> materials;
        ComponentArray<GenericMaterialData> materialData;
        ComponentArray<DrawConstants> drawConstants;

        // This is used to calculate the number of components and store them.
        // DO NOT MOVE -- it's position in the layout of this class is crucial to it's functionality
        uint32_t numComponents = 0;

        uint32_t numEntities = 0;
        uint32_t numAllocatedEntities = 0;
        FreeEntityNode* pFreeNode = nullptr;

        inline GenericComponentArray& getComponentArray(uint32_t index)
        {
            if (index >= numComponents) {
                throw std::runtime_error("Index is greater than or equal to our number of component types");
            }

            GenericComponentArray* dataBegin = (GenericComponentArray*)this;
            return dataBegin[index];
        };

        void clearComponentData();
    };

    void initializeFreeList(ECSRegistry& registry);

    void increaseRegistrySize(ECSRegistry& registry, uint32_t size = 1024);

    uint32_t getNewEntity(ECSRegistry& registry);

}

