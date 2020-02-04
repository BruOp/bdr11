#include "pch.h"

#include "ECSRegistry.h"
#include "Core/bdrMemory.h"


namespace bdr
{
    void initializeFreeList(ECSRegistry& registry)
    {
        ASSERT(registry.pFreeNode == nullptr, "Not sure why we're initializing a non empty free list!");
        registry.pFreeNode = nullptr;

        // Build our linked list, starting from the back
        for (int32_t i = registry.numAllocatedEntities - 1; i >= 0; --i) {
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

    void increaseRegistrySize(ECSRegistry& registry, uint32_t size)
    {
        uint32_t newSize = registry.numAllocatedEntities + size;

        for (size_t i = 0; i < registry.numComponents; i++) {
            GenericComponentArray& cmpArray = registry.getComponentArray(i);
            uint32_t newMemSize = cmpArray.sizeOfComponent * newSize;

            if (cmpArray.data != nullptr) {
                cmpArray.data = bdr::Memory::reallocate(cmpArray.data, newMemSize);

                // Fill new Memory with zeros
                uint32_t prevMemSize = registry.numAllocatedEntities * cmpArray.sizeOfComponent;
                uint8_t* offset = (uint8_t*)cmpArray.data + prevMemSize;
                bdr::Memory::zero(offset, newMemSize - prevMemSize);
            }
            else {
                // No existing data! Allocate Memory
                cmpArray.data = Memory::allocate(newMemSize);
                bdr::Memory::zero(cmpArray.data, newMemSize);
            }
        }

        registry.numAllocatedEntities = newSize;
        initializeFreeList(registry);
    }

    uint32_t createEntity(ECSRegistry& registry)
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
    }

    void assignMesh(ECSRegistry& registry, const uint32_t entity, const uint32_t meshId)
    {
        registry.meshes[entity] = meshId;
        registry.cmpMasks[entity] |= CmpMasks::MESH;
    }

    void assignMaterial(ECSRegistry& registry, const uint32_t entity, const uint32_t materialId)
    {
        registry.meshes[entity] = materialId;
        registry.cmpMasks[entity] |= CmpMasks::MATERIAL;
    }

    void assignTransform(ECSRegistry& registry, const uint32_t entity, const Transform& transform)
    {
        registry.transforms[entity] = transform;
        registry.cmpMasks[entity] = CmpMasks::TRANSFORM;
    }

    void ECSRegistry::clearComponentData()
    {
        for (uint32_t i = 0; i < numComponents; ++i) {
            GenericComponentArray& cmpArray = getComponentArray(i);
            bdr::Memory::release(cmpArray.data);
            cmpArray.data = nullptr;
        }

        numAllocatedEntities = 0;
        numEntities = 0;
    }
}