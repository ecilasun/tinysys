/** \file
 * Memory test example
 *
 * \ingroup examples
 * This example demonstrates how to test the memory on the system using the memtest library.
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "basesystem.h"
#include "memtest.h"
#include "uart.h"

int main()
{
	UARTPrintf("\nTesting DDR3 on AXI4 bus\n");

	uint8_t *testbuffer = (uint8_t*)malloc(0x02000000);

	UARTPrintf("Clearing memory\n");
	uint64_t startclock = E32ReadTime();
	for (uint32_t m=0; m<0x02000000; m+=4)
		*((volatile uint32_t*)testbuffer) = 0x00000000;

	uint64_t endclock = E32ReadTime();
	uint32_t deltams = ClockToMs(endclock-startclock);
	UARTPrintf("Clearing 32Mbytes took %d ms\n", (unsigned int)deltams);

	int rate = (1024*32*1024) / deltams;
	UARTPrintf("Zero-write rate is %d Kbytes/sec\n", rate);

	UARTPrintf("\n-------------MemTest--------------\n");
	UARTPrintf("Copyright (c) 2000 by Michael Barr\n");
	UARTPrintf("----------------------------------\n");

	UARTPrintf("Data bus test...");
	int failed = 0;
	volatile datum* tbuf = (volatile datum*)testbuffer;
	for (uint32_t i=0; i<262144; i+=4)
	{
		failed += memTestDataBus(tbuf);
		tbuf++;
	}
	UARTPrintf("%s (%d failures)\n", failed == 0 ? "passed" : "failed", failed);

	UARTPrintf("Address bus test (0x0B000000-0x0B03FFFF)...");
	int errortype = 0;
	datum* res = memTestAddressBus((volatile datum*)testbuffer, 262144, &errortype);
	UARTPrintf("%s\n", res == NULL ? "passed" : "failed");
	if (res != NULL)
	{
		if (errortype == 0)
			UARTPrintf("Reason: Address bit stuck high at 0x%x\n",(unsigned int)res);
		if (errortype == 1)
			UARTPrintf("Reason: Address bit stuck low at 0x%x\n",(unsigned int)res);
		if (errortype == 2)
			UARTPrintf("Reason: Address bit shorted at 0x%x\n",(unsigned int)res);
	}

	UARTPrintf("Memory device test (0x0C000000-0x0C03FFFF)...");
	datum* res2 = memTestDevice((volatile datum *)testbuffer, 262144);
	UARTPrintf("%s\n", res2 == NULL ? "passed" : "failed");
	if (res2 != NULL)
	{
		UARTPrintf("Reason: incorrect value read at 0x%x\n",(unsigned int)res2);
	}

	if ((failed != 0) | (res != NULL) | (res2 != NULL))
	UARTPrintf("Memory device does not appear to be working correctly.\n");

	free(testbuffer);

	return 0;
}
