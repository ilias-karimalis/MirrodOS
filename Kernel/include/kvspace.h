#pragma once

#include <stddef.h>
#include <types/number.h>

struct allocation
{
        void* buffer;
        size_t size;
};

/// Use the higher half direct mapping to translate a physical address to a kernel virtual address.
void*
kernel_hhdm_phys_to_virt(u64 phys_addr);

/// Use the higher half direct mapping to translate a virtual address to  a kernel physical address.
/// This will only work if the ptr was in the hhdm region.
u64
kernel_hhdm_virt_to_phys(void* ptr);

/// Allocate a physically contiguous region of memory of `size` bytes, aligned to `alignment` bytes. The size must be a
/// multiple of the page size, and the alignment must be a power of two multiple of the page size.
struct allocation
kalloc(size_t size, size_t alignment);

struct allocation
kalloc_array(size_t count, size_t size);

struct allocation
krealloc(struct allocation old, size_t new_size, size_t alignment);

struct allocation
krealloc_array(struct allocation old, size_t count, size_t size);

/// Returns a pointer to a previously allocated region of memory to the physical memory manager.
void
kfree(struct allocation region);
