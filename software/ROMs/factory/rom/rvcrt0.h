#ifdef __cplusplus
extern "C"
{
#endif

	void __attribute__((naked, section (".boot"))) _start()
	{
		asm volatile (

		"li sp, 0x00007FF8;"	   // Stack is at near end of BRAM

		// Single hardware thread simply needs to use the setup address
		"mv s0, sp;"				// Set frame pointer to current stack pointer

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