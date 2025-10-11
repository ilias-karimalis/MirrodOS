#include <limine/limine.h>
#include <limine/platform_info.h>
#include <types/error.h>

#define LIMINE_REQ __attribute__((used, section(".limine_requests")))
#define LIMINE_START __attribute__((used, section(".limine_requests_start")))
#define LIMINE_END __attribute__((used, section(".limine_requests_end")))

/// Start marker for the limine request section
LIMINE_START volatile LIMINE_REQUESTS_START_MARKER;
LIMINE_REQ volatile LIMINE_BASE_REVISION(0);

/// Framebuffer request
LIMINE_REQ volatile struct limine_framebuffer_request framebuffer_request = {
        .id = LIMINE_FRAMEBUFFER_REQUEST,
        .response = NULL,
};

/// Paging mode request
LIMINE_REQ volatile struct limine_paging_mode_request paging_mode_request = { .id = LIMINE_PAGING_MODE_REQUEST,
                                                                              .response = NULL,
                                                                              .mode = LIMINE_PAGING_MODE_RISCV_SV39 };

/// Memory map request
LIMINE_REQ volatile struct limine_memmap_request mem_map_request = {
        .id = LIMINE_MEMMAP_REQUEST,
        .response = NULL,
};

/// Device tree blob request
LIMINE_REQ volatile struct limine_dtb_request dtb_request = {
        .id = LIMINE_DTB_REQUEST,
        .response = NULL,
};

/// Higher half direct mapping request
LIMINE_REQ volatile struct limine_hhdm_request hhdm_request = {
        .id = LIMINE_HHDM_REQUEST,
        .revision = 0,
        .response = NULL,
};

/// RISCV BSP Hart ID request
LIMINE_REQ volatile struct limine_riscv_bsp_hartid_request bsp_hartid_req = {
        .id = LIMINE_RISCV_BSP_HARTID_REQUEST,
        .revision = 0,
        .response = NULL,
};

/// End marker for the limine request section
LIMINE_END volatile LIMINE_REQUESTS_END_MARKER;

/// The platform info filled with limine info.
struct platform_info pinfo = { 0 };

void
populate_platform_info(void)
{
        pinfo.framebuffer_response = framebuffer_request.response;
        pinfo.paging_mode_response = paging_mode_request.response;
        pinfo.memmap_response = mem_map_request.response;
        pinfo.dtb_response = dtb_request.response;
        pinfo.hhdm_response = hhdm_request.response;
        pinfo.hhdm_offset = pinfo.hhdm_response->offset;
        pinfo.bsp_hartid_response = bsp_hartid_req.response;
        pinfo.bsp_hartid = pinfo.bsp_hartid_response->bsp_hartid;
}