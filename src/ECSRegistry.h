#pragma once
#include "pch.h"

namespace bdr
{
    template<typename T>
    struct ComponentArray
    {
        uint32_t sizeOfComponent = sizeof(T);
        T* data = nullptr;

        inline T& operator[](const size_t entityId)
        {
            return components[entityId];
        };
        inline const T& operator[](const size_t entityId) const
        {
            return components[entityId];
        };
    };

    struct GenericComponentArray
    {
        uint32_t sizeOfComponent;
        void* data;

        void* operator[](const size_t index)
        {
            uint8_t* byteData = (uint8_t*)data;
            uint8_t* indexedByteData = &byteData[index * sizeOfComponent];
            return (void*)(indexedByteData);
        };
    };

    class ECSRegistry
    { 
    public:
        ECSRegistry()
        {
            numComponents = (((size_t)&numComponents - (size_t)&entities)) / sizeof(ECSRegistry);
        };
        ~ECSRegistry() = default;

        ECSRegistry(const ECSRegistry&) = delete;
        ECSRegistry(ECSRegistry&&) = delete;

        ECSRegistry& operator=(ECSRegistry&&) = delete;
        ECSRegistry& operator=(const ECSRegistry&) = delete;

        // Component Data
        ComponentArray<uint32_t> entities;
        ComponentArray<uint32_t> sceneNodeIds;
        ComponentArray<uint32_t> skinIds;
        ComponentArray<DirectX::SimpleMath::Matrix> localTransforms;
        ComponentArray<DirectX::SimpleMath::Matrix> globalTransforms;

        // This is use to calculate the number of components and store them.
        // DO NOT MOVE -- it's position in the layout of this struct is crucial to it's functionality
        uint32_t numComponents;

        uint32_t numEntities = 0;

        inline GenericComponentArray& getGenericComponent(uint32_t index) {
            if (index >= numComponents) {
                throw std::runtime_error("Index is greater than or equal to our number of component types");
            }

            GenericComponentArray* dataBegin = (GenericComponentArray*)this;
            return dataBegin[index];
        };

    };
}

