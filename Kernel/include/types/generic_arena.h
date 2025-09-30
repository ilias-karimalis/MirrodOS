#pragma once

#include <types/error.h>
#include <types/number.h>

struct generic_arena_block
{
        struct generic_arena_block* next_block;
};

struct generic_arena_region
{
        u8* current_offset;
        u8* region_end;
        struct generic_arena_region* previous_region;
        struct generic_arena_region* next_region;
};

struct generic_arena
{
        u64 block_size;
        u64 free_blocks;
        u64 total_blocks;
        /// gereric_arena operations maintain an invariant where previous_regions of current_region
        /// are fully allocated out whereas regions with free slots left are added as a next_region.
        struct generic_arena_region* current_region;
        struct generic_arena_block* block_list;
};

#define GA_BLOCK_BASE_SIZE sizeof(struct generic_arena_block)
#define GA_BLOCK_SIZE(objsize) ((objsize) > GA_BLOCK_BASE_SIZE ? (objsize) : GA_BLOCK_BASE_SIZE)
#define GA_REGION_SIZE(objsize, count)                                                                                 \
        (ALIGN_UP(sizeof(struct generic_arena_region), GA_BLOCK_SIZE(objsize)) + GA_BLOCK_SIZE(objsize) * (count))
#define GA_REGION_ALIGN alignof(struct generic_arena_region)

void
generic_arena_initialize(struct generic_arena* arena, size_t objsize);

void
generic_arena_grow(struct generic_arena* arena, void* buffer, size_t buffer_size);

void*
generic_arena_alloc(struct generic_arena* arena);

error_t
generic_arena_free(struct generic_arena* arena, void* obj);