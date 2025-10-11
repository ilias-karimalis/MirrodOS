#include <assert.h>
#include <devices/device.h>
#include <fmt/print.h>
#include <riscv.h>
#include <trap.h>

u64
kernel_c_interrupt_handler(u64 sepc, u64 stval, u64 scause, u64 sstatus, struct trap_frame* frame)
{
        // kprintln(
        //   SV("In interrupt handler! sepc: {X}, stval: {X}, scause: {X}, sstatus: {X}"), sepc, stval, scause,
        //   sstatus);

        u64 cause_code = scause & 0xFFF;
        u64 next_pc = sepc;
        switch (cause_code) {
                case IPT_TYPE_SOFTWARE:
                        PANIC(SV("Supervisor software interrupt on CPU:{X}"), frame->hartid);
                        break;
                case IPT_TYPE_TIMER:
                        // The frequency given by QEMU is 10_000_000 Hz, so this sets
                        // the next interrupt to fire every 0.025 seconds.
                        riscv_stimecmp_write(riscv_time() + 10000000);
                        break;
                case IPT_TYPE_EXTERNAL:
                        error_t err = plic_driver_handle_interrupt(devices_get_plic_driver(frame->hartid));
                        switch (error_top(err)) {
                                case EC_SUCCESS:
                                case EC_PLIC_NO_INTERRUPT:
                                        break;
                                case EC_PLIC_UNREGISTERED_DRIVER:
                                        kprintln(SV("PLIC interrupt for unregistered driver on CPU:{X}"),
                                                 frame->hartid);
                                        break;
                                case EC_PLIC_UNREGISTERED_INTERRUPT:
                                        kprintln(SV("PLIC interrupt for unregistered interrupt on CPU:{X}"),
                                                 frame->hartid);
                                        break;
                                default:
                                        __builtin_unreachable();
                        }
                        break;
                case IPT_TYPE_OVERFLOW:
                        PANIC(SV("Supervisor overflow interrupt on CPU:{X}"), frame->hartid);
                        break;
                default:
                        PANIC(SV("Unknown interrupt type {X} on CPU:{X}"), cause_code, frame->hartid);
        }
        return next_pc;
}

u64
kernel_c_exception_handler(u64 sepc, u64 stval, u64 scause, u64 sstatus, struct trap_frame* frame)
{

        kprintln(
          SV("In exception handler! sepc: {X}, stval: {X}, scause: {X}, sstatus: {X}"), sepc, stval, scause, sstatus);

        u64 cause_code = scause & 0xFFF;
        u64 next_pc = sepc;
        switch (cause_code) {
                case EXC_TYPE_ILLEGAL_INSTRUCTION:
                        PANIC(SV("Illegal instruction on CPU:{X} -> pc: {X}, tval: {X}"), frame->hartid, sepc, stval);
                        break;
                case EXC_TYPE_ENV_CALL_FROM_U_MODE:
                        PANIC(SV("E-call from U-mode on CPU:{X} -> pc: {X}"), frame->hartid, sepc);
                        next_pc += 4;
                        break;
                case EXC_TYPE_ENV_CALL_FROM_S_MODE:
                        PANIC(SV("E-call from S-mode on CPU:{X} -> pc: {X}"), frame->hartid, sepc);
                        next_pc += 4;
                        break;
                case EXC_TYPE_INSTRUCTION_PAGE_FAULT:
                        PANIC(
                          SV("Instruction page fault on CPU:{X} -> pc: {X}, tval: {X}"), frame->hartid, sepc, stval);
                        break;
                case EXC_TYPE_LOAD_PAGE_FAULT:
                        PANIC(SV("Load page fault on CPU:{X} -> pc: {X}, tval: {X}"), frame->hartid, sepc, stval);
                        break;
                case EXC_TYPE_STORE_AMO_PAGE_FAULT:
                        PANIC(SV("Store/AMO page fault on CPU:{X} -> pc: {X}, tval: {X}"), frame->hartid, sepc, stval);
                        break;
                default:
                        PANIC(SV("Exception type {X} on CPU:{X} -> pc: {X}, tval: {X}"),
                              cause_code,
                              frame->hartid,
                              sepc,
                              stval);
                        break;
        }

        return next_pc;
}

u64
kernel_c_trap_handler(u64 sepc, u64 stval, u64 scause, u64 sstatus, struct trap_frame* frame)
{
        if (((scause >> 63) & 0x1) == 1) {
                return kernel_c_interrupt_handler(sepc, stval, scause, sstatus, frame);
        }
        return kernel_c_exception_handler(sepc, stval, scause, sstatus, frame);
}
