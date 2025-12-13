// Virtual Memory Manager, creates and manages the address space for a processs
#pragma once

#include <riscv.h>
#include <types/error.h>
#include <types/slab.h>

struct mapped_region
{
        vaddr_t base;
        size_t size;
        paddr_t phys_base;
        struct mapped_region* next;
};

struct virtual_region
{
        vaddr_t base;
        size_t size;
        u64 flags;
        struct mapped_region* mappings;
        struct virtual_region* next;
};

// What does this structure need to track:
// 1. Allcoated regions in the address space (all must be page aligned)
// 2. Which of the allocated regions are mapped to physical memory, and what phuscal pages are they mapped to.
struct vspace
{
        /// The root page table for this address space.
        struct riscv_sv39_pt* page_table;
        /// Slab allocator for virtual regions.
        struct slab_alloc region_alloc;
        /// Linked list of allocated virtual regions.
        struct virtual_region* regions;
};

error_t
vspace_initialize(struct vspace* vspace);

error_t
vspace_allocate_fixed(struct vspace* vspace, vaddr_t vaddr, size_t size, u64 flags);

error_t
vspace_allocate(struct vspace* vspace, vaddr_t* vaddr, size_t size, size_t alignment, u64 flags);

error_t
vspace_allocmap(struct vspace* vspace, vaddr_t* vaddr, size_t size, size_t alignment, paddr_t* paddr, u64 flags);

error_t
vspace_map_fixed(struct vspace* vspace, vaddr_t va, paddr_t pa, size_t size, u64 flags);

error_t
vspace_allocmap_fixed(struct vspace* vspace, vaddr_t va, paddr_t pa, size_t size, u64 flags);
