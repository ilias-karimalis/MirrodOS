#pragma once

// For now we're only handling a very simple short set of printing directives:
// {S} : print string
// {V} : print str_view
// {C} : print char
// {D} : print unsigned size_t base 10
// {X} : print unsigned size_t base 16
// {B} : print unsigned size_t base 2


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