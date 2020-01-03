#pragma once
#include "pch.h"
#include "bdrMemory.h"

namespace bdr
{

    template<typename T>
    class RingBuffer
    {
    public:

        ~RingBuffer()
        {
            if (data) {
                bdr::Memory::release(data);
            }
        }

        UNCOPIABLE(RingBuffer);
        UNMOVABLE(RingBuffer);

        inline void add(const T item)
        {
            if (isFull()) {
                increaseSize(capacity == 0 ? 1024 : capacity * 2);
            }

            data[end++ & (capacity - 1)] = item;
        };
        inline T& pop()
        {
            return data[start++];
        };

        inline void reset()
        {
            start = end;
        };

        inline uint32_t size()
        {
            return end - start;
        };
        inline uint32_t getCapacity()
        {
            return capacity;
        };
        inline bool isFull()
        {
            return (end - start) == capacity;
        }
        inline bool isEmpty()
        {
            return end == start;
        }

    private:
        uint32_t capacity = 0;
        uint32_t start = 0;
        uint32_t end = 0;
        T* data = nullptr;

        void increaseSize(const uint32_t newSize = 1024)
        {
            ASSERT(newSize > capacity, "Cannot decrease size of ring buffer using this function");
            size_t newMemSize = sizeof(T) * newSize
                if (data != nullptr) {
                    data = bdr::Memory::reallocate(data, newMemSize);

                    // Fill new Memory with zeros
                    size_t prevMemSize = sizeof(T) * capacity;
                    uint8_t* offset = (uint8_t*)data + prevMemSize;
                    bdr::Memory::zero(offset, newMemSize - prevMemSize);
                }
                else {
                    // No existing data! Allocate Memory
                    data = Memory::allocate(newMemSize);
                    bdr::Memory::zero(data, newMemSize);
                }
            capacity = newSize;
        }
    };
}