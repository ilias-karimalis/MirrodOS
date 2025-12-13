#pragma once

#include "types/error.h"
#include "vmm.h"
#include <types/number.h>

typedef u64 Elf64_Addr;
typedef u64 Elf64_Off;
typedef u16 Elf64_Half;
typedef u32 Elf64_Word;
typedef i32 Elf64_Sword;
typedef u64 Elf64_Xword;
typedef i64 Elf64_Sxword;

struct Elf64_Ehdr
{
        u8 ident[16];
        Elf64_Half type;
        Elf64_Half machine;
        Elf64_Word version;
        Elf64_Addr entry;
        Elf64_Off phoff;
        Elf64_Off shoff;
        Elf64_Word flags;
        Elf64_Half ehsize;
        Elf64_Half phentsize;
        Elf64_Half phnum;
        Elf64_Half shentsize;
        Elf64_Half shnum;
        Elf64_Half shstrndx;
};

enum Elf_Etype
{
        Elf_Etype_None = 0,
        Elf_Etype_Relocatable = 1,
        Elf_Etype_Executable = 2,
        Elf_Etype_Shared = 3,
        // Elf_Etype_Core = 4,
};

struct Elf64_Phdr
{
        Elf64_Word type;
        Elf64_Word flags;
        Elf64_Off offset;
        Elf64_Addr vaddr;
        Elf64_Addr paddr;
        Elf64_Xword filesz;
        Elf64_Xword memsz;
        Elf64_Xword align;
};

struct Elf64
{
        /// The original ELF blob.
        struct str_view blob;
        /// Pointer to the ELF header within the blob.
        struct Elf64_Ehdr* ehdr;
        // /// Entry point address
        Elf64_Addr entry;
        struct Elf64_Phdr* phdrs;
        Elf64_Half phnum;
};

error_t
elf64_parse_from_blob(struct Elf64* elf, struct str_view blob);

error_t
elf64_load_program(struct Elf64* elf, struct vspace* address_space);

u64
elf_map_flags_to_riscv(u64 elf_flags);
