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
inline u64
riscv_sv39_create_pte(paddr_t pa, u64 flags) {
        return (pa >> 12) << 10 | flags;
}

/// Returns the physical address to the next level page table or leaf page this pte points to.
inline paddr_t
riscv_sv39_pte_get_address(u64 pte) {
        return (pte >> 10) << 12;
}

// Note(Ilias): Why would the page calculation be different for different page sizes?
// /// Returns the physical address of the SV39_MEGAPAGE this pte points to.
// inline paddr_t
// riscv_sv39_pte_megapage_address(u64 pte) {
//         return (pte >> 10) << 21;
// }

// /// Returns the physical address of the SV39_GIGAPAGE this pte points to.
// inline paddr_t
// riscv_sv39_pte_gigapage_address(u64 pte) {
//         return (pte >> 10) << 30;
// }

/// Returns true if this entry is valid.
inline bool
riscv_sv39_pte_valid(u64 pte) {
        return (pte & RISCV_SV39_PTFLAG_VALID) != 0;
}

/// Returns true if this entry is readable
inline bool
riscv_sv39_pte_readable(u64 pte) {
        return (pte & RISCV_SV39_PTFLAG_READ) != 0;
}

/// Returns true if this entry is writable
inline bool
riscv_sv39_pte_writable(u64 pte) {
        return (pte & RISCV_SV39_PTFLAG_WRITE) != 0;
}

/// Returns true if this entry is executable
inline bool
riscv_sv39_pte_executable(u64 pte) {
        return (pte & RISCV_SV39_PTFLAG_EXECUTE) != 0;
}

/// Returns true if the entry points to a leaf (a page) and not to another page table.
inline bool
riscv_sv39_pte_leaf(u64 pte) {
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