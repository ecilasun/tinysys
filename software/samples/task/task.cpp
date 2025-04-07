/** \file
 * Task management example
 *
 * \ingroup examples
 * This sample shows how to manage multiple tasks on the two HARTs of the tinysys RISC-V core.
 * The two HARTs communicate with each other using shared memory.
 * Both HARTs run a task that increments a counter in their respective shared memory slots.
 * The main task running on HART0 displays the counter values for both HARTs on the screen.
 */

#include "basesystem.h"
#include "core.h"
#include "task.h"
#include "leds.h"
#include "vpu.h"
#include "uart.h"

#include <stdio.h>
#include <string.h>

// This sample shows HART-To-HART communication using shared memory and task management

static struct EVideoContext s_vx;
static struct EVideoSwapContext s_sc;

static uint8_t *s_framebufferA;
static uint8_t *s_framebufferB;

void MyTaskOne()
{
	// Set up shared memory for this HART
	volatile int *sharedmem = (volatile int*)E32GetScratchpad();
	volatile int *s_frame1 = sharedmem+4;

	while(1)
	{
		LEDSetState(*s_frame1);

		TaskYield();
		E32Sleep(150*ONE_MILLISECOND_IN_TICKS);

		*s_frame1 = *s_frame1 + 1;

		// Print something onto the UART port (or the terminal on the emulator)
		UARTPrintf("HART#1: %d\n", *s_frame1);
	}
}

int main(int argc, char *argv[])
{
	// Grab some framebuffer memory
	s_framebufferA = VPUAllocateBuffer(320*240);
	s_framebufferB = VPUAllocateBuffer(320*240);

	// Set up video output
	s_vx.m_vmode = EVM_320_Wide;
	s_vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&s_vx, EVS_Enable);
	VPUSetWriteAddress(&s_vx, (uint32_t)s_framebufferA);
	VPUSetScanoutAddress(&s_vx, (uint32_t)s_framebufferB);

	// Prepare swap context
	s_sc.cycle = 0;
	s_sc.framebufferA = s_framebufferA;
	s_sc.framebufferB = s_framebufferB;

	// Set up shared memory for this HART
	volatile int *sharedmem = (volatile int*)E32GetScratchpad();
	volatile int *s_frame0 = sharedmem;
	volatile int *s_frame1 = sharedmem+4;

	*s_frame0 = 0;
	*s_frame1 = 0;

	// Grab task context of CPU#1
	struct STaskContext *taskctx1 = TaskGetContext(1);

	// Add a new tasks to run for each HART
	uint32_t* stackAddress = new uint32_t[1024];
	int taskID1 = TaskAdd(taskctx1, "MyTaskOne", MyTaskOne, TS_RUNNING, QUARTER_MILLISECOND_IN_TICKS, (uint32_t)stackAddress);
	if (taskID1 == 0)
	{
		printf("Error: No room to add new task on CPU 1\n");
	}

	char tmpstr[128];
	do
	{
		VPUClear(&s_vx, 0x03050305);

		int L1 = snprintf(tmpstr, 127, "HART#1: %d", *s_frame1);
		VPUPrintString(&s_vx, 0x00, 0x0F, 8, 10, tmpstr, L1);

		int L0 = snprintf(tmpstr, 127, "HART#0: %d", *s_frame0);
		VPUPrintString(&s_vx, 0x00, 0x0F, 8, 12, tmpstr, L0);

		TaskYield();
		E32Sleep(60*ONE_MILLISECOND_IN_TICKS);

		// Make sure the video memory write is visible to the VPU
		CFLUSH_D_L1();

		VPUWaitVSync();
		VPUSwapPages(&s_vx, &s_sc);

		*s_frame0 = *s_frame0 + 1;
	} while(1);

	// We're done with the test, remove our task
	TaskExitTaskWithID(taskctx1, taskID1, 0);

	return 0;
}
