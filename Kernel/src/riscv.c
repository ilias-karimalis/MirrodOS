#include <kvspace.h>
#include <pmm.h>
#include <riscv.h>
#include <stdalign.h>
#include <types/number.h>

error_t
riscv_sv39_map_small_page(struct riscv_sv39_pt* root, vaddr_t va, paddr_t pa, u64 flags)
{
        if (!IS_ALIGNED(va, RISCV_SV39_PAGE_SIZE) || !IS_ALIGNED(pa, RISCV_SV39_PAGE_SIZE)) {
                return EC_RISCV_SV39_UNALIGNED_ADDR;
        }

        u64 vpn[] = {
                (va >> 12) & 0x1FF, // Level 0 index
                (va >> 21) & 0x1FF, // Level 1 index
                (va >> 30) & 0x1FF  // Level 2 index
        };

        u64* l2_entry = &root->entries[vpn[2]];
        if (!riscv_sv39_pte_valid(*l2_entry)) {
                paddr_t new_page;
                error_t err = pmm_alloc(RISCV_SV39_PAGE_SIZE, &new_page);
                if (error_is_err(err)) {
                        return error_push(err, EC_RISCV_SV39_ALLOC_FAILED);
                }
                *l2_entry = riscv_sv39_create_pte(new_page, RISCV_SV39_PTFLAG_VALID);
        }

        struct riscv_sv39_pt* l1_pt = kernel_hhdm_phys_to_virt(riscv_sv39_pte_get_address(*l2_entry));
        u64* l1_entry = &l1_pt->entries[vpn[1]];
        if (!riscv_sv39_pte_valid(*l1_entry)) {
                paddr_t new_page;
                error_t err = pmm_alloc(RISCV_SV39_PAGE_SIZE, &new_page);
                if (error_is_err(err)) {
                        return error_push(err, EC_RISCV_SV39_ALLOC_FAILED);
                }
                *l1_entry = riscv_sv39_create_pte(new_page, RISCV_SV39_PTFLAG_VALID);
        }

        struct riscv_sv39_pt* l0_pt = kernel_hhdm_phys_to_virt(riscv_sv39_pte_get_address(*l1_entry));
        u64* l0_entry = &l0_pt->entries[vpn[0]];
        if (riscv_sv39_pte_valid(*l0_entry)) {
                return EC_RISCV_SV39_MAPPING_EXISTS;
        }
        *l0_entry = riscv_sv39_create_pte(pa, flags | RISCV_SV39_PTFLAG_VALID);
        return EC_SUCCESS;
}

error_t
riscv_sv39_map_megapage(struct riscv_sv39_pt* root, vaddr_t va, paddr_t pa, u64 flags)
{
        if (!IS_ALIGNED(va, RISCV_SV39_MEGAPAGE_SIZE) || !IS_ALIGNED(pa, RISCV_SV39_MEGAPAGE_SIZE)) {
                return EC_RISCV_SV39_UNALIGNED_ADDR;
        }

        u64 vpn[] = {
                (va >> 21) & 0x1FF, // Level 1 index
                (va >> 30) & 0x1FF  // Level 2 index
        };

        u64* l2_entry = &root->entries[vpn[1]];
        if (!riscv_sv39_pte_valid(*l2_entry)) {
                paddr_t new_page;
                error_t err = pmm_alloc(RISCV_SV39_PAGE_SIZE, &new_page);
                if (error_is_err(err)) {
                        return error_push(err, EC_RISCV_SV39_ALLOC_FAILED);
                }
                *l2_entry = riscv_sv39_create_pte(new_page, RISCV_SV39_PTFLAG_VALID);
        }

        struct riscv_sv39_pt* l1_pt = kernel_hhdm_phys_to_virt(riscv_sv39_pte_get_address(*l2_entry));
        u64* l1_entry = &l1_pt->entries[vpn[0]];
        if (riscv_sv39_pte_valid(*l1_entry)) {
                return EC_RISCV_SV39_MAPPING_EXISTS;
        }
        *l1_entry = riscv_sv39_create_pte(pa, flags | RISCV_SV39_PTFLAG_VALID);
        return EC_SUCCESS;
}

error_t
riscv_sv39_map_gigapage(struct riscv_sv39_pt* root, vaddr_t va, paddr_t pa, u64 flags)
{
        if (!IS_ALIGNED(va, RISCV_SV39_GIGAPAGE_SIZE) || !IS_ALIGNED(pa, RISCV_SV39_GIGAPAGE_SIZE)) {
                return EC_RISCV_SV39_UNALIGNED_ADDR;
        }

        u64 vpn[] = {
                (va >> 30) & 0x1FF // Level 2 index
        };

        u64* l2_entry = &root->entries[vpn[0]];
        if (riscv_sv39_pte_valid(*l2_entry)) {
                return EC_RISCV_SV39_MAPPING_EXISTS;
        }
        *l2_entry = riscv_sv39_create_pte(pa, flags | RISCV_SV39_PTFLAG_VALID);
        return EC_SUCCESS;
}

error_t
riscv_sv39_unmap_small_page(struct riscv_sv39_pt* root, vaddr_t va, paddr_t* pa)
{
        if (!IS_ALIGNED(va, RISCV_SV39_PAGE_SIZE)) {
                return EC_RISCV_SV39_UNALIGNED_ADDR;
        }

        u64 vpn[] = {
                (va >> 12) & 0x1FF, // Level 0 index
                (va >> 21) & 0x1FF, // Level 1 index
                (va >> 30) & 0x1FF  // Level 2 index
        };

        u64* l2_entry = &root->entries[vpn[2]];
        if (!riscv_sv39_pte_valid(*l2_entry)) {
                return EC_RISCV_SV39_NO_MAPPING;
        } else if (riscv_sv39_pte_leaf(*l2_entry)) {
                return EC_RISCV_SV39_MAPPING_EXISTS;
        }

        struct riscv_sv39_pt* l1_pt = kernel_hhdm_phys_to_virt(riscv_sv39_pte_get_address(*l2_entry));
        u64* l1_entry = &l1_pt->entries[vpn[1]];
        if (!riscv_sv39_pte_valid(*l1_entry)) {
                return EC_RISCV_SV39_NO_MAPPING;
        } else if (riscv_sv39_pte_leaf(*l1_entry)) {
                return EC_RISCV_SV39_MAPPING_EXISTS;
        }

        struct riscv_sv39_pt* l0_pt = kernel_hhdm_phys_to_virt(riscv_sv39_pte_get_address(*l1_entry));
        u64* l0_entry = &l0_pt->entries[vpn[0]];
        if (!riscv_sv39_pte_valid(*l0_entry)) {
                return EC_RISCV_SV39_NO_MAPPING;
        }

        if (pa != NULL) {
                *pa = riscv_sv39_pte_get_address(*l0_entry);
        }
        *l0_entry = 0;
        return EC_SUCCESS;
}