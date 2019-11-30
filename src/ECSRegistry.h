#pragma once
#include "pch.h"

#include "Memory.h"
#include "Animation.h"

namespace bdr
{
    enum CmpMasks : uint32_t
    {
        ALLOCATED = (1 << 0),
        PARENT = (1 << 1),
        SKIN = (1 << 2),
        MESH = (1 << 3),
        TRANFORM = (1 << 4),
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

    struct Transform
    {
        DirectX::SimpleMath::Quaternion rotation;
        DirectX::SimpleMath::Vector3 translation;
        uint32_t mask;
        DirectX::SimpleMath::Vector3 scale;
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
        ECSRegistry()
        {
            numComponents = (((size_t)&numComponents - (size_t)&cmpMasks)) / sizeof(ECSRegistry);
        };
        ~ECSRegistry()
        {
            clearComponentData();
        };

        ECSRegistry(const ECSRegistry&) = delete;
        ECSRegistry(ECSRegistry&&) = delete;

        ECSRegistry& operator=(ECSRegistry&&) = delete;
        ECSRegistry& operator=(const ECSRegistry&) = delete;

        // Component Data
        ComponentArray<uint32_t> cmpMasks;
        ComponentArray<uint32_t> parents;
        ComponentArray<uint32_t> skinIds;
        ComponentArray<uint32_t> meshes;
        ComponentArray<Transform> transforms;
        ComponentArray<DirectX::SimpleMath::Matrix> localMatrices;
        ComponentArray<DirectX::SimpleMath::Matrix> globalMatrices;
        ComponentArray<FreeEntityNode> freeEntitiesNodes;

        // This is used to calculate the number of components and store them.
        // DO NOT MOVE -- it's position in the layout of this struct is crucial to it's functionality
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

        void clearComponentData()
        {
            for (uint32_t i = 0; i < numComponents; ++i) {
                GenericComponentArray& cmpArray = getComponentArray(i);
                memory::release(cmpArray.data);
                cmpArray.data = nullptr;
            }

            numAllocatedEntities = 0;
            numEntities = 0;
        }
    };

    void initializeFreeList(ECSRegistry& registry)
    {
        ASSERT(registry.pFreeNode == nullptr, "Not sure why we're initializing a non empty free list!");
        registry.pFreeNode = nullptr;

        // Build our linked list, starting from the back
        for (size_t i = registry.numAllocatedEntities; i > 0; --i) {
            registry.freeEntitiesNodes[i].node = i;
            
            if (!(registry.cmpMasks[i] & CmpMasks::ALLOCATED)) {
                FreeEntityNode* head = &registry.freeEntitiesNodes[i];
                head->next = registry.pFreeNode;

                if (head->next != nullptr) {
                    head->next->prev = head;
                }

                registry.pFreeNode = head;
            }
        }

        ASSERT(registry.pFreeNode != nullptr);
    }

    void increaseRegistrySize(ECSRegistry& registry, uint32_t size = 1024)
    {
        uint32_t newSize = registry.numAllocatedEntities + size;

        for (size_t i = 0; i < registry.numComponents; i++) {
            GenericComponentArray& cmpArray = registry.getComponentArray(i);
            uint32_t newMemSize = cmpArray.sizeOfComponent * newSize;

            if (cmpArray.data != nullptr) {
                cmpArray.data = memory::realloc(cmpArray.data, newMemSize);

                // Fill new memory with zeros
                uint32_t prevMemSize = registry.numAllocatedEntities * cmpArray.sizeOfComponent;
                uint8_t* offset = (uint8_t*)cmpArray.data + prevMemSize;
                memory::zero(offset, newMemSize - prevMemSize);
            }
            else {
                // No existing data! Allocate memory
                cmpArray.data = memory::alloc(newMemSize);
                memory::zero(cmpArray.data, newMemSize);
            }
        }

        registry.numAllocatedEntities = newSize;
        initializeFreeList(registry);
    }

    uint32_t getNewEntity(ECSRegistry& registry)
    {
        if (registry.pFreeNode == nullptr) {
            // Need to allocate new entities
            increaseRegistrySize(registry);
        }

        const FreeEntityNode* head = registry.pFreeNode;
        uint32_t entity = head->node;

        registry.numEntities = registry.numEntities + 1;
        registry.pFreeNode = head->next;
        // Set default entity state
        registry.parents[entity] = 0;
        registry.cmpMasks[entity] |= CmpMasks::ALLOCATED;
        return entity;
    };
    
}

