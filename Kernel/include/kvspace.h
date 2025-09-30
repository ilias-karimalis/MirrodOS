#pragma once

#include <stddef.h>
#include <types/number.h>

/// Use the higher half direct mapping to translate a physical address to a kernel virtual address.
void*
kernel_hhdm_phys_to_virt(u64 phys_addr);

/// Use the higher half direct mapping to translate a virtual address to  a kernel physical address.
/// This will only work if the ptr was in the hhdm region.
u64
kernel_hhdm_virt_to_phys(void* ptr);
