#include "basesystem.h"
#include <stdio.h>

int main()
{
    printf("SRAM test!\n");

	// SRAM addresses are only allowed to be 2 byte aligned
	volatile uint16_t *SRAM = (uint16_t *)0x10000000;

	// Write
	for (uint32_t i=0;i<16;++i)
		SRAM[i] = i*2;

	// Read
	for (uint32_t i=0;i<16;++i)
		printf("0x%lx : 0x%x\n", i, SRAM[i]);

    return 0;
}
