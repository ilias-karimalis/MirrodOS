#pragma once

#include <types/error.h>
#include <types/str_view.h>

error_t
kprint_initialize(void (*put_char_fn)(char));

void
kprint(struct str_view str, ...);

void
kprintln(struct str_view str, ...);

void
kprint_string(struct str_view str);

void
kprintln_string(struct str_view str);