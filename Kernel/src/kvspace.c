#include <assert.h>
#include <kvspace.h>
#include <limine/platform_info.h>
#include <memory.h>
#include <pmm.h>
#include <types/error.h>

void*
kernel_hhdm_phys_to_virt(u64 phys_addr)
{
        return (void*)(pinfo.hhdm_offset + phys_addr);
}

u64
kernel_hhdm_virt_to_phys(void* ptr)
{
        return (u64)ptr - pinfo.hhdm_offset;
}

struct allocation
kalloc(size_t size, size_t alignment)
{
        paddr_t pa = 0;
        error_t err = pmm_alloc_aligned(size, alignment, &pa);
        // pa = pmm_alloc_aligned_noerr(size, alignment);
        if (error_is_err(err)) {
                PANIC(SV("kernel_alloc: {V}"), SVP(error_string(err)));
        }
        return (struct allocation){ kernel_hhdm_phys_to_virt(pa), size };
}

struct allocation
kalloc_array(size_t count, size_t size)
{
        return kalloc(count * size, size);
}

struct allocation
krealloc(struct allocation old, size_t new_size, size_t alignment)
{
        if (old.buffer == NULL) {
                return kalloc(new_size, alignment);
        }
        if (new_size == 0) {
                kfree(old);
                return (struct allocation){ .buffer = NULL, .size = 0 };
        }

        struct allocation new_alloc = kalloc(new_size, alignment);
        if (new_alloc.buffer == NULL) {
                return (struct allocation){ .buffer = NULL, .size = 0 };
        }
        size_t copy_size = old.size < new_size ? old.size : new_size;
        memcopy(new_alloc.buffer, old.buffer, copy_size);
        kfree(old);
        return new_alloc;
}

void
kfree(struct allocation region)
{
        paddr_t pa = kernel_hhdm_virt_to_phys(region.buffer);
        error_t err = pmm_free(pa);
        if (error_is_err(err)) {
                PANIC(SV("kernel_free: Failed to free memory: {X}"), error_string(err));
        }
}