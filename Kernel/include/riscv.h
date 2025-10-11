#pragma once

#include <stdbool.h>
#include <types/error.h>
#include <types/number.h>

#define RISCV_SV39_PAGE_SIZE 0x1000
#define RISCV_SV39_MEGAPAGE_SIZE 0x200000
#define RISCV_SV39_GIGAPAGE_SIZE 0x40000000

#define RISCV_SV39_PT_ENTRY_COUNT 512

enum riscv_sv39_pt_flags
{
        RISCV_SV39_PTFLAG_VALID = 0x1,
        RISCV_SV39_PTFLAG_READ = 0x2,
        RISCV_SV39_PTFLAG_WRITE = 0x4,
        RISCV_SV39_PTFLAG_EXECUTE = 0x8,
        RISCV_SV39_PTFLAG_USER = 0x10,
        RISCV_SV39_PTFLAG_GLOBAL = 0x20,
        RISCV_SV39_PTFLAG_ACCESSED = 0x40,
        RISCV_SV39_PTFLAG_DIRTY = 0x80,
};

///  Creates a page table entry from the given physical address and flags.
static inline u64
riscv_sv39_create_pte(paddr_t pa, u64 flags)
{
        return (pa >> 12) << 10 | flags;
}

/// Returns the physical address to the next level page table or leaf page this pte points to.
static inline paddr_t
riscv_sv39_pte_get_address(u64 pte)
{
        return (pte >> 10) << 12;
}

/// Returns true if this entry is valid.
static inline bool
riscv_sv39_pte_valid(u64 pte)
{
        return (pte & RISCV_SV39_PTFLAG_VALID) != 0;
}

/// Returns true if this entry is readable
static inline bool
riscv_sv39_pte_readable(u64 pte)
{
        return (pte & RISCV_SV39_PTFLAG_READ) != 0;
}

/// Returns true if this entry is writable
static inline bool
riscv_sv39_pte_writable(u64 pte)
{
        return (pte & RISCV_SV39_PTFLAG_WRITE) != 0;
}

/// Returns true if this entry is executable
static inline bool
riscv_sv39_pte_executable(u64 pte)
{
        return (pte & RISCV_SV39_PTFLAG_EXECUTE) != 0;
}

/// Returns true if the entry points to a leaf (a page) and not to another page table.
static inline bool
riscv_sv39_pte_leaf(u64 pte)
{
        return (pte & (RISCV_SV39_PTFLAG_READ | RISCV_SV39_PTFLAG_WRITE | RISCV_SV39_PTFLAG_EXECUTE)) != 0;
}

struct riscv_sv39_pt
{
        u64 entries[RISCV_SV39_PT_ENTRY_COUNT];
};

/// Maps a page into the given page table. If any level of the page table doesn't exist, then it is created.
error_t
riscv_sv39_map_small_page(struct riscv_sv39_pt* root, vaddr_t va, paddr_t pa, u64 flags);

/// Maps a mega page into the given page table. If any level of the page table doesn't exist, then it is created.
error_t
riscv_sv39_map_megapage(struct riscv_sv39_pt* root, vaddr_t va, paddr_t pa, u64 flags);

/// Maps a giga page into the given page table. If any level of the page table doesn't exist, then it is created.
error_t
riscv_sv39_map_gigapage(struct riscv_sv39_pt* root, vaddr_t va, paddr_t pa, u64 flags);

/// Unmaps a page from the given page table. If any level of the page table doesn't exist, then an error is returned.
error_t
riscv_sv39_unmap_small_page(struct riscv_sv39_pt* root, vaddr_t va, paddr_t* pa);

/// Converts a virtual address to a physical address using the given page table. If no mapping exists, then 0 is
/// returned.
paddr_t
riscv_sv39_virt_to_phys(struct riscv_sv39_pt* root, vaddr_t va);

// ===================================================================================================
// Machine-level CSR Functions
// ===================================================================================================

