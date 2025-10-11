#include <memory.h>
#include <types/number.h>

void
memzero(void* ptr, size_t count)
{
        u8* u8_ptr = ptr;
        for (size_t i = 0; i < count; i++) {
                u8_ptr[i] = 0;
        }
}

void
memcopy(void* dest, const void* src, size_t count)
{
        u8* u8_dest = dest;
        const u8* u8_src = src;
        for (size_t i = 0; i < count; i++) {
                u8_dest[i] = u8_src[i];
        }
}
