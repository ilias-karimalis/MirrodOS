#include <assert.h>
#include <fmt/print.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <types/error.h>
#include <types/str_view.h>

void (*put)(char) = NULL;

void
kprint_null_terminated(const char* str)
{
        while (*str != '\0') {
                put(*str++);
        }
}

void
kprint_print_int(size_t value, size_t base)
{
#define MAX_INT_BUF_SIZE 67
        static char buf[MAX_INT_BUF_SIZE] = { 0 };
        size_t i = MAX_INT_BUF_SIZE - 2;
        if (value == 0) {
                buf[i--] = '0';
                goto base_compute;
        }
        while (value && i) {
                buf[i] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[value % base];
                i--;
                value /= base;
        }
base_compute:
        switch (base) {
                case 16:
                        buf[i--] = 'x';
                        buf[i--] = '0';
                        break;
                case 8:
                        buf[i--] = 'o';
                        buf[i--] = '0';
                        break;
                case 2:
                        buf[i--] = 'b';
                        buf[i--] = '0';
                        break;
                default:
                        break;
        }
        kprint_null_terminated(&buf[i + 1]);
}

void
kprint_print_spec(struct str_view spec, va_list* args)
{
        // For now we're only handling a very simple short set of printing directives:
        // {S} : print string
        // {V} : print str_view
        // {C} : print char
        // {D} : print unsigned size_t base 10
        // {X} : print unsigned size_t base 16
        // {B} : print unsigned size_t base 2
        ASSERT(spec.size == 1);
        char spec_byte = spec.data[0];

        switch (spec_byte) {
                case 'S': {
                        const char* str = va_arg(*args, const char*);
                        kprint_null_terminated(str);
                } break;

                case 'V': {
                        const char* str = va_arg(*args, const char*);
                        size_t len = va_arg(*args, size_t);
                        kprint_string((struct str_view){ str, len });
                        break;
                }

                case 'C': {
                        char c = (char)va_arg(*args, int);
                        put(c);

                } break;
                case 'D': {
                        size_t v = va_arg(*args, size_t);
                        kprint_print_int(v, 10);
                } break;
                case 'X': {
                        size_t v = va_arg(*args, size_t);
                        kprint_print_int(v, 16);
                } break;

                default:
                        PANIC(SV("Unknown format specifier: {C}"), spec_byte);
                        break;
        }
}

void
kprint_formatted_print(struct str_view format, va_list* args)
{
        bool IN_FORMAT_SPEC = false;
        size_t spec_start = 0;
        size_t spec_end = 0;

        for (size_t i = 0; i < format.size; i++) {
                char curr = format.data[i];
                switch (curr) {
                        case '{':
                                if (IN_FORMAT_SPEC) {
                                        put('{');
                                        IN_FORMAT_SPEC = false;
                                        spec_end = i;
                                } else {
                                        IN_FORMAT_SPEC = true;
                                        spec_start = i;
                                }
                                break;

                        case '}':
                                if (IN_FORMAT_SPEC) {
                                        spec_end = i;
                                        struct str_view spec =
                                          sv_substr(format, spec_start + 1, spec_end - spec_start - 1);
                                        kprint_print_spec(spec, args);
                                        IN_FORMAT_SPEC = false;
                                } else {
                                        put('}');
                                }
                                break;
                        default:
                                if (!IN_FORMAT_SPEC) {
                                        put(curr);
                                }
                                break;
                }
        }
}

error_t
kprint_initialize(void (*put_char_fn)(char))
{
        if (put_char_fn == NULL) {
                return EC_NULL_ARGUMENT;
        }

        put = put_char_fn;
        return EC_SUCCESS;
}

void
kprint_string(struct str_view str)
{
        for (size_t i = 0; i < str.size; i++) put(str.data[i]);
}

void
kprintln_string(struct str_view str)
{
        for (size_t i = 0; i < str.size; i++) put(str.data[i]);
        put('\n');
}

void
kprint(struct str_view format, ...)
{
        va_list args;
        va_start(args, format);
        kprint_formatted_print(format, &args);
        va_end(args);
}

void
kprintln(struct str_view format, ...)
{
        va_list args;
        va_start(args, format);
        kprint_formatted_print(format, &args);
        va_end(args);
        put('\n');
}
