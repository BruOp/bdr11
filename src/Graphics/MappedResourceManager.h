#pragma once
#include "pch.h"
#include "Core/Map.h"

namespace bdr
{
    template <typename T, typename Handle>
    class MappedResourceManager
    {
    public:
        MappedResourceManager() : resources{} { }
        ~MappedResourceManager()
        {
            for (T& resource : resources.values) {
                bdr::reset(resource);
            }
        }

        UNMOVABLE(MappedResourceManager);
        UNCOPIABLE(MappedResourceManager);

        Handle create(const Handle handle)
        {
            resources.
        }

        Handle getOrCreate(const Handle handle)
        {

        }

        bool get_in(const Handle handle, T* outValue) const
        {

        }

        T& get(const Handle handle) const
        {

        }
    private:
        <T> resources;

    };
}