#include <assert.h>
#include <fmt/print.h>
#include <kvspace.h>
#include <memory.h>
#include <riscv.h>
#include <types/error.h>
#include <types/number.h>
#include <types/slab.h>

void
slab_init(struct slab_alloc* arena, size_t objsize)
{
        ASSERT(arena != NULL);
        arena->block_size = SLAB_BLOCK_SIZE(objsize);
        arena->free_blocks = 0;
        arena->total_blocks = 0;
        arena->current_region = NULL;
        arena->block_list = NULL;
        arena->auto_refill = false;
}

void
slab_autorefill_init(struct slab_alloc* arena, size_t objsize)
{
        ASSERT(arena != NULL);
        arena->block_size = SLAB_BLOCK_SIZE(objsize);
        arena->free_blocks = 0;
        arena->total_blocks = 0;
        arena->current_region = NULL;
        arena->block_list = NULL;
        arena->auto_refill = true;
}

void
slab_grow(struct slab_alloc* arena, void* buffer, size_t buffer_size)
{
        ASSERT(arena != NULL);
        ASSERT(buffer != NULL);
        ASSERT(buffer_size >= SLAB_REGION_SIZE(arena->block_size, 1));

        struct slab_region* new_region = buffer;
        new_region->current_offset = (u8*)ALIGN_UP(((size_t)buffer) + sizeof(struct slab_region), arena->block_size);
        new_region->region_end = (u8*)ALIGN_DOWN(((size_t)buffer + buffer_size), arena->block_size);
        size_t new_blocks = (new_region->region_end - new_region->current_offset) / arena->block_size;
        arena->free_blocks += new_blocks;
        arena->total_blocks += new_blocks;

        if (arena->current_region == NULL) {
                arena->current_region = new_region;
                new_region->previous_region = NULL;
                new_region->next_region = NULL;
                return;
        }

        if (arena->current_region->next_region == NULL) {
                arena->current_region->next_region = new_region;
                new_region->next_region = NULL;
                new_region->previous_region = arena->current_region;
        } else {
                new_region->next_region = arena->current_region->next_region;
                new_region->next_region->previous_region = new_region;
                new_region->previous_region = arena->current_region;
                arena->current_region->next_region = new_region;
        }

        if (arena->current_region->current_offset == arena->current_region->region_end) {
                arena->current_region = new_region;
        }
}

void*
slab_allocate(struct slab_alloc* arena)
{
        ASSERT(arena != NULL);

        if (arena->block_list != NULL) {
                void* retval = arena->block_list;
                arena->block_list = arena->block_list->next_block;
                arena->free_blocks--;
                memzero(retval, arena->block_size);
                return retval;
        } else if (arena->free_blocks == 0 && arena->auto_refill == false) {
                return NULL;
        } else if (arena->free_blocks == 0) {
                struct allocation slab_mem = kalloc(RISCV_SV39_PAGE_SIZE, RISCV_SV39_PAGE_SIZE);
                slab_grow(arena, slab_mem.buffer, slab_mem.size);
        }

        /// Simple allocation
        if (arena->current_region->current_offset != arena->current_region->region_end) {
                void* retval = arena->current_region->current_offset;
                arena->current_region->current_offset += arena->block_size;
                arena->free_blocks--;
                memzero(retval, arena->block_size);
                return retval;
        }

        /// Move to the next region if the current one is full, we know there's at least one more
        /// region because free_blocks > 0.
        arena->current_region = arena->current_region->next_region;
        void* retval = arena->current_region->current_offset;
        arena->current_region->current_offset += arena->block_size;
        arena->free_blocks--;
        memzero(retval, arena->block_size);
        return retval;
}

error_t
slab_free(struct slab_alloc* arena, void* obj)
{
        ASSERT(arena != NULL);
        ASSERT(obj != NULL);

        struct slab_block* new_block = obj;
        new_block->next_block = arena->block_list;
        arena->block_list = new_block;
        arena->free_blocks++;
        return EC_SUCCESS;
}