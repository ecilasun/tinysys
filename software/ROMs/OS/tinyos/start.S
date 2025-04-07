/// Code section

.section .text
	.global _start
	.global _exit

_start:
	.option norelax

	// NOTE: I$ invalidate MUST be the first instruction executed.
	// If we're loaded from storage as a ROM overlay, this ensures
	// that the I$ sees the new set of instructions.

	// Invalidate I$ and reset all registers
	fence.i
	csrw mie,0				// No external (hardware) interrupts
	csrw mstatus,0			// Disable all interrupts (mstatus:mie=0)
	li ra, 0				// Clear return address
	li sp, 0x0FFDFFF0		// Clear stack pointer
	li gp, 0				// Clear global pointer
	li tp, 0				// Clear thread pointer
	li t0, 0				// Clear temporary registers
	li t1, 0
	li t2, 0
	li s0, 0x0FFDFFF0		// Clear frame pointer
	li s1, 0
	li a0, 0				// Clear argument registers
	li a1, 0
	li a2, 0
	li a4, 0
	li a5, 0
	li a6, 0
	li a7, 0
	li s2, 0				// Clear saved registers
	li s3, 0
	li s4, 0
	li s5, 0
	li s6, 0
	li s7, 0
	li s8, 0
	li s9, 0
	li s10, 0
	li s11, 0
	li t3, 0				// Clear temporary registers
	li t4, 0
	li t5, 0
	li t6, 0

	// Set up default heap address range for dynamic memory allocation
	// NOTE: Same defines in basesystem.h
	li a3, 0x00001000		// HEAP_START
	csrw 0x900, a3			// __heap_start
	csrw 0x902, a3			// __breakpos
	li a3, 0x0F000000		// HEAP_END
	csrw 0x901, a3			// __heap_end
	csrw 0x903, 0			// brk_lock

	// Detect boot override address in mscratch register
	csrr a3, mscratch		// Check if we have a boot addres override (defaults to zero on hard reset)
	beqz a3, _defaultboot	// Skip if it's zero
	jalr a3					// Otherwise branch to the override code
	j _freezecore			// Lock just in case we manage to return from override

	// Normal boot procedure
_defaultboot:
	csrr a3, mhartid		// Check the index of this processor
	bnez a3, _helpercore	// All CPUs except CPU#0 will set up their stack and go to WFI loop

	// CPU#0 takes a detour and does the initial setup
	la a0, __malloc_max_total_mem
	la a2, __BSS_END__$
	sub a2, a2, a0
	li a1, 0
	jal ra, memset

	la a0, atexit
	beqz a0, _noatexitfound

	auipc	a0,0x1
	la a0, __libc_fini_array
	jal ra, atexit

_noatexitfound:
	jal ra, __libc_init_array

	// CPU#0 enters main()
	j main					// Jump to main

	// All other cores have reached here before CPU#0 and are in a WFI loop
_helpercore:				// Put hardware thread to sleep (NOTE: even with MIE disabled we can reset this core later)
	slli a3, a3, 2			// *TASK_MAX
	slli a3, a3, 10			// *1024
	slli a3, a3, 10			// *1024 (1Mbytes per task)
	sub sp, sp, a3			// TASKMEM_END_STACK_END - (_hartid*TASK_MAX*TASK_STACK_SIZE)
	mv s0, sp				// Set frame pointer to current stack pointer

_freezecore:				// Go to sleep (will be 'reset' by CPU#0 to jump to address in mscratch)
	wfi
	j _freezecore

	.align 6				// Align to 64-byte boundary
_exit:
	csrw mie,0				// No external (hardware) interrupts
	csrw mstatus,0			// Disable all interrupts (mstatus:mie=0)
_romfreeze:					// Halt if we ever attempt to exit ROM
	wfi
	j _romfreeze

/// Data section

.section .data
	.align 4
	.global s_cliCtx

// Command line context is 200 bytes i.e. 50 words
s_cliCtx:
	.rept 50
	.word 0
	.endr
