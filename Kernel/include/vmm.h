// Virtual Memory Manager, creates and manages the address space for a processs
#pragma once

#include <types/error.h>

struct vspace
{};

void
vspace_initialize(struct vspace* vspace);

error_t
vspace_allocate_fixed(struct vspace* vspace, vaddr_t vaddr, size_t size, u64 flags);

error_t
vspace_allocate(struct vspace* vspace, vaddr_t* vaddr, size_t size, size_t alignment, u64 flags);