#ifdef __cplusplus
extern "C"
{
#endif

	void __attribute__((naked, section (".boot"))) _start()
	{
		asm volatile (
			// NOTE: I$ invalidate MUST be the first instruction executed.
			// If we're loaded from storage as a ROM overlay, this ensures
			// that the I$ sees the new set of instructions.
			"fence.i;"							// Invalidate I$
			"csrw mstatus,0;"					// Disable all interrupts (mstatus:mie=0)
			"csrr a3, mhartid;"					// Check the index of this processor
			"bnez a3, _freezecore;"				// Halt if we're anything other than CPU#0

			"li sp, 0x0FFDFFF0;"				// Stack is at near end of BRAM
			"mv s0, sp;"						// Set frame pointer to current stack pointer

			"la a0, __malloc_max_total_mem;"	// NOTE: can skip BSS clear for hardware debug builds
			"la a2, __BSS_END__$;"
			"sub a2, a2, a0;"
			"li a1, 0;"
			"jal ra, memset;"

			"la a0, atexit;"
			"beqz a0, _noatexitfound;"

			"auipc	a0,0x1;"
			"la a0, __libc_fini_array;"
			"jal ra, atexit;"

			"_noatexitfound: "
			"jal ra, __libc_init_array;"

			"j main;"							// Jump to main

			"_freezecore: "						// Put main hardware thread to sleep if main() somehow manages to exit
			"nop;"
			"j _freezecore;"					// Do not use WFI here to avoid deadlock (fetch unit won't service hw reset IRQ while in WFI)
		);
	}

	void __attribute__((noreturn, naked, section (".boot"))) _exit(int x)
	{
		asm (
			"_romfreeze: "						// Halt if we ever attempt to exit ROM
			"csrw mstatus, 0;"
			"wfi;"								// NOTE: This will deadlock the CPU (fetch unit won't service hw reset IRQ while in WFI)
			"j _romfreeze;"
		);
	}

#ifdef __cplusplus
};
#endif