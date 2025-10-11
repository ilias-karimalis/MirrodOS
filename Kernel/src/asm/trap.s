# RISC-V trap handling assembly code

# Macros to help with saving and restoring registers to/from the trap frame.
.altmacro
.set NUM_GENERAL_PURPOSE_REGISTERS, 32
.set NUM_FLOATING_POINT_REGISTERS, 32
.set REG_SIZE, 8
.set NUM_GP_REGS, 32
.set MAX_CPUS, 8

.macro SAVE_GP_REGISTER i, basereg=t6
	sd	x\i, ((\i)*REG_SIZE)(\basereg)
.endm
.macro LOAD_GP_REGISTER i, basereg=t6
	ld	x\i, ((\i)*REG_SIZE)(\basereg)
.endm
.macro SAVE_FP_REGISTER i, basereg=t6
	fsd	f\i, ((NUM_GP_REGS+(\i))*REG_SIZE)(\basereg)
.endm
.macro LOAD_FP_REGISTER i, basereg=t6
	fld	f\i, ((NUM_GP_REGS+(\i))*REG_SIZE)(\basereg)
.endm

.option norvc
.global kernel_asm_trap_handler
.align 2
kernel_asm_trap_handler:
        # All registers are volatile in the trap handler, we need to save them all. To do so,
        # we'll use the trap frame structure, which we store in the sscratch register.
        csrrw  t6, sscratch, t6
        # We can now use t6 as a pointer to the trap frame and write all the registers.
        sd x1, 8(t6)
        sd x2, 16(t6)
        sd x3, 24(t6)
        sd x4, 32(t6)
        sd x5, 40(t6)
        sd x6, 48(t6)
        sd x7, 56(t6)
        sd x8, 64(t6)
        sd x9, 72(t6)
        sd x10, 80(t6)
        sd x11, 88(t6)
        sd x12, 96(t6)
        sd x13, 104(t6)
        sd x14, 112(t6)
        sd x15, 120(t6)
        sd x16, 128(t6)
        sd x17, 136(t6)
        sd x18, 144(t6)
        sd x19, 152(t6)
        sd x20, 160(t6)
        sd x21, 168(t6)
        sd x22, 176(t6)
        sd x23, 184(t6)
        sd x24, 192(t6)
        sd x25, 200(t6)
        sd x26, 208(t6)
        sd x27, 216(t6)
        sd x28, 224(t6)
        sd x29, 232(t6)
        sd x30, 240(t6)
        # Lastly we save the actual t6 register, which we swapped in to sscratch.
        mv t5, t6
        csrr t6, sscratch
        sd x31, 248(t5)

        # Restore the kernel trap frame into sscratch
        csrw sscratch, t5

        # Get ready and jump into the c trap handler
        csrr a0, sepc
        csrr a1, stval
        csrr a2, scause
        csrr a3, sstatus
        mv   a4, t5
        # Load the stack pointer from the trap frame
        ld   sp, 520(a4)
        call kernel_c_trap_handler

        # We are back from the C trap handler, which retursns the trap return address in a0.
        csrw sepc, a0

        csrr t6, sscratch
        # Restore all the registers from the trap frame
        ld      x1, 8(t6)
        ld      x2, 16(t6)
        ld      x3, 24(t6)
        ld      x4, 32(t6)
        ld      x5, 40(t6)
        ld      x6, 48(t6)
        ld      x7, 56(t6)
        ld      x8, 64(t6)
        ld      x9, 72(t6)
        ld      x10, 80(t6)
        ld      x11, 88(t6)
        ld      x12, 96(t6)
        ld      x13, 104(t6)
        ld      x14, 112(t6)
        ld      x15, 120(t6)
        ld      x16, 128(t6)
        ld      x17, 136(t6)
        ld      x18, 144(t6)
        ld      x19, 152(t6)
        ld      x20, 160(t6)
        ld      x21, 168(t6)
        ld      x22, 176(t6)
        ld      x23, 184(t6)
        ld      x24, 192(t6)
        ld      x25, 200(t6)
        ld      x26, 208(t6)
        ld      x27, 216(t6)
        ld      x28, 224(t6)
        ld      x29, 232(t6)
        ld      x30, 240(t6)
        ld      x31, 248(t6)

        sret
        