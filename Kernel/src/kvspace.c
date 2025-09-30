#include <kvspace.h>
#include <limine/platfrom_info.h>

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