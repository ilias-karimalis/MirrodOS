#pragma once

#include <stddef.h>

void
memzero(void* ptr, size_t count);

void
memcopy(void* dest, const void* src, size_t count);