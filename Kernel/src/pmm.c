#include "fmt/print.h"
#include <assert.h>
#include <kvspace.h>
#include <memory.h>
#include <pmm.h>
#include <riscv.h>
#include <stdalign.h>
#include <types/error.h>
#include <types/number.h>
#include <types/slab.h>

struct pmm_memory_block
{
        u64 block_base;
        size_t block_size;
        struct pmm_memory_block* next;
};

struct pmm_memory_region
{
        u64 region_base;
        size_t region_size;
        size_t free_bytes;
        struct pmm_memory_block* free_blocks;
};

#define REGION_COUNT 32

enum pmm_policy policy = PMM_POLICY_FIRST_FIT;
size_t total_bytes = 0;
size_t free_bytes = 0;
size_t region_count = 0;
struct pmm_memory_region regions[REGION_COUNT] = { 0 };
struct slab_alloc block_arena = { 0 };

/// Initial slab buffer
#define INITIAL_BUF_SIZE SLAB_REGION_SIZE(sizeof(struct pmm_memory_block), 100)
alignas(SLAB_REGION_ALIGN) u8 initial_buf[INITIAL_BUF_SIZE] = {};

void
pmm_initialize(enum pmm_policy pol)
{
        policy = pol;
        slab_init(&block_arena, sizeof(struct pmm_memory_block));
        slab_grow(&block_arena, initial_buf, INITIAL_BUF_SIZE);
}

error_t
pmm_add_region(u64 region_base, size_t region_size)
{
        if (region_count >= REGION_COUNT) {
                return EC_PMM_REGION_LIST_FULL;
        }

        size_t aligned_base = ALIGN_UP(region_base, RISCV_SV39_PAGE_SIZE);
        size_t aligned_size = ALIGN_DOWN(region_size - (aligned_base - region_base), RISCV_SV39_PAGE_SIZE);
        bool ALIGNED_REGION_FITS = aligned_base + aligned_size <= region_base + region_size;
        bool NEW_SIZE_NON_ZERO = aligned_size >= RISCV_SV39_PAGE_SIZE;
        if (!ALIGNED_REGION_FITS || !NEW_SIZE_NON_ZERO) {
                return EC_PMM_REGION_TOO_SMALL;
        }

        for (size_t i = 0; i < region_count; i++) {
                bool LOWER_BOUND = regions[i].region_base <= aligned_base;
                bool UPPER_BOUND = aligned_base <= regions[i].region_base + regions[i].region_size;
                if (LOWER_BOUND && UPPER_BOUND) {
                        return EC_PMM_REGION_ALREADY_MANAGED;
                }
        }

        struct pmm_memory_block* free_block = slab_allocate(&block_arena);
        ASSERT(free_block != NULL);
        regions[region_count].region_base = aligned_base;
        regions[region_count].region_size = aligned_size;
        regions[region_count].free_bytes = aligned_size;
        regions[region_count].free_blocks = free_block;
        free_block->block_base = aligned_base;
        free_block->block_size = aligned_size;
        free_block->next = NULL;
        region_count++;

        total_bytes += aligned_size;
        free_bytes += aligned_size;
        return EC_SUCCESS;
}

error_t
pmm_alloc_aligned(size_t size, size_t alignment, paddr_t* region)
{
        size_t aligned_size = ALIGN_UP(size, RISCV_SV39_PAGE_SIZE);
        if (region == NULL) {
                return EC_NULL_ARGUMENT;
        }
        if (alignment < RISCV_SV39_PAGE_SIZE || (alignment & (alignment - 1)) != 0) {
                *region = 0;
                return EC_PMM_BAD_ALIGNMENT;
        }
        if (free_bytes < aligned_size) {
                *region = 0;
                return EC_PMM_OUT_OF_MEMORY;
        }
        if (block_arena.free_blocks < 16) {
                TODO("struct pmm_memory_block arena allocator is out of memory. Need to implement "
                     "refilling.");
        }
        if (policy != PMM_POLICY_FIRST_FIT) {
                TODO("Chosen policy is not implemented.");
        }

        for (size_t i = 0; i < region_count; i++) {
                if (regions[i].free_bytes < aligned_size) {
                        continue;
                }

                struct pmm_memory_block* curr = regions[i].free_blocks;
                struct pmm_memory_block* prev = NULL;
                while (curr != NULL) {
                        size_t curr_base = curr->block_base;
                        size_t curr_size = curr->block_size;
                        size_t aligned_base = ALIGN_UP(curr_base, alignment);
                        if (curr_base + curr_size < aligned_base + aligned_size) {
                                prev = curr;
                                curr = curr->next;
                                continue;
                        }

                        size_t offset = aligned_base - curr_base;
                        bool EXISTS_PRECEEDING = curr_base != aligned_base;
                        bool EXISTS_POSTCEEDING = curr_base + curr_size > aligned_base + aligned_size;
                        if (EXISTS_PRECEEDING && EXISTS_POSTCEEDING) {
                                curr->block_size = offset;
                                struct pmm_memory_block* extra = slab_allocate(&block_arena);
                                ASSERT(extra != NULL);
                                extra->block_base = aligned_base + aligned_size;
                                extra->block_size = curr_base + curr_size - (aligned_base + aligned_size);
                                extra->next = curr->next;
                        } else if (EXISTS_PRECEEDING) {
                                curr->block_size = offset;
                        } else if (EXISTS_POSTCEEDING) {
                                curr->block_base = aligned_base + aligned_size;
                                curr->block_size = curr_base + curr->block_size - (aligned_base + aligned_size);
                        } else if (prev == NULL) {
                                regions[i].free_blocks = curr->next;
                                error_t err = slab_free(&block_arena, curr);
                                ASSERT(error_is_ok(err));
                        } else {
                                prev->next = curr->next;
                                error_t err = slab_free(&block_arena, curr);
                                ASSERT(error_is_ok(err));
                        }

                        regions[i].free_bytes -= aligned_size;
                        free_bytes -= aligned_size;
                        *region = aligned_base;
                        memzero(kernel_hhdm_phys_to_virt(*region), aligned_size);
                        return EC_SUCCESS;
                }
        }

        *region = 0;
        return EC_PMM_OUT_OF_MEMORY;
}

paddr_t
pmm_alloc_aligned_noerr(size_t size, size_t alignment)
{
        paddr_t region = 0;
        error_t err = pmm_alloc_aligned(size, alignment, &region);
        if (err != EC_SUCCESS) {
                return 0;
        }
        return region;
}

error_t
pmm_alloc(size_t size, paddr_t* region)
{
        return pmm_alloc_aligned(size, RISCV_SV39_PAGE_SIZE, region);
}

paddr_t
pmm_alloc_noerr(size_t size)
{
        return pmm_alloc_aligned_noerr(size, RISCV_SV39_PAGE_SIZE);
}

error_t
pmm_free(paddr_t region)
{
        return EC_NOT_IMPLEMENTED;
}

size_t
pmm_free_memory(void)
{
        return free_bytes;
}

size_t
pmm_total_memory(void)
{
        return total_bytes;
}