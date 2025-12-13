#include "fmt/print.h"
#include "kvspace.h"
#include "pmm.h"
#include "riscv.h"
#include "types/error.h"
#include "types/number.h"
#include "types/str_view.h"
#include "vmm.h"
#include <assert.h>
#include <elf.h>
#include <memory.h>

enum Elf_IdentIndex
{
        Elf_Ident_MAG0 = 0,
        Elf_Ident_MAG1 = 1,
        Elf_Ident_MAG2 = 2,
        Elf_Ident_MAG3 = 3,
        Elf_Ident_CLASS = 4,
        Elf_Ident_DATA = 5,
        Elf_Ident_VERSION = 6,
        Elf_Ident_OSABI = 7,
        Elf_Ident_ABIVERSION = 8,
        Elf_Ident_PAD = 9,
        Elf_Ident_NIDENT = 16,
};

enum Elf_Class
{
        Elf_Class_None = 0,
        Elf_Class_32 = 1,
        Elf_Class_64 = 2,
};

enum Elf_PType
{
        Elf_PType_NULL = 0,
        Elf_PType_LOAD = 1,
        Elf_PType_DYNAMIC = 2,
        Elf_PType_INTERP = 3,
        Elf_PType_NOTE = 4,
        Elf_PType_SHLIB = 5,
        Elf_PType_PHDR = 6,
        Elf_PType_TLS = 7,
};

error_t
elf64_parse_from_blob(struct Elf64* elf, struct str_view blob)
{
        if (blob.size < sizeof(struct Elf64_Ehdr)) {
                kprintln(S("ELF blob too small: {}."), blob.size);
                return EC_ELF_INVALID;
        }
        struct Elf64_Ehdr* ehdr = (struct Elf64_Ehdr*)blob.data;

        if (ehdr->ident[Elf_Ident_MAG0] != 0x7f || ehdr->ident[Elf_Ident_MAG1] != 'E' ||
            ehdr->ident[Elf_Ident_MAG2] != 'L' || ehdr->ident[Elf_Ident_MAG3] != 'F' ||
            ehdr->ident[Elf_Ident_CLASS] != Elf_Class_64 || ehdr->ident[Elf_Ident_VERSION] != 1) {
                kprintln(S("ELF blob has invalid magic number, class, or version."));
                return EC_ELF_INVALID;
        }
        // TODO(Ilias): Maybe we should be also checking the OSABI fields?

        // Check that the program header table size is at least the size of one entry
        if (ehdr->phentsize < sizeof(struct Elf64_Phdr) || ehdr->phnum == 0) {
                kprintln(S("ELF blob has invalid program header table size or number of entries."));
                return EC_ELF_INVALID;
        }

        elf->ehdr = ehdr;
        elf->entry = ehdr->entry;
        elf->phdrs = (struct Elf64_Phdr*)(blob.data + ehdr->phoff);
        elf->phnum = ehdr->phnum;
        return EC_SUCCESS;
}

error_t
elf64_load_program(struct Elf64* elf, struct vspace* address_space)
{
        // Iterate over the program headers and load the segments
        for (size_t i = 0; i < elf->phnum; i++) {
                struct Elf64_Phdr phdr = elf->phdrs[i];
                if (phdr.type != Elf_PType_LOAD) {
                        kprintln(S("Skipping loading of program segment of type: {X}."), phdr.type);
                        continue;
                }

                ASSERT(phdr.vaddr % RISCV_SV39_PAGE_SIZE == 0,
                       S("ELF segment virtual address not aligned to page size."));

                size_t align = phdr.align <= 1 ? phdr.vaddr % RISCV_SV39_PAGE_SIZE : phdr.align;
                size_t memory_size = phdr.memsz;
                size_t file_size = phdr.filesz;
                u64 flags = elf_map_flags_to_riscv(phdr.flags);

                paddr_t pa = pmm_alloc_aligned_noerr(memory_size, align);
                if (pa == 0) {
                        kprintln(S("Failed to allocate physical memory for ELF segment."));
                        return EC_ELF_INVALID;
                }

                // Create the mapping in the child address space.
                error_t err = vspace_allocmap_fixed(address_space, phdr.vaddr, pa, memory_size, flags);
                if (error_is_err(err)) {
                        return error_push(err, EC_ELF_INVALID);
                }

                // Copy the segment data from the ELF blob to the allocated physical memory.
                void* local_address = kernel_hhdm_phys_to_virt(pa);
                memcopy(local_address, sv_advance(elf->blob, phdr.offset).data, file_size);
        }

        return EC_SUCCESS;
}

u64
elf_map_flags_to_riscv(u64 elf_flags)
{
        const u64 Elf_Executable = 0x1;
        const u64 Elf_Writable = 0x2;
        const u64 Elf_Readable = 0x4;
        u64 riscv_flags = 0;
        if (elf_flags & Elf_Readable) {
                riscv_flags |= RISCV_SV39_PTFLAG_READ;
        }
        if (elf_flags & Elf_Writable) {
                riscv_flags |= RISCV_SV39_PTFLAG_WRITE;
        }
        if (elf_flags & Elf_Executable) {
                riscv_flags |= RISCV_SV39_PTFLAG_EXECUTE;
        }
        return riscv_flags;
}
