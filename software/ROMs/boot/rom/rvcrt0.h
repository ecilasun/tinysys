// This unit has more than one HART
// Defined this to allow for auto-setup of per-HART stacks
//#define MULTIHART_SUPPORT

#ifdef __cplusplus
extern "C"
{
#endif

	void __attribute__((naked, section (".boot"))) _start()
	{
		asm volatile (

		// NOTE: This MUST be the first instruction executed.
		// If we're loaded from storage as a ROM overlay,
		// this ensures that the instruction cache now sees
		// the new set of instructions.
		"fence.i;"					// Invalidate I$

		// Set up global pointer - NOTE: ROM does not use GP
		/*".option push;"
		".option norelax;"
		"la gp, __global_pointer$;"
		".option pop;"*/

		"li sp, 0x0FFDFFFC;"	   // Stack is at near end of BRAM

#if defined(MULTIHART_SUPPORT)
		// Set up stack spaces automatically when supporting
		// more than one hardware thread
		// Note that this version leaves one stack space worth of gap at the end
		"la s0, __stack_size$;"		// Grab per-hart stack size from linker script
		"csrr	s1, mhartid;"		// Grab hart id
		"mul s2, s1, s0;"			// stepback = hartid * __stack_size;
		"sub sp, sp, s2;"			// stacktop = base - stepback;
		"mv s0, sp;"				// Set frame pointer to current stack pointer

		"bnez s1, gotoworkermain;"	// Shortcut directly to worker hart entry point (mhartid != 0)
#else
		// Single hardware thread simply needs to use the setup address
		"mv s0, sp;"				// Set frame pointer to current stack pointer
#endif

		// Clear BSS - NOTE: can skip for hardware debug builds
		"la a0, __malloc_max_total_mem;"
		"la a2, __BSS_END__$;"
		"sub a2, a2, a0;"
		"li a1, 0;"
		"jal ra, memset;"

		// Skip if there's no atexit function
		"la a0, atexit;"
		"beqz a0, noatexitfound;"

		// Register finiarray with atexit
		"auipc	a0,0x1;"
		"la a0, __libc_fini_array;"
		"jal ra, atexit;"

		// call initarray
		"noatexitfound: "
		"jal ra, __libc_init_array;"

		"lw a0,0(sp);"
		"addi	a1,sp,4;"
		"li a2,0;"

		// Jump to main
		"j main;"

		// Stop at breakpoint / no return / _exit is useless
		"ebreak;"

#if defined(MULTIHART_SUPPORT)
		// Set up and branch to worker hardware thread entry point
		"gotoworkermain:"
		"lw a0,0(sp);"
		"addi	a1,sp,4;"
		"li a2,0;"
		"j _Z10workermainv;"

		// Put worker hardware thread to sleep if its workermain() exits
		"_workerfreeze: "
		"wfi;"
		"j _workerfreeze;"
#endif
		);
	}

	void __attribute__((noreturn, naked, section (".boot"))) _exit(int x)
	{
		// Halt if we ever attempt to exit ROM
		asm (
			"_romfreeze: "
			"wfi;"
			"j _romfreeze;"
		);
	}

#ifdef __cplusplus
};
#endif