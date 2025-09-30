#pragma once

#include <types/error.h>
#include <types/number.h>
enum pmm_policy
{
        PMM_POLICY_FIRST_FIT,
        PMM_POLICY_BEST_FIT,
        PMM_POLICY_WORST_FIT,
        PMM_POLICY_NEXT_FIT,
};

void
pmm_initialize(enum pmm_policy pol);

/// Adds a new contiguous memory region to the physical memory manager.
error_t
pmm_add_region(u64 region_base, size_t region_size);

/// Allocates a region from the physical memory manager with the requested size and alignment.
error_t
pmm_alloc_aligned(size_t size, size_t alignment, paddr_t* region);

/// Allocates a region from the physical memory manager with the requested size and alignment, if
/// there's an error, it will return NULL instead.
paddr_t
pmm_alloc_aligned_noerr(size_t size, size_t alignment);

/// Allocates a region from the physical memory manager with the requsted size and PAGE_SIZE
/// alignment.
error_t
pmm_alloc(size_t size, paddr_t* region);

/// Allocates a region from the physical memory manager with the requsted size and PAGE_SIZE
/// alignment, if there's an error, it will return NULL instead.
paddr_t
pmm_alloc_noerr(size_t size);

/// Frees a previously allocated region back to the physical memory manager.
error_t
pmm_free(paddr_t region);

/// Returns the total amount of free memory managed by the physical memory manager.
size_t pmm_free_memory(void);

/// Returns the total amount of memory managed by the physical memory manager.
size_t pmm_total_memory(void);