#pragma once
#include "pch.h"

namespace bdr
{
    template<typename T>
    struct Array
    {
        size_t size = 0;
        size_t capacity = 0;
        const size_t elementByteSize = sizeof(T);
        T* data = nullptr;

        Array() = default;
        Array(size_t startingCapacity)
        {
            grow(startingCapacity);
        }

        ~Array()
        {
            Memory::release(data);
            size = 0;
            capacity = 0;
        }

        UNCOPIABLE(Array);

        Array(Array&& other)
        {
            size = other.size;
            capcity = other.capacity;
            data = other.data;
            other.size = 0;
            other.capacity = 0;
            other.data = nullptr;
        }
        Array& operator=(Array&& other)
        {
            size = other.size;
            capcity = other.capacity;
            data = other.data;
            other.size = 0;
            other.capacity = 0;
            other.data = nullptr;
            return *this;
        }

        T& operator[](size_t idx)
        {
            ASSERT(idx < size, "Index out of bounds!");
            return data[idx];
        }
        const T& operator[](size_t idx) const
        {
            ASSERT(idx < size, "Index out of bounds!");
            return data[idx];
        }

        void grow(const size_t additionalSlots = 1024)
        {
            reallocate(capacity + additionalSlots);
        }

        size_t byteSizeOf(Array<T>& arr) const
        {
            return capacity * elementByteSize;
        }

        size_t pushBack(const T& data)
        {
            if (size == capacity) {
                grow();
            }
            data[size] = T;
            return size++;
        }

        void reallocate(const size_t newCapacity)
        {
            if (newCapacity < capacity) {
                DEBUGPRINT("Shrinking Array!");
            }
            capacity = newCapacity;
            data = Memory::reallocate(data, capacity * elementByteSize);
        }
    };

}
