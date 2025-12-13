/* Minimal RISC-V 64-bit crt0 for MirrodOS userspace
 * - Assumes kernel sets up user stack with standard layout:
 *   [ argc (u64) ][ argv pointers ... ][ NULL ][ envp pointers ... ][ NULL ]
 * - Loads argc/argv/envp into a0/a1/a2 and calls `main`.
 * - After `main` returns the exit code is in a0; we issue `ecall` with a7=0
 *   so the kernel can handle process exit. If the kernel has a different
 *   syscall numbering (or no exit handler yet), you'll need to add one.
 */

.section .text
.globl _start
.p2align 2
_start:
	/* Load argc from stack */
	ld a0, 0(sp)

	/* argv pointer is at sp + 8 */
	addi t0, sp, 8
	mv a1, t0

	/* compute envp pointer: sp+8 + argc*8 + 8 (skip NULL) */
	slli t1, a0, 3      /* t1 = argc * 8 */
	add t1, t1, t0      /* t1 = address of NULL after argv */
	addi a2, t1, 8      /* a2 = envp (pointer) */

	/* Call C entry point */
	call main

	/* main returned in a0 -> treat as exit code
	 * Use ecall with a7 = 0 to notify kernel of process exit.
	 * (Adjust syscall number if your kernel expects a different code.)
	 */
	li a7, 0
	ecall

/* If the kernel returns from ecall, just spin here */
.L_halt:
	j .L_halt

.size _start, . - _start

.section .note.GNU-stack,"",@progbits
