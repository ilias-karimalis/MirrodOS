#include <assert.h>
#include <devices/device_tree/blob.h>
#include <fmt/print.h>
#include <kvspace.h>
#include <limine/platfrom_info.h>
#include <pmm.h>
#include <types/error.h>
#include <uart.h>

struct device_tree dt = { 0 };

void
kernel_c_entry()
{
        error_t err = EC_SUCCESS;
        populate_platform_info();

        uart_initialize(kernel_hhdm_phys_to_virt(0x10000000));
        kprint_initialize(&uart_put_char);

        pmm_initialize(PMM_POLICY_FIRST_FIT);
        for (size_t i = 0; i < pinfo.memmap_response->entry_count; i++) {
                struct limine_memmap_entry* entries = *pinfo.memmap_response->entries;
                if (entries[i].type == LIMINE_MEMMAP_USABLE) {
                        err = pmm_add_region(entries[i].base, entries[i].length);
                        if (error_is_err(err)) {
                                PANIC(error_string(err));
                        }
                }
        }
        kprintln(SV("PMM initialized with {X} bytes of free memory"), pmm_free_memory());
        kprintln(SV("Hello, World!"));

        err = device_tree_parse_blob(pinfo.dtb_response->dtb_ptr, 0, &dt);
        if (error_is_err(err)) {
                PANIC(error_string(err));
        }
        for (;;);
}
