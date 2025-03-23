/// Code section

.section .text
	.global _start
	.global _exit
	.global _interrupt_vector_table

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

	// Detect boot override address in mscratch register
	csrr a3, mscratch				// Check if we have a boot addres override (defaults to zero on hard reset)
	beqz a3, _defaultboot			// Skip if it's zero
	jalr a3							// Otherwise branch to the override code
	j _freezecore					// Lock just in case we manage to return from override

	// Normal boot procedure
_defaultboot:
	csrr a3, mhartid				// Check the index of this processor
	bnez a3, _helpercore			// All CPUs except CPU#0 will set up their stack and go to WFI loop

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

	// Install interrupt vector
	la a0, _interrupt_vector_table
	or a0, a0, 0x1		// Set the least significant bit to indicate it's a vector table
	csrw mtvec, a0		// Set the mtvec register to point to the interrupt vector table
	li a0, 0x888		// MIE(11) | SIE(3) | MTE(7)
	csrw mie, a0		// Enable machine timer interrupts
	csrw mstatus, 0x8	// Enable interrupts (MIE=1)

_noatexitfound:
	jal ra, __libc_init_array

	// CPU#0 enters main()
	j main							// Jump to main

	// All other cores have reached here before CPU#0 and are in a WFI loop
_helpercore:						// Put hardware thread to sleep (NOTE: even with MIE disabled we can reset this core later)
	slli a3, a3, 8					// hartid*256
	sub sp, sp, a3					// Stack offset for this CPU
	mv s0, sp						// Set frame pointer to current stack pointer

_freezecore:						// Go to sleep (will be 'reset' by CPU#0 to jump to address in mscratch)
	wfi
	j _freezecore

.align 6							// Align to 64-byte boundary
_exit:
	csrw mie,0						// No external (hardware) interrupts
	csrw mstatus,0					// Disable all interrupts (mstatus:mie=0)
_romfreeze:							// Halt if we ever attempt to exit ROM
	wfi
	j _romfreeze

/// Interrupt vectors
.align 6
_interrupt_vector_table:
	jal zero, isr_unknown	// 0x00 - None
	jal zero, isr_unknown	// 0x04
	jal zero, isr_swi		// 0x08 - Machine software interrupt (2)
	jal zero, isr_ebreak	// 0x0C - EBREAK (3)
	jal zero, isr_unknown	// 0x10
	jal zero, isr_unknown	// 0x14
	jal zero, isr_unknown	// 0x18
	jal zero, isr_unknown	// 0x1C
	jal zero, isr_unknown	// 0x20
	jal zero, isr_unknown	// 0x24
	jal zero, isr_unknown	// 0x28
	jal zero, isr_ecall		// 0x2C - ECALL (11)
	jal zero, isr_unknown	// 0x30
	jal zero, isr_unknown	// 0x34
	jal zero, isr_unknown	// 0x38
	jal zero, isr_unknown	// 0x3C
	jal zero, isr_unknown	// 0x40
	jal zero, isr_unknown	// 0x44
	jal zero, isr_unknown	// 0x48
	jal zero, isr_unknown	// 0x4C
	jal zero, isr_unknown	// 0x50
	jal zero, isr_unknown	// 0x54
	jal zero, isr_unknown	// 0x58
	jal zero, isr_tmi		// 0x5C - Machine timer interrupt (7)
	jal zero, isr_unknown	// 0x60
	jal zero, isr_unknown	// 0x64
	jal zero, isr_unknown	// 0x68
	jal zero, isr_hwi		// 0x6C - Machine external interrupt (11)
	jal zero, isr_unknown	// 0x70
	jal zero, isr_unknown	// 0x74
	jal zero, isr_unknown	// 0x78
	jal zero, isr_unknown	// 0x7C

isr_unknown:
	mret

isr_swi:
	// Illegal instruction handler
	mret

isr_ebreak:
	// Breakpoint handler
	mret

isr_ecall:
	// Syscall handler
	mret

isr_tmi:
	// Task scheduler interrupt handler
	mret

isr_hwi:
	// Hardware interrupt handler
	csrw 0x8A1, ra;
	csrw 0x8A2, sp;
	csrw 0x8A3, gp;
	csrw 0x8A4, tp;
	csrw 0x8A5, t0;
	csrw 0x8A6, t1;
	csrw 0x8A7, t2;
	csrw 0x8A8, s0;
	csrw 0x8A9, s1;
	csrw 0x8AA, a0;
	csrw 0x8AB, a1;
	csrw 0x8AC, a2;
	csrw 0x8AD, a3;
	csrw 0x8AE, a4;
	csrw 0x8AF, a5;
	csrw 0x8B0, a6;
	csrw 0x8B1, a7;
	csrw 0x8B2, s2;
	csrw 0x8B3, s3;
	csrw 0x8B4, s4;
	csrw 0x8B5, s5;
	csrw 0x8B6, s6;
	csrw 0x8B7, s7;
	csrw 0x8B8, s8;
	csrw 0x8B9, s9;
	csrw 0x8BA, s10;
	csrw 0x8BB, s11;
	csrw 0x8BC, t3;
	csrw 0x8BD, t4;
	csrw 0x8BE, t5;
	csrw 0x8BF, t6;
	csrr a5, mepc;
	csrw 0x8A0, a5;

	// Grab hardware ID
	csrr a5, 0xFFF
	and a5,a5,4
	beqz a5, _hwidone
	// Read from UART until there's nothing left
	j _handleuart

_hwidone:

	csrr a5, 0x8A0;
	csrw mepc, a5;
	csrr ra,  0x8A1;
	csrr sp,  0x8A2;
	csrr gp,  0x8A3;
	csrr tp,  0x8A4;
	csrr t0,  0x8A5;
	csrr t1,  0x8A6;
	csrr t2,  0x8A7;
	csrr s0,  0x8A8;
	csrr s1,  0x8A9;
	csrr a0,  0x8AA;
	csrr a1,  0x8AB;
	csrr a2,  0x8AC;
	csrr a3,  0x8AD;
	csrr a4,  0x8AE;
	csrr a5,  0x8AF;
	csrr a6,  0x8B0;
	csrr a7,  0x8B1;
	csrr s2,  0x8B2;
	csrr s3,  0x8B3;
	csrr s4,  0x8B4;
	csrr s5,  0x8B5;
	csrr s6,  0x8B6;
	csrr s7,  0x8B7;
	csrr s8,  0x8B8;
	csrr s9,  0x8B9;
	csrr s10, 0x8BA;
	csrr s11, 0x8BB;
	csrr t3,  0x8BC;
	csrr t4,  0x8BD;
	csrr t5,  0x8BE;
	csrr t6,  0x8BF;
	mret

_handleuart:
	// Echo back everything we've received
	// while (*(DEVICE_UART+0x08) & 0x00000001
	// 		*(DEVICE_UART+0x04) = *(DEVICE_UART+0x00);
	li a0, 0x80080000 // Base device address for UART
	addi a1, a0, 0x08 // UART status address
	addi a2, a0, 0x04 // UART transmit
	addi a3, a0, 0x00 // UART receive
	_echoloop:
		lw a4, 0(a1) // Read status
		and a4, a4, 0x00000001 // Check if there's data
		beqz a4, _doneecho // No more data, we're done
		lw a4, 0(a3) // Read data
		sw a4, 0(a2) // Write data
		j _echoloop
	_doneecho:
	ret

/// Data section

.section .data
	.align 4
	.global s_cliCtx

// Command line context is 200 bytes i.e. 50 words
s_cliCtx:
	.rept 50
	.word 0
	.endr
