#include <types/str_view.h>

struct str_view
sv_from_null_term(const char* str)
{
    size_t length = 0;
    for (const char* curr = str; *curr != '\0'; curr++, length++);
    return (struct str_view){ str, length };
}

struct str_view
sv_advance(struct str_view sv, size_t n)
{
    return (struct str_view){ sv.data + n, sv.size - n };
}

struct str_view
sv_shorten(struct str_view sv, size_t n)
{
    return (struct str_view){ sv.data, sv.size - n };
}

struct str_view
sv_substr(struct str_view sv, size_t start, size_t length)
{
    return (struct str_view){ sv.data + start, length };
}

size_t
sv_find(struct str_view sv, size_t start, char c)
{
    for (size_t i = start; i < sv.size; i++) {
        if (sv.data[i] == c) {
            return i;
        }
    }
    return SV_SENTINEL;
}

ssize_t
sv_compare(struct str_view s1, struct str_view s2)
{
    for (size_t i = 0; i < s1.size && i < s2.size; i++) {
        if (s1.data[i] != s2.data[i]) {
            return s1.data[i] != s2.data[i];
        }
    }
    return s1.size - s2.size;
}
