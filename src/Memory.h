#pragma once
#include "pch.h"

namespace bdr
{
    namespace memory
    {
        void* alloc(uint32_t allocSizeBytes)
        {
            return malloc(allocSizeBytes);
        }

        void* realloc(void* data, uint32_t sizeBytes)
        {
            return realloc(data, sizeBytes);
        }

        void zero(void* data, uint32_t sizeBytes)
        {
            memset(data, 0, sizeBytes);
        }

        void release(void* data)
        {
            free(data);
        }
    }
}