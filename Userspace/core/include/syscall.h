#pragma once

#include <number.h>

enum SyscallCode
{
        SYSCALL_WRITE = 1,
};


#define SYSCALL0(code) syscall_generic(code, 0, 0, 0, 0, 0, 0, 0)
#define SYSCALL1(code, arg1) syscall_generic(code, (u64)(arg1), 0, 0, 0, 0, 0, 0)
#define SYSCALL2(code, arg1, arg2) syscall_generic(code, (u64)(arg1), (u64)(arg2), 0, 0, 0, 0, 0)
#define SYSCALL3(code, arg1, arg2, arg3) syscall_generic(code, (u64)(arg1), (u64)(arg2), (u64)(arg3), 0, 0, 0, 0)
#define SYSCALL4(code, arg1, arg2, arg3, arg4)                                                                         \
        syscall_generic(code, (u64)(arg1), (u64)(arg2), (u64)(arg3), (u64)(arg4), 0, 0, 0)
#define SYSCALL5(code, arg1, arg2, arg3, arg4, arg5)                                                                   \
        syscall_generic(code, (u64)(arg1), (u64)(arg2), (u64)(arg3), (u64)(arg4), (u64)(arg5), 0, 0)
#define SYSCALL6(code, arg1, arg2, arg3, arg4, arg5, arg6)                                                             \
        syscall_generic(code, (u64)(arg1), (u64)(arg2), (u64)(arg3), (u64)(arg4), (u64)(arg5), (u64)(arg6), 0)
#define SYSCALL7(code, arg1, arg2, arg3, arg4, arg5, arg6, arg7)                                                       \
        syscall_generic(code, (u64)(arg1), (u64)(arg2), (u64)(arg3), (u64)(arg4), (u64)(arg5), (u64)(arg6), (u64)(arg7))

static inline u64
syscall_generic(u64 code, u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 a6)
{
        register u64 x10 asm("a0") = a0;
        register u64 x11 asm("a1") = a1;
        register u64 x12 asm("a2") = a2;
        register u64 x13 asm("a3") = a3;
        register u64 x14 asm("a4") = a4;
        register u64 x15 asm("a5") = a5;
        register u64 x16 asm("a6") = a6;
        register u64 x17 asm("a7") = code;

        asm volatile("ecall"
                     : "+r"(x10)
                     : "r"(x11), "r"(x12), "r"(x13), "r"(x14), "r"(x15), "r"(x16), "r"(x17)
                     : "memory");

        return x10;
}
