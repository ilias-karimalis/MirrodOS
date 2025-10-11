#pragma once

#include <stdalign.h>
#include <types/error.h>
#include <types/number.h>

struct slab_block
{
        struct slab_block* next_block;
};

struct slab_region
{
        u8* current_offset;
        u8* region_end;
        struct slab_region* previous_region;
        struct slab_region* next_region;
};

struct slab_alloc
{
        u64 block_size;
        u64 free_blocks;
        u64 total_blocks;
        /// slab operations maintain an invariant where previous_regions of current_region
        /// are fully allocated out whereas regions with free slots left are added as a next_region.
        struct slab_region* current_region;
        struct slab_block* block_list;
        bool auto_refill;
};

#define SLAB_BLOCK_BASE_SIZE sizeof(struct slab_block)
#define SLAB_BLOCK_SIZE(objsize) ((objsize) > SLAB_BLOCK_BASE_SIZE ? (objsize) : SLAB_BLOCK_BASE_SIZE)
#define SLAB_REGION_SIZE(objsize, count)                                                                               \
        (ALIGN_UP(sizeof(struct slab_region), SLAB_BLOCK_SIZE(objsize)) + SLAB_BLOCK_SIZE(objsize) * (count))
#define SLAB_REGION_ALIGN (alignof(*(struct slab_region*)(0)))

void
slab_init(struct slab_alloc* arena, size_t objsize);

void
slab_autorefill_init(struct slab_alloc* arena, size_t objsize);

void
slab_grow(struct slab_alloc* arena, void* buffer, size_t buffer_size);

void*
slab_allocate(struct slab_alloc* arena);

error_t
slab_free(struct slab_alloc* arena, void* obj);