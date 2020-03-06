#ifndef BDR_MEMORY
#define BDR_MEMORY
namespace bdr
{
    namespace Memory
    {
        inline void* allocate(uint32_t allocSizeBytes);
        inline void* reallocate(void* data, uint32_t sizeBytes);
        inline void zero(void* data, uint32_t sizeBytes);
        inline void release(void* data);

        void* allocate(uint32_t allocSizeBytes)
        {
            return malloc(allocSizeBytes);
        }

        void* reallocate(void* data, uint32_t sizeBytes)
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
#endif BDR_MEMORY