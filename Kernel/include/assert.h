#pragma once

#include <fmt/print.h>

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

#define ASSERT(condition, ...)                                                                                         \
        do {                                                                                                           \
                if (!(condition)) {                                                                                    \
                        kprintln_string(                                                                               \
                          SV("Assertion failed: " #condition " at " __FILE__ ": " S__LINE__ " with message: "));       \
                        __VA_OPT__(kprintln(__VA_ARGS__);)                                                             \
                        for (;;);                                                                                      \
                }                                                                                                      \
        } while (0)

#define TODO(...)                                                                                                      \
        do {                                                                                                           \
                kprintln_string(SV("TODO: at " __FILE__ ": " S__LINE__ " with message: "));                            \
                __VA_OPT__(kprintln(SV(__VA_ARGS__));)                                                                 \
                for (;;);                                                                                              \
        } while (0)

#define PANIC(...)                                                                                                     \
        do {                                                                                                           \
                kprintln_string(SV("[Kernel Panic]:"));                                                                \
                __VA_OPT__(kprintln(__VA_ARGS__);)                                                                     \
                for (;;);                                                                                              \
        } while (0)
