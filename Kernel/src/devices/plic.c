#include <assert.h>
#include <devices/device.h>
#include <devices/plic.h>
#include <memory.h>
#include <riscv.h>

#define CONTEXT(hart) ((hart) * 2 + 1)

#define PLIC_PRIORITY 0x0
#define PLIC_PENDING 0x1000
#define PLIC_INTERRUPT_ENABLE 0x2000
#define PLIC_THRESHOLD 0x200000
#define PLIC_CLAIM_COMPLETE 0x200004

void
plic_driver_init(struct plic_driver* driver, void* base, u32 hartid)
{
        driver->hartid = hartid;
        driver->ctxt_threshold = (u32*)(base + PLIC_THRESHOLD + CONTEXT(hartid) * 0x1000);
        driver->ctxt_claim = driver->ctxt_threshold + 1;
        driver->ctxt_interrupt_enable = (u32*)(base + PLIC_INTERRUPT_ENABLE + CONTEXT(hartid) * 0x80);
        driver->ctxt_interrupt_priority = (u32*)(base + PLIC_PRIORITY);
        driver->driver_map = kalloc(sizeof(struct driver*) * 1024, RISCV_SV39_PAGE_SIZE).buffer;
        memzero(driver->driver_map, sizeof(struct driver*) * 1024);
        *driver->ctxt_threshold = 0;
}

error_t
plic_driver_handle_interrupt(struct plic_driver* plic)
{
        u32 claim = *plic->ctxt_claim;
        if (claim == 0) {
                return EC_PLIC_NO_INTERRUPT;
        }
        ASSERT(claim < 1024);

        if (plic->driver_map[claim] != NULL) {
                struct driver* dev = plic->driver_map[claim];
                switch (dev->type) {
                        case DEVICE_TYPE_UART:
                                dev->d.uart.handle_interrupt(&dev->d.uart);
                                break;
                        default:
                                return EC_PLIC_UNREGISTERED_DRIVER;
                }
                *plic->ctxt_claim = claim;
                return EC_SUCCESS;
        }

        *plic->ctxt_claim = claim;
        return EC_PLIC_UNREGISTERED_INTERRUPT;
}

void
plic_driver_set_thresh(struct plic_driver* driver, u32 threshold)
{
        *driver->ctxt_threshold = threshold;
}

void
plic_driver_enable_int(struct plic_driver* plic, u32 interrupt, u8 priority, struct driver* driver)
{
        // For now we only handle the first 32 interrupts.
        ASSERT(interrupt < 32);
        priority &= 0x7;
        plic->driver_map[interrupt] = driver;
        *plic->ctxt_interrupt_enable |= (1 << interrupt);
        plic->ctxt_interrupt_priority[interrupt] = priority;
}

void
plic_interrupt_set_priority(struct plic_driver* plic, u32 interrupt, u8 priority)
{
        ASSERT(interrupt < 32);
        priority &= 0x7;
        plic->ctxt_interrupt_priority[interrupt] = priority;
}