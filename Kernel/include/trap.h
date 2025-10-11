#pragma once

#include <kvspace.h>
#include <types/number.h>

struct trap_frame
{
        u64 registers[32];
        u64 float_registers[32];
        u64 satp;
        u8* trap_stack;
        u64 hartid;
        struct allocation stack_allocation;
};

#define TRAP_TYPE_INTERRUPT 0x8000000000000000
#define TRAP_TYPE_EXCEPTION 0x0000000000000000

enum interrupt_type
{
        IPT_TYPE_SOFTWARE = 1,
        IPT_TYPE_TIMER = 5,
        IPT_TYPE_EXTERNAL = 9,
        IPT_TYPE_OVERFLOW = 13,
};

enum exception_type
{
        EXC_TYPE_INSTRUCTION_ADDRESS_MISALIGNED = 0,
        EXC_TYPE_INSTRUCTION_ACCESS_FAULT = 1,
        EXC_TYPE_ILLEGAL_INSTRUCTION = 2,
        EXC_TYPE_BREAKPOINT = 3,
        EXC_TYPE_LOAD_ADDRESS_MISALIGNED = 4,
        EXC_TYPE_LOAD_ACCESS_FAULT = 5,
        EXC_TYPE_STORE_AMO_ADDRESS_MISALIGNED = 6,
        EXC_TYPE_STORE_AMO_ACCESS_FAULT = 7,
        EXC_TYPE_ENV_CALL_FROM_U_MODE = 8,
        EXC_TYPE_ENV_CALL_FROM_S_MODE = 9,
        EXC_TYPE_INSTRUCTION_PAGE_FAULT = 12,
        EXC_TYPE_LOAD_PAGE_FAULT = 13,
        EXC_TYPE_STORE_AMO_PAGE_FAULT = 15,
        EXC_TYPE_SOFTWARE_CHECK = 18,
        EXC_TYPE_HARDWARE_ERROR = 19,
};

// Exceptions to delegate to supervisor mode
#define MEDELEG_INSTRUCTION_ADDRESS_MISALIGNED (1UL << 0)
#define MEDELEG_INSTRUCTION_ACCESS_FAULT (1UL << 1)
#define MEDELEG_ILLEGAL_INSTRUCTION (1UL << 2)
#define MEDELEG_BREAKPOINT (1UL << 3)
#define MEDELEG_LOAD_ADDRESS_MISALIGNED (1UL << 4)
#define MEDELEG_LOAD_ACCESS_FAULT (1UL << 5)
#define MEDELEG_STORE_AMO_ADDRESS_MISALIGNED (1UL << 6)
#define MEDELEG_STORE_AMO_ACCESS_FAULT (1UL << 7)
#define MEDELEG_ENVIRONMENT_CALL_FROM_U_MODE (1UL << 8)
// Note: Don't delegate S-mode ecall (bit 9) - machine mode should handle it
#define MEDELEG_INSTRUCTION_PAGE_FAULT (1UL << 12)
#define MEDELEG_LOAD_PAGE_FAULT (1UL << 13)
#define MEDELEG_STORE_AMO_PAGE_FAULT (1UL << 15)

#define MEDELEG_SUPERVISOR_EXCEPTIONS                                                                                  \
        (MEDELEG_INSTRUCTION_ADDRESS_MISALIGNED | MEDELEG_INSTRUCTION_ACCESS_FAULT | MEDELEG_ILLEGAL_INSTRUCTION |     \
         MEDELEG_BREAKPOINT | MEDELEG_LOAD_ADDRESS_MISALIGNED | MEDELEG_LOAD_ACCESS_FAULT |                            \
         MEDELEG_STORE_AMO_ADDRESS_MISALIGNED | MEDELEG_STORE_AMO_ACCESS_FAULT |                                       \
         MEDELEG_ENVIRONMENT_CALL_FROM_U_MODE | MEDELEG_INSTRUCTION_PAGE_FAULT | MEDELEG_LOAD_PAGE_FAULT |             \
         MEDELEG_STORE_AMO_PAGE_FAULT)

// Interrupts to delegate to supervisor mode
#define MIDELEG_USER_SOFTWARE_INTERRUPT (1UL << 0)
#define MIDELEG_SUPERVISOR_SOFTWARE_INTERRUPT (1UL << 1)
#define MIDELEG_USER_TIMER_INTERRUPT (1UL << 4)
#define MIDELEG_SUPERVISOR_TIMER_INTERRUPT (1UL << 5)
#define MIDELEG_USER_EXTERNAL_INTERRUPT (1UL << 8)
#define MIDELEG_SUPERVISOR_EXTERNAL_INTERRUPT (1UL << 9)

#define MIDELEG_SUPERVISOR_INTERRUPTS                                                                                  \
        (MIDELEG_USER_SOFTWARE_INTERRUPT | MIDELEG_SUPERVISOR_SOFTWARE_INTERRUPT | MIDELEG_USER_TIMER_INTERRUPT |      \
         MIDELEG_SUPERVISOR_TIMER_INTERRUPT | MIDELEG_USER_EXTERNAL_INTERRUPT | MIDELEG_SUPERVISOR_EXTERNAL_INTERRUPT)

u64
kernel_c_trap_handler(u64 epc, u64 trap_value, u64 cause, u64 status, struct trap_frame* frame);