/// Reads the current time from the `time` CSR register.
static inline u64
riscv_time(void)
{
        u64 value;
        __asm__ volatile("csrr %0, time" : "=r"(value));
        return value;
}

/// Writes the given value to the `mstatus` CSR register.
static inline void
riscv_mstatus_write(u64 value)
{
        __asm__ volatile("csrw mstatus, %0" ::"r"(value));
}

/// Reads the value of the `mstatus` CSR register.
static inline u64
riscv_mstatus_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, mstatus" : "=r"(value));
        return value;
}

/// Writes the given value to the `mtvec` CSR register.
static inline void
riscv_mtvec_write(u64 value)
{
        __asm__ volatile("csrw mtvec, %0" ::"r"(value));
}

/// Reads the value of the `mtvec` CSR register.
static inline u64
riscv_mtvec_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, mtvec" : "=r"(value));
        return value;
}

/// Writes the given value to the `medeleg` CSR register.
static inline void
riscv_medeleg_write(u64 value)
{
        __asm__ volatile("csrw medeleg, %0" ::"r"(value));
}

/// Reads the value of the `medeleg` CSR register.
static inline u64
riscv_medeleg_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, medeleg" : "=r"(value));
        return value;
}

/// Writes the given value to the `mideleg` CSR register.
static inline void
riscv_mideleg_write(u64 value)
{
        __asm__ volatile("csrw mideleg, %0" ::"r"(value));
}

/// Reads the value of the `mideleg` CSR register.
static inline u64
riscv_mideleg_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, mideleg" : "=r"(value));
        return value;
}

/// Writes the given value to the `mie` CSR register.
static inline void
riscv_mie_write(u64 value)
{
        __asm__ volatile("csrw mie, %0" ::"r"(value));
}

/// Reads the value of the `mie` CSR register.
static inline u64
riscv_mie_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, mie" : "=r"(value));
        return value;
}

/// Writes the given value to the `mip` CSR register.
static inline void
riscv_mip_write(u64 value)
{
        __asm__ volatile("csrw mip, %0" ::"r"(value));
}

/// Reads the value of the `mip` CSR register.
static inline u64
riscv_mip_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, mip" : "=r"(value));
        return value;
}

/// Writes the given value to the `mcounteren` CSR register.
static inline void
riscv_mcounteren_write(u64 value)
{
        __asm__ volatile("csrw mcounteren, %0" ::"r"(value));
}

/// Reads the value of the `mcounteren` CSR register.
static inline u64
riscv_mcounteren_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, mcounteren" : "=r"(value));
        return value;
}

/// Reads the value of the `mhartid` CSR register.
static inline u64
riscv_mhartid_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, mhartid" : "=r"(value));
        return value;
}

// ===================================================================================================
// Physical Memory Protection CSR Functions
// ===================================================================================================

/// Writes the given value to the `pmpaddr0` CSR register.
static inline void
riscv_pmpaddr0_write(u64 value)
{
        __asm__ volatile("csrw pmpaddr0, %0" ::"r"(value));
}

/// Reads the value of the `pmpaddr0` CSR register.
static inline u64
riscv_pmpaddr0_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, pmpaddr0" : "=r"(value));
        return value;
}

/// Writes the given value to the `pmpcfg0` CSR register.
static inline void
riscv_pmpcfg0_write(u64 value)
{
        __asm__ volatile("csrw pmpcfg0, %0" ::"r"(value));
}

/// Reads the value of the `pmpcfg0` CSR register.
static inline u64
riscv_pmpcfg0_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, pmpcfg0" : "=r"(value));
        return value;
}

// ===================================================================================================
// Supervisor-level CSR Functions
// ===================================================================================================

/// Writes the given value to the `sstatus` CSR register.
static inline void
riscv_sstatus_write(u64 value)
{
        __asm__ volatile("csrw sstatus, %0" ::"r"(value));
}

/// Reads the value of the `sstatus` CSR register.
static inline u64
riscv_sstatus_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, sstatus" : "=r"(value));
        return value;
}

/// Writes the given value to the `stvec` CSR register.
static inline void
riscv_stvec_write(u64 value)
{
        __asm__ volatile("csrw stvec, %0" ::"r"(value));
}

/// Reads the value of the `stvec` CSR register.
static inline u64
riscv_stvec_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, stvec" : "=r"(value));
        return value;
}

/// Writes the given value to the `sie` CSR register.
static inline void
riscv_sie_write(u64 value)
{
        __asm__ volatile("csrw sie, %0" ::"r"(value));
}

/// Reads the value of the `sie` CSR register.
static inline u64
riscv_sie_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, sie" : "=r"(value));
        return value;
}

/// Writes the given value to the `sip` CSR register.
static inline void
riscv_sip_write(u64 value)
{
        __asm__ volatile("csrw sip, %0" ::"r"(value));
}

/// Reads the value of the `sip` CSR register.
static inline u64
riscv_sip_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, sip" : "=r"(value));
        return value;
}

/// Writes the given value to the `scounteren` CSR register.
static inline void
riscv_scounteren_write(u64 value)
{
        __asm__ volatile("csrw scounteren, %0" ::"r"(value));
}

/// Reads the value of the `scounteren` CSR register.
static inline u64
riscv_scounteren_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, scounteren" : "=r"(value));
        return value;
}

/// Writes the given value to the `sscratch` CSR register.
static inline void
riscv_sscratch_write(u64 value)
{
        __asm__ volatile("csrw sscratch, %0" ::"r"(value));
}

/// Reads the value of the `sscratch` CSR register.
static inline u64
riscv_sscratch_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, sscratch" : "=r"(value));
        return value;
}

/// Writes the given value to the `sepc` CSR register.
static inline void
riscv_sepc_write(u64 value)
{
        __asm__ volatile("csrw sepc, %0" ::"r"(value));
}

/// Reads the value of the `sepc` CSR register.
static inline u64
riscv_sepc_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, sepc" : "=r"(value));
        return value;
}

/// Writes the given value to the `scause` CSR register.
static inline void
riscv_scause_write(u64 value)
{
        __asm__ volatile("csrw scause, %0" ::"r"(value));
}

/// Reads the value of the `scause` CSR register.
static inline u64
riscv_scause_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, scause" : "=r"(value));
        return value;
}

/// Writes the given value to the `stval` CSR register.
static inline void
riscv_stval_write(u64 value)
{
        __asm__ volatile("csrw stval, %0" ::"r"(value));
}

/// Reads the value of the `stval` CSR register.
static inline u64
riscv_stval_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, stval" : "=r"(value));
        return value;
}

/// Writes the given value to the `senvcfg` CSR register.
static inline void
riscv_senvcfg_write(u64 value)
{
        __asm__ volatile("csrw senvcfg, %0" ::"r"(value));
}

/// Reads the value of the `senvcfg` CSR register.
static inline u64
riscv_senvcfg_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, senvcfg" : "=r"(value));
        return value;
}

/// Writes the given value to the `satp` CSR register.
static inline void
riscv_satp_write(u64 value)
{
        __asm__ volatile("csrw satp, %0" ::"r"(value));
}

/// Reads the value of the `satp` CSR register.
static inline u64
riscv_satp_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, satp" : "=r"(value));
        return value;
}

/// Writes the given value to the `stimecmp` CSR register.
static inline void
riscv_stimecmp_write(u64 value)
{
        __asm__ volatile("csrw stimecmp, %0" ::"r"(value));
}

/// Reads the value of the `stimecmp` CSR register.
static inline u64
riscv_stimecmp_read(void)
{
        u64 value;
        __asm__ volatile("csrr %0, stimecmp" : "=r"(value));
        return value;
}