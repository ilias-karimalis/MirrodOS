/// Platform Info extracted from Limine
#pragma once

#include <limine/limine.h>
#include <types/error.h>
#include <types/number.h>

extern struct platform_info
{
        /// Framebuffers
        struct limine_framebuffer_response* framebuffer_response;
        /// Paging mode
        struct limine_paging_mode_response* paging_mode_response;
        /// Memory map
        struct limine_memmap_response* memmap_response;
        /// Device tree blob
        struct limine_dtb_response* dtb_response;
        /// Higher half direct mapping base
        struct limine_hhdm_response* hhdm_response;
        size_t hhdm_offset;
} pinfo;

void
populate_platform_info(void);