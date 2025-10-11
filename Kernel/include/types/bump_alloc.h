#pragma once

#include <types/error.h>
#include <types/number.h>

struct bump_alloc_region
{
        u8* offset;
        u8* end;
        struct bump_alloc_region* next;
};

struct bump_alloc
{
        /// Linked list of regions, sorted by the amount of space left over in each region. This sorting is only
        /// triggered when an allocation fails to fit in the first region.
        ///
        /// New regions are pushed straight to the front of the list.
        struct bump_alloc_region* current;
        struct bump_alloc_region* used_list;
};

void
bump_initialize(struct bump_alloc* bump);

void
bump_grow(struct bump_alloc* bump, void* buffer, size_t buffer_size);

void*
bump_allocate(struct bump_alloc* bump, size_t size);

void*
bump_allocate_aligned(struct bump_alloc* bump, size_t size, size_t alignment);

error_t
bump_destroy(struct bump_alloc* bump);
