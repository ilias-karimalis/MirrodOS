#include <assert.h>
#include <devices/device.h>
#include <devices/device_tree/blob.h>
#include <fmt/print.h>
#include <kvspace.h>
#include <limine/platform_info.h>
#include <pmm.h>
#include <riscv.h>
#include <trap.h>
#include <types/bump_alloc.h>
#include <types/error.h>
#include <types/number.h>
#include <uart.h>

struct device_tree dt = { 0 };
struct trap_frame kernel_trap_frame = { 0 };
struct riscv_sv39_pt* kernel_page_table = NULL;

// Assembly trap handler entry point
extern void
kernel_asm_trap_handler(void);

void
kernel_c_entry()
{
        error_t err = EC_SUCCESS;
        populate_platform_info();

        uart_initialize(kernel_hhdm_phys_to_virt(0x10000000));
        kprint_initialize(&uart_put_char);
        kprintln(SV("Hello, World!"));

        paddr_t kernel_pt_paddr = riscv_satp_read() << 12;
        kernel_page_table = kernel_hhdm_phys_to_virt(kernel_pt_paddr);

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
        kprintln(SV("PMM initialized with {X} bytes of free memory."), pmm_free_memory());

        // Parse the device tree blob structure.
        err = device_tree_parse_blob(pinfo.dtb_response->dtb_ptr, &dt);
        if (error_is_err(err)) {
                PANIC(error_string(err));
        }
        kprintln(SV("Device tree blob parsed."));

        // Initialize device drivers based on the device tree.
        devices_init(&dt, pinfo.bsp_hartid);
        kprintln(SV("Device driver initialization complete."));

        // Initialize the interrupt system.
        struct allocation tf_stack_alloc = kalloc(4 * RISCV_SV39_PAGE_SIZE, RISCV_SV39_PAGE_SIZE);
        kernel_trap_frame.trap_stack = (u8*)tf_stack_alloc.buffer + tf_stack_alloc.size;
        kernel_trap_frame.stack_allocation = tf_stack_alloc;
        kernel_trap_frame.satp = riscv_satp_read();
        kernel_trap_frame.hartid = pinfo.bsp_hartid;
        riscv_sscratch_write((u64)&kernel_trap_frame);
        paddr_t trap_vector_pa = riscv_sv39_virt_to_phys(kernel_page_table, (vaddr_t)&kernel_asm_trap_handler);
        riscv_stvec_write(trap_vector_pa);
        riscv_stimecmp_write(riscv_time() + 10000000);
        riscv_sie_write((1UL << 1) | // SSIE - Software interrupts
                        (1UL << 5) | // STIE - Timer interrupts
                        (1UL << 9)); // SEIE - External interrupts
        riscv_sstatus_write(riscv_sstatus_read() | (1UL << 1));
        kprintln(SV("Core local interrupt system initialized."));

        kprintln(SV("Entering wait loop."));
        for (;;);
}
