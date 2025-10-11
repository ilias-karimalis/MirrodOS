#pragma once

#include <types/number.h>

struct str_view
{
        /// Pointer to the underlying byte sequence.
        const char* data;
        /// The number of characters in the sequence.
        size_t size;
};

/// Constructs a str_view from a null-terminated string.
struct str_view
sv_from_null_term(const void* str);

/// Constructs a str_view from a string literal.
#define SV(str) (struct str_view){ .data = str, .size = sizeof(str) / sizeof(str[0]) - 1 }
// Not sure this is a good idea and it's quite jank, but we can print str_views by expanding them to the pointer size
// and accounting for it being two arguments:
#define SVP(strview) (strview).data, (strview).size

/// Accesses the i-th character.
// char
// sv_get(struct str_view sv, size_t index);

/// Returns a new str_view that is the current one advanced by `n` bytes.
struct str_view
sv_advance(struct str_view sv, size_t n);

/// Returns a new str_view that is the current one shortened by `n` bytes.
struct str_view
sv_shorten(struct str_view sv, size_t n);

/// Returns the str_view of a substring starting at `start` with length up to `len`.
struct str_view
sv_substr(struct str_view sv, size_t start, size_t len);

/// Returns the index of the first occurrence of `c` in the str_view, starting from `start`, or
/// `sentinel` if not found.
size_t
sv_find(struct str_view sv, size_t start, char c);

ssize_t
sv_compare(struct str_view s1, struct str_view s2);

static const size_t SV_SENTINEL = (size_t)-1;
