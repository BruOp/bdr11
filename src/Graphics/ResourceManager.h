#pragma once
#include "pch.h"


namespace bdr
{
    template <typename T, typename Handle>
    class ResourceManager
    {
    public:
        ResourceManager() : resources{} { };
        ~ResourceManager()
        {
            for (T& resource : resources) {
                bdr::reset(resource);
            }
        }

        UNMOVABLE(ResourceManager);
        UNCOPIABLE(ResourceManager);

        inline Handle create()
        {
            const size_t idx = resources.size();
            resources.emplace_back();
            return { uint32_t(idx) };
        }

        inline void add(T& resource)
        {
            resources.push_back(resource);
        }

        inline size_t size() const
        {
            return resources.size();
        }

        T& operator[](const size_t idx)
        {
            return resources[idx];
        };

        const T& operator[](const size_t idx) const
        {
            return resources[idx];
        };

        T& operator[](const Handle handle)
        {
            return resources[handle.idx];
        };

        const T& operator[](const Handle handle) const
        {
            return resources[handle.idx];
        };

    private:
        std::vector<T> resources;
    };
}