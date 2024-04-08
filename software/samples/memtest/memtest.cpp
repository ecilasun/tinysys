#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "basesystem.h"
#include "memtest.h"

int main()
{
	printf("\nTesting DDR3 on AXI4 bus\n");

	uint8_t *testbuffer = (uint8_t*)malloc(0x02000000);

	printf("Clearing memory\n");
	uint64_t startclock = E32ReadTime();
	for (uint32_t m=0; m<0x02000000; m+=4)
		*((volatile uint32_t*)testbuffer) = 0x00000000;

	uint64_t endclock = E32ReadTime();
	uint32_t deltams = ClockToMs(endclock-startclock);
	printf("Clearing 32Mbytes took %d ms\n", (unsigned int)deltams);

	int rate = (1024*32*1024) / deltams;
	printf("Zero-write rate is %d Kbytes/sec\n", rate);

	printf("\n-------------MemTest--------------\n");
	printf("Copyright (c) 2000 by Michael Barr\n");
	printf("----------------------------------\n");

	printf("Data bus test...");
	int failed = 0;
	volatile datum* tbuf = (volatile datum*)testbuffer;
	for (uint32_t i=0; i<262144; i+=4)
	{
		failed += memTestDataBus(tbuf);
		tbuf++;
	}
	printf("%s (%d failures)\n", failed == 0 ? "passed" : "failed", failed);

	printf("Address bus test (0x0B000000-0x0B03FFFF)...");
	int errortype = 0;
	datum* res = memTestAddressBus((volatile datum*)testbuffer, 262144, &errortype);
	printf("%s\n", res == NULL ? "passed" : "failed");
	if (res != NULL)
	{
		if (errortype == 0)
			printf("Reason: Address bit stuck high at 0x%x\n",(unsigned int)res);
		if (errortype == 1)
			printf("Reason: Address bit stuck low at 0x%x\n",(unsigned int)res);
		if (errortype == 2)
			printf("Reason: Address bit shorted at 0x%x\n",(unsigned int)res);
	}

	printf("Memory device test (0x0C000000-0x0C03FFFF)...");
	datum* res2 = memTestDevice((volatile datum *)testbuffer, 262144);
	printf("%s\n", res2 == NULL ? "passed" : "failed");
	if (res2 != NULL)
	{
		printf("Reason: incorrect value read at 0x%x\n",(unsigned int)res2);
	}

	if ((failed != 0) | (res != NULL) | (res2 != NULL))
	printf("Memory device does not appear to be working correctly.\n");

	free(testbuffer);

	return 0;
}
