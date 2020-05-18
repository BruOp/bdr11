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

        Array(const std::initializer_list<T>& list)
        {
            grow(list.size());
            for (size_t i = 0; i < list.size(); i++) {
                data[i] = list[i];
            }
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
            capacity = other.capacity;
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

        size_t pushBack(const T newData)
        {
            if (size == capacity) {
                grow();
            }
            data[size] = newData;
            return size++;
        }

        void reallocate(const size_t newCapacity)
        {
            if (newCapacity < capacity) {
                DEBUGPRINT("Shrinking Array!");
            }
            capacity = newCapacity;
            data = (T*)Memory::reallocate(data, capacity * elementByteSize);
        }
    };

    template<typename T>
    size_t findIndexOf(const Array<T>& array, const T& value)
    {
        for (size_t i = 0; i < array.size; i++) {
            if (array[i] == value) {
                return i;
            }
        }
        return SIZE_MAX;
    }
}
