#pragma once

#include <kvspace.h>
#include <types/error.h>
#include <types/number.h>
#include <types/slab.h>

struct device;

struct plic_driver
{
        u32 hartid;
        volatile u32* ctxt_threshold;
        volatile u32* ctxt_claim;
        volatile u32* ctxt_interrupt_enable;
        volatile u32* ctxt_interrupt_priority;
        struct driver** driver_map;
};

// struct driver_map_node
// {
//         u32 interrupt;
//         struct driver* driver;
//         struct driver_map_node* next;
// };

void
plic_driver_init(struct plic_driver* driver, void* base, u32 hartid);

error_t
plic_driver_handle_interrupt(struct plic_driver* driver);

void
plic_driver_set_thresh(struct plic_driver* driver, u32 threshold);

void
plic_driver_enable_int(struct plic_driver* plic, u32 interrupt, u8 priority, struct driver* driver);

// void
// plic_driver_set_int_prio(struct plic_driver* plic, u32 interrupt, u8 priority);