#pragma once
#include "pch.h"

#include <utility>

#include "murmur/MurmurHash3.h"


namespace bdr
{
    template<typename V>
    struct SimpleMap32
    {
        size_t size = 0;
        size_t capacity = 0;
        uint32_t* keys = nullptr;
        V* values = nullptr;
        // Obviously not meant to be secure!
        static constexpr uint32_t seed = 1337u;

        SimpleMap32() = default;
        SimpleMap32(const size_t startingCapacity)
        {
            grow(startingCapacity);
        }

        SimpleMap32(const std::initializer_list<std::pair<std::string, V>>& list)
        {
            grow(list.size());
            for (const auto& item : list) {
                uint32_t key = hashKey(item.first);
                fast_insert(key, item.second);
            }
        }

        ~SimpleMap32()
        {
            Memory::release(keys);
            Memory::release(values);
            size = 0;
            capacity = 0;
        }

        SimpleMap32(const SimpleMap32& other)
        {
            _copy(other);
        }
        SimpleMap32& operator=(const SimpleMap32& other)
        {
            _copy(other);
            return *this;
        }

        SimpleMap32(SimpleMap32&& other) : SimpleMap32()
        {
            _swap(std::forward<SimpleMap32<V>>(other));
        }
        SimpleMap32& operator=(SimpleMap32&& other)
        {
            _swap(std::forward<SimpleMap32<V>>(other));
            return *this;
        }

        void _copy(const SimpleMap32& other)
        {
            keys = nullptr;
            values = nullptr;
            capacity = other.capacity;
            size = other.size;
            reallocate(capacity);
            memcpy(keys, other.keys, sizeof(uint32_t) * size);
            memcpy(values, other.values, sizeof(V) * size);
        }

        void _swap(SimpleMap32&& other)
        {
            size = other.size;
            capacity = other.capacity;
            keys = other.keys;
            values = other.values;
            other.size = 0;
            other.capacity = 0;
            other.keys = nullptr;
            other.values = nullptr;
        }

        bool get(const std::string& unhashedKey, V* outValue) const
        {
            uint32_t key = hashKey(unhashedKey);

            for (size_t i = 0; i < size; i++) {
                if (keys[i] == key) {
                    *outValue = values[i];
                    return true;
                };
            }
            return false;
        };

        // Also used to update entries
        bool insert(const std::string& unhashedKey, const V& value)
        {
            uint32_t key = hashKey(unhashedKey);
            size_t index = UINT64_MAX;
            for (size_t i = 0; i < size; i++) {
                if (keys[i] == key) {
                    index = i;
                }
            }
            if (index == UINT64_MAX) {
                return fast_insert(key, value);
            }
            else {
                values[index] = value;
                return true;
            }
        };

        // Inserts without checking for existing entry. Use carefully!
        bool fast_insert(uint32_t key, const V& value)
        {
            if (size == capacity) {
                grow(capacity == 0 ? 2 : capacity * 2);
            }
            keys[size] = key;
            values[size++] = value;
            return true;
        }

        inline static uint32_t hashKey(const std::string& unhashedKey)
        {
            uint32_t key;
            MurmurHash3_x86_32(unhashedKey.data(), unhashedKey.length(), seed, &key);
            return key;
        }

        void grow(const size_t additionalSlots)
        {
            reallocate(capacity + additionalSlots);
        }

        void reallocate(const size_t newSize)
        {
            ASSERT(newSize < UINT64_MAX, "Cannont allocate such a large map!");
            if (newSize < capacity) {
                DEBUGPRINT("Shrinking Map!");
            }
            keys = (uint32_t*)(Memory::reallocate(keys, newSize * sizeof(uint32_t)));
            values = (V*)(Memory::reallocate(values, newSize * sizeof(V)));
            capacity = newSize;
        }
    };
}