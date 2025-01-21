// Boot ROM

#include "rombase.h"
#include "apu.h"
#include "vpu.h"
#include "uart.h"
#include "serialinringbuffer.h"
#include "keyringbuffer.h"
#include "serialinput.h"
#include "commandline.h"
#include "device.h"
#include "gdbstub.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Names of registers for crash dump
static const char *s_regnames[]={"pc", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

void ClearStatics()
{
	// Clear static variables for all systems that use them
	SerialInitStatics();
	CLIClearStatics();
}

void UnrecoverableError()
{
	asm volatile(
		"csrw mie, 0;"			// Disable all interrupt types
		"csrw mstatus, 0;"		// Disable machine interrupts
	);

	// We have experienced a situation which requires a hard-boot
	while (1)
	{
		LEDSetState(0xF);
		E32Sleep(ONE_SECOND_IN_TICKS);
		LEDSetState(0x0);
		E32Sleep(ONE_SECOND_IN_TICKS);
	}
}

void _stubTask()
{
	// NOTE: This task won't actually run
	// It is a stub routine which will be stomped over by main()
	// on first entry to the timer ISR
	while(1)
	{
		asm volatile("nop;");
	}
}

// Make sure we're aligned to sit at a cache line boundary
void __attribute__((aligned(64), noinline)) CopyPayloadAndChainOverlay(void *source)
{
	// Copy this code outside ROM area
	asm volatile(
		"csrw mie, 0;"			// Disable all interrupt types
		"csrw mstatus, 0;"		// Disable machine interrupts
		"mv s0, %0;"			// Payload address: @payload
		"lui s1, 0x0;"			// Target: 0x00000000
		"addi s2, zero, 0x10;"	// Count:  64/4 (0x10)
		"copypayloadloop:"
		"lw a0, 0(s0);"			// Read source word
		"sw a0, 0(s1);"			// Store target word
		"addi s0,s0,4;"
		"addi s1,s1,4;"
		"addi s2,s2,-1;"
		"bnez s2, copypayloadloop;"
		".insn 0xFC000073;"		// Invalidate D$ and I$ (CFLUSH_D_L1)
		"fence.i;"				// Discard anything previously in I$ which might point at the CopyOverlayToROM function's new location
		"lui s0, 0x0;"			// Branch to 0x00000000 which will then copy the ROM from its temporary position onto ROM shadow area
		"jalr s0;"
		:
		// Return values
		:
		// Input parameters
		"r" (source)
		:
		// Clobber list
	);
}

// This is the payload to copy (56 bytes the copy loop copies over 64 in total, adjust to cover all code)
void __attribute__((aligned(64), noinline)) CopyOverlayToROM()
{
	// NOTE: Chained ROM must invalidate I$ on entry!
	asm volatile(
		"lui s0, 0x00010;"		// Source: 0x00010000 (Overlay buffer)
		"lui s1, 0x0FFE0;"		// Target: 0x0FFE0000 (ROM Shadow)
		"lui s2, 0x4;"			// Count:  65536/4 (0x4000, 64Kbytes)
		"copyoverlayloop:"
		"lw a0, 0(s0);"			// Read source word
		"sw a0, 0(s1);"			// Store target word
		"addi s0,s0,4;"
		"addi s1,s1,4;"
		"addi s2,s2,-1;"
		"bnez s2, copyoverlayloop;"
		".insn 0xFC000073;"		// Invalidate & Write Back D$ (CFLUSH.D.L1)
		"lui s0, 0x0FFE0;"		// Branch to reset vector: 0x0FFE0000
		"jalr s0;"				// NOTE: ROM must invalidate I$ on entry
	);
}

uint32_t LoadOverlay(const char *filename)
{
	FIL fp;
	FRESULT fr = f_open(&fp, filename, FA_READ);

	// If the binary blob exists, load it into memory
	if (fr == FR_OK)
	{
		// Reset device to overlay time defaults
		DeviceDefaultState(2);

		FSIZE_t fileSize = f_size(&fp);
		UINT readsize = 0;
		uint32_t *overlay = (uint32_t *)0x00010000; // Overlay buffer
		__builtin_memset(overlay, 0, 65536);		// Clear overlay buffer to zeros
		f_read(&fp, overlay, fileSize, &readsize);	// Load the overlay binary
		f_close(&fp);								// Done with file access
		f_mount(NULL, "sd:", 1);					// Unmount the SD card

		// New ROM image will do its own thing to initialize the CPUs,
		// remove our previous function pointers to branch to on reboot.
		E32SetupCPU(1, (void*)0x0);
		E32SetupCPU(0, (void*)0x0);

		// Watermark register will retain this value on soft reboot
		// so that when we load the overlay (which is the same code as this)
		// we won't attempt to re-load the overlay over and over.
		// NOTE: Only hart#0 updates its watermark register
		write_csr(0xFF0, 0xFFFFFFFF);

		return 1;
	}

	return 0;
}

// Core task for CPU#1 onwards
void __attribute__((aligned(64), noinline)) UserMain()
{
	// Boot sequence for CPU#1
	asm volatile(
		"fence.i;"				// Discard I$
		"csrw 0xFEE, 0;"		// Stop reset
		"csrw mstatus,0;"		// Disable all interrupts (mstatus:mie=0)
		"li sp, 0x0FFDFFF0;"	// Stack base
		"csrr a3, mhartid;"
		"slli a3, a3, 8;"		// hartid*256
		"sub sp, sp, a3;"		// Stack offset for this CPU
		"mv s0, sp;"			// Set frame pointer to current stack pointer
	);

	// Worker cores do not handle hardware interrupts by default,
	// only task switching (timer) and software (illegal instruction)
	uint32_t self = read_csr(mhartid);
	_task_init_context(self);

	struct STaskContext *taskctx = _task_get_context(self);
	_task_add(taskctx, "CPUIdle", _stubTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS, self, TASK_STACK_POINTER(self, 0, TASK_STACK_SIZE));

	InstallISR(self, false, true);

	while(1)
	{
		// Kernel error halts all CPUs other than CPU#0
		// It is up to CPU#0 to detect errors and report/reset others
		if (taskctx->kernelError)
			while(1) { asm volatile("wfi;"); }

		asm volatile("wfi;");

		// Yield time back to any tasks running on this core after handling an interrupt
		clear_csr(mie, MIP_MTIP);
		/*uint64_t currentTime =*/ _task_yield();
		set_csr(mie, MIP_MTIP);
	}
}

void HandleCPUError(struct STaskContext *ctx, const uint32_t cpu)
{
	ksetcolor(CONSOLERED, CONSOLEDIMGRAY);

	switch (ctx->kernelError)
	{
		case 1: kprintf("Unknown hardware device"); break;
		case 2: kprintf("Unknown interrupt type"); break;
		case 3: kprintf("Guru meditation"); break;
		case 4: kprintf("Illegal instruction"); break;
		case 5: kprintf("Breakpoint with no debugger attached"); break;
		default: kprintf("Unknown kernel error"); break;
	}

	if (ctx->kernelError == 1)
		kprintf(" (0x%08X)", ctx->kernelErrorData[0]);

	kprintf(" on CPU #%d", cpu);
	kfillline(' ');

	// Dump task registers
	if (ctx->kernelError == 4)
	{
		uint32_t taskid = ctx->kernelErrorData[0];
		struct STask *task = &ctx->tasks[taskid];
		// Skip zero register and emit '0' since we save PC there
		kprintf("Task: %s", task->name);
		kfillline(' ');
		kprintf("IR=0x%08X", ctx->kernelErrorData[1]);
		kfillline(' ');
		for (uint32_t i=0; i<32; ++i)
		{
			kprintf("%s=0x%08X ", s_regnames[i], task->regs[i]);
			if ((i+1)%4==0)
				kfillline(' ');
		}
		kprintf("\n");
	}

	ksetcolor(CONSOLEDEFAULTFG, CONSOLEDEFAULTBG);

	// Reset CPU#1 here if anything faulted
	// This way we won't risk some stale task being hung on CPU#1 if CPU#0 crashes
	E32SetupCPU(1, UserMain);
	E32ResetCPU(1);

	// Clear error once handled and reported
	ctx->kernelError = 0;
}

// Core task for CPU#0
void __attribute__((aligned(64), noinline)) KernelMain()
{
	// Boot sequence for CPU#1
	asm volatile(
		"fence.i;"				// Discard I$
		"csrw 0xFEE, 0;"		// Stop reset
		"csrw mstatus,0;"		// Disable all interrupts (mstatus:mie=0)
		"li sp, 0x0FFDFFF0;"	// Stack poiner for CPU#0
		"mv s0, sp;"			// Set frame pointer to current stack pointer
	);

	// Mount FAT32 file system on SDcard, if one exists
	LEDSetState(0x1);															// xxxO
	SetWorkDir("sd:/");
	uint32_t mountSuccess = MountDrive();

	// Attempt to load ROM overlay, if it exists
	// Watermark register is zero on hard boot
	LEDSetState(0x2);															// xxOx
	uint32_t waterMark = read_csr(0xFF0);
	if (mountSuccess && (waterMark == 0))
	{
		if (LoadOverlay("sd:/sys/bin/rom.bin"))
		{
			// Copy and branch into CopyOverlayToROM which will then chain back into the new ROM
			CopyPayloadAndChainOverlay(CopyOverlayToROM);
			// We should never arrive here
			UnrecoverableError();
		}
	}

	// Reset buffers
	LEDSetState(0x4);															// xOxx
	KeyRingBufferReset();
	// Initialize serial comm and GDB stub
	SerialInRingBufferReset();

	// Reset peripherals to default states
	LEDSetState(0x8);															// Oxxx
	DeviceDefaultState(1);

	// Worker cores do not handle hardware interrupts by default,
	// only task switching (timer) and software (illegal instruction)
	LEDSetState(0xC);															// OOxx
	uint32_t self = read_csr(mhartid);
	_task_init_context(self);

	// Initialize GDB stub - debugger won't run properly without this init!
	LEDSetState(0x6);															// xOOx
	//GDBStubInit();

	LEDSetState(0x3);															// xxOO
	struct STaskContext *taskctx[3];
	taskctx[0] = _task_get_context(self);
	taskctx[1] = _task_get_context(1);
	taskctx[2] = _task_get_context(2);
	_task_add(taskctx[0], "CPUIdle", _stubTask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS, self, TASK_STACK_POINTER(self, 0, TASK_STACK_SIZE));
	_task_add(taskctx[0], "CLI", _CLITask, TS_RUNNING, HUNDRED_MILLISECONDS_IN_TICKS, self, TASK_STACK_POINTER(self, 1, TASK_STACK_SIZE));

	LEDSetState(0x7);															// xOOO
	InstallISR(self, true, true);

	LEDSetState(0xE);															// OOOx

	// Reset secondary CPUs
	LEDSetState(0x0);															// xxxx

	//kprintf("CPU1 entry:%x mtvec:%x rst:%x\n", (uint32_t)UserMain, E32ReadMemMappedCSR(1, CSR_MSCRATCH), E32ReadMemMappedCSR(1, CSR_CPURESET));

	// Main CLI loop
	struct EVideoContext *kernelgfx = VPUGetKernelGfxContext();
	while (1)
	{
		// ----------------------------------------------------------------
		// Tasks which can be interrupted go here
		// ----------------------------------------------------------------

		// Kernel error / crash handler for this CPU
		// TODO: Catch kernel errors on CPU#1 onwards
		for (int cpu=0; cpu<MAX_HARTS; ++cpu)
		{
			if (taskctx[cpu]->kernelError)
				HandleCPUError(taskctx[cpu], cpu);
		}

		// Refresh console output
		// DEBUG: Optionally echo to UART if no task is intercepting it
		if (kernelgfx->m_consoleUpdated)
			VPUConsoleResolve(kernelgfx/*, !taskctx[0]->interceptUART*/);

		// Yield time as soon as we're done here (disables/enables interrupts)
		clear_csr(mie, MIP_MTIP);
		/*uint64_t currentTime =*/ _task_yield();
		set_csr(mie, MIP_MTIP);

		// ----------------------------------------------------------------
		// H/W related tasks which should't cause IRQs or be interrupted
		// ----------------------------------------------------------------

		/*clear_csr(mstatus, MSTATUS_MIE);
		ProcessUSBDevice(currentTime); // TODO: Any device driver work goes here
		set_csr(mstatus, MSTATUS_MIE);*/

		// ----------------------------------------------------------------
		// Handle serial input to feed to keyboard buffer if there are no
		// user tasks that intercept UART input (only OS apps should)
		// ----------------------------------------------------------------

		if(!taskctx[0]->interceptUART)
			HandleSerialInput();
	}
}

int main()
{
	// Reset static variable pool just in case we're rebooted
	ClearStatics();

	// Reset and wake up all CPUs again, this time with their correct entry points
	E32SetupCPU(1, UserMain);
	E32ResetCPU(1);
	E32SetupCPU(0, KernelMain);
	E32ResetCPU(0);

	// We should never arrive here
	UnrecoverableError();
	return 0;
} 
