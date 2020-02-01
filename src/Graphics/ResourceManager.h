#pragma once
#include "pch.h"


namespace bdr
{
    template <typename T>
    class ResourceManager
    {
    public:
        ResourceManager() : resources{} { };
        ~ResourceManager()
        {
            for (T& resource : resources) {
                resource.reset();
            }
        }

        UNMOVABLE(ResourceManager);
        UNCOPIABLE(ResourceManager);

        inline size_t create()
        {
            const size_t idx = resources.size();
            resources.emplace_back();
            return idx;
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

    private:
        std::vector<T> resources;
    };
}