#include "riscv.h"
#include "types/error.h"
#include "types/slab.h"
#include <assert.h>
#include <kvspace.h>
#include <pmm.h>
#include <vmm.h>

error_t
vspace_initialize(struct vspace* vspace)
{
        ASSERT(vspace != NULL);

        /// Allocate and initialize the root page table.
        paddr_t pt_pa;
        error_t err = pmm_alloc(RISCV_SV39_PAGE_SIZE, &pt_pa);
        if (error_is_err(err)) {
                return error_push(err, EC_VSPACE_INIT_FAILURE);
        }
        vspace->page_table = kernel_hhdm_phys_to_virt(pt_pa);
        riscv_sv39_pt_initialize(vspace->page_table);

        /// Initialize the slab allocator for virtual regions and setup the first one to cover the entire
        /// addressable space.
        slab_autorefill_init(&vspace->region_alloc, sizeof(struct virtual_region));

        // Adding in two "allocated" regions that cover the 0-1Mb hole that we're leaving empty and the upper half
        // kernel.
        // Region 1: 0x0000000000000000 - 0x0000000000100000 (0 - 1MB)
        // Region 2: 0xffffffc000000000 - 0xffffffffffffffff (Upper half kernel space)
        struct virtual_region* early_mem = slab_allocate(&vspace->region_alloc);
        if (early_mem == NULL) {
                return EC_VSPACE_INIT_FAILURE;
        }
        early_mem->base = 0x0000000000000000;
        early_mem->size = 0x0000000000100000;
        early_mem->flags = 0;
        early_mem->mappings = NULL;

        struct virtual_region* kernel_mem = slab_allocate(&vspace->region_alloc);
        if (kernel_mem == NULL) {
                return EC_VSPACE_INIT_FAILURE;
        }
        kernel_mem->base = 0xffffffc000000000;
        kernel_mem->size = 0x4000000000;
        kernel_mem->flags = 0;
        kernel_mem->mappings = NULL;
        kernel_mem->next = NULL;
        early_mem->next = kernel_mem;
        return EC_SUCCESS;
}

error_t
vspace_allocate_fixed(struct vspace* vspace, vaddr_t vaddr, size_t size, u64 flags)
{
        // TODO: We've changed the semantics of the region list, adding the early and kernel regions, which are
        // unmappable, so we need to adjust this function to account for that.
        vaddr_t alloc_base = vaddr;
        vaddr_t alloc_end = vaddr + size;

        // Check if the requested region is already allocated.
        struct virtual_region* prev = NULL;
        struct virtual_region* curr = vspace->regions;
        while (curr != NULL) {
                vaddr_t region_base = curr->base;
                vaddr_t region_end = curr->base + curr->size;
                if (region_base <= alloc_base && alloc_end <= region_end) {
                        return EC_VSPACE_REGION_ALREADY_ALLOCATED;
                } else if (alloc_end <= region_base) {
                        // No overlap, and the requested region is before the current region.
                        struct virtual_region* new_region = slab_allocate(&vspace->region_alloc);
                        if (new_region == NULL) {
                                return EC_VSPACE_REGION_ALLOC_FAILED;
                        }
                        new_region->base = alloc_base;
                        new_region->size = size;
                        new_region->flags = flags;
                        new_region->mappings = NULL;
                        new_region->next = curr;
                        if (prev == NULL) {
                                vspace->regions = new_region;
                        } else {
                                prev->next = new_region;
                        }
                        return EC_SUCCESS;
                }
                prev = curr;
                curr = curr->next;
        }

        struct virtual_region* new_region = slab_allocate(&vspace->region_alloc);
        if (new_region == NULL) {
                return EC_VSPACE_REGION_ALREADY_ALLOCATED;
        }
        new_region->base = alloc_base;
        new_region->size = size;
        new_region->flags = flags;
        new_region->mappings = NULL;
        new_region->next = NULL;
        if (prev != NULL) {
                prev->next = new_region;
        } else {
                vspace->regions = new_region;
        }
        return EC_SUCCESS;
}

error_t
vspace_allocate(struct vspace* vspace, vaddr_t* vaddr, size_t size, size_t alignment, u64 flags)
{
        // Instead of having a specific virtual address we want to allocate, we now just want to find a gap in the
        // address space that fits our size and alignment requirements.
        if (alignment % RISCV_SV39_PAGE_SIZE != 0) {
                return EC_VSPACE_BAD_ALIGNMENT;
        }
        size_t rounded_size = ALIGN_UP(size, RISCV_SV39_PAGE_SIZE);

        struct virtual_region *first = vspace->regions, *second = vspace->regions->next;
        while (second != NULL) {
                size_t aligned_base = ALIGN_UP(first->base + first->size, alignment);
                if (second->base - aligned_base >= rounded_size) {
                        // We found a gap that fits the requested size and alignment.
                        struct virtual_region* new_region = slab_allocate(&vspace->region_alloc);
                        if (new_region == NULL) {
                                return EC_VSPACE_REGION_ALLOC_FAILED;
                        }
                        new_region->base = aligned_base;
                        new_region->size = rounded_size;
                        new_region->flags = flags;
                        new_region->mappings = NULL;
                        new_region->next = second;
                        first->next = new_region;
                        *vaddr = aligned_base;
                        return EC_SUCCESS;
                }
                first = second;
                second = second->next;
        }

        return EC_VSPACE_REGION_ALLOC_FAILED;
}

error_t
vspace_allocmap(struct vspace* vspace, vaddr_t* vaddr, size_t size, size_t alignment, paddr_t* paddr, u64 flags)
{
        // First, allocate a virtual region.
        vaddr_t va;
        error_t err = vspace_allocate(vspace, &va, size, alignment, flags);
        if (error_is_err(err)) {
                return error_push(err, EC_VSPACE_REGION_ALLOC_FAILED);
        }

        // Next, allocate physical pages and map them.
        size_t rounded_size = ALIGN_UP(size, RISCV_SV39_PAGE_SIZE);
        paddr_t pa;
        err = pmm_alloc_aligned(rounded_size, alignment, &pa);
        if (error_is_err(err)) {
                return error_push(err, EC_VSPACE_REGION_MAP_FAILED);
        }
        err = vspace_map_fixed(vspace, va, pa, size, flags);
        if (error_is_err(err)) {
                return error_push(err, EC_VSPACE_REGION_MAP_FAILED);
        }

        *vaddr = va;
        *paddr = pa;
        return EC_SUCCESS;
}

error_t
vspace_map_fixed(struct vspace* vspace, vaddr_t va, paddr_t pa, size_t size, u64 flags)
{
        size_t rounded_size = ALIGN_UP(size, RISCV_SV39_PAGE_SIZE);
        // Assume that the region has already been allocated already.
        for (size_t offset = 0; offset < rounded_size; offset += RISCV_SV39_PAGE_SIZE) {
                error_t err = riscv_sv39_map_small_page(vspace->page_table, va + offset, pa + offset, flags);
                if (error_is_err(err)) {
                        return error_push(err, EC_VSPACE_REGION_MAP_FAILED);
                }
        }
        return EC_SUCCESS;
}

error_t
vspace_allocmap_fixed(struct vspace* vspace, vaddr_t va, paddr_t pa, size_t size, u64 flags)
{
        error_t err = vspace_allocate_fixed(vspace, va, size, flags);
        if (error_is_err(err)) {
                return error_push(err, EC_VSPACE_REGION_ALLOC_FAILED);
        }
        err = vspace_map_fixed(vspace, va, pa, size, flags);
        if (error_is_err(err)) {
                return error_push(err, EC_VSPACE_REGION_MAP_FAILED);
        }

        return EC_NOT_IMPLEMENTED;
}
