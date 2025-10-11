#include <assert.h>
#include <fmt/print.h>
#include <riscv.h>
#include <types/bump_alloc.h>

void
bump_initialize(struct bump_alloc* bump)
{
        ASSERT(bump != NULL);
        bump->current = NULL;
        bump->used_list = NULL;
}

void
insert_into_sorted_list(struct bump_alloc_region** head, struct bump_alloc_region* new_region)
{
        if (new_region == NULL) {
                return;
        }

        if (*head == NULL || (*head)->end - (*head)->offset <= new_region->end - new_region->offset) {
                new_region->next = *head;
                *head = new_region;
                return;
        }

        struct bump_alloc_region* current = *head;
        while (current->next != NULL &&
               (current->next->end - current->next->offset) > (new_region->end - new_region->offset)) {
                current = current->next;
        }

        new_region->next = current->next;
        current->next = new_region;
}

void
debug_print_region_list(struct bump_alloc* bump)
{
        kprintln(SV("Current Region: Free Space = {D} bytes"), bump->current->end - bump->current->offset);
        struct bump_alloc_region* current = bump->used_list;
        int index = 0;
        while (current != NULL) {
                size_t free_space = current->end - current->offset;
                kprintln(SV("Used Region {D}: Free Space = {D} bytes"), index, free_space);
                current = current->next;
                index++;
        }
}

void
bump_grow(struct bump_alloc* bump, void* buffer, size_t buffer_size)
{
        ASSERT(bump != NULL);
        ASSERT(buffer != NULL);
        ASSERT(buffer_size >= RISCV_SV39_PAGE_SIZE);

        struct bump_alloc_region* new_region = buffer;
        buffer = (u8*)buffer + sizeof(struct bump_alloc_region);
        buffer_size -= sizeof(struct bump_alloc_region);

        new_region->offset = buffer;
        new_region->end = buffer + buffer_size;
        new_region->next = NULL;

        // Heuristically, we assume that the new region is the most empty region, and we immediately set it as the
        // current list and move the previous `current` region into the sorted used list.
        insert_into_sorted_list(&bump->used_list, bump->current);
        bump->current = new_region;
}

void*
bump_allocate_aligned(struct bump_alloc* bump, size_t size, size_t alignment)
{
        ASSERT(bump != NULL);
        if (size == 0 || bump->current == NULL) {
                return NULL;
        }

        if (ALIGN_UP(bump->current->offset, alignment) + size <= (size_t)bump->current->end) {
                u8* retval = (u8*)ALIGN_UP(bump->current->offset, alignment);
                bump->current->offset = retval + size;
                return retval;
        }

        // We start by replacing the largest used region into the current region, and inserting the previous current region into the used list.
        struct bump_alloc_region* new_current = bump->used_list;
        bump->used_list = bump->used_list->next;
        insert_into_sorted_list(&bump->used_list, bump->current);
        bump->current = new_current;

        if (ALIGN_UP(bump->current->offset, alignment) + size <= (size_t)bump->current->end) {
                u8* retval = (u8*)ALIGN_UP(bump->current->offset, alignment);
                bump->current->offset = retval + size;
                return retval;
        }

        return NULL;
}

void*
bump_allocate(struct bump_alloc* bump, size_t size)
{
        return bump_allocate_aligned(bump, size, 1);
}
