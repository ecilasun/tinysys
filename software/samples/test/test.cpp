#include "basesystem.h"
#include "encoding.h"
#include <stdio.h>

int main()
{
    printf("Various tests of hardware systems and OS!\n");

	// CSR access test for all cores

	for (int core= 0; core<2; ++core)
	{
		printf("\nCPU#%d\n", core);
		// Watermark
		{
			unsigned int val = E32ReadMemMappedCSR(core, CSR_WATERMARK);
			printf("watermark: %.8x\n", val);
		}
		// CPU Reset
		{
			unsigned int val = E32ReadMemMappedCSR(core, CSR_CPURESET);
			printf("cpureset: %.8x\n", val);
		}
		// Scratch
		{
			unsigned int val = E32ReadMemMappedCSR(core, CSR_MSCRATCH);
			printf("mscratch: %.8x\n", val);
		}
		// Architecture ID
		{
			unsigned int val = E32ReadMemMappedCSR(core, CSR_MARCHID);
			printf("marchid: %.8x\n", val);
		}
		// Machine ISA
		{
			unsigned int val = E32ReadMemMappedCSR(core, CSR_MISA);
			printf("misa: %.8x\n", val);
		}
		// Retired instruction count
		{
			unsigned int valh = E32ReadMemMappedCSR(core, CSR_INSTRETH);
			unsigned int vall = E32ReadMemMappedCSR(core, CSR_INSTRET);
			printf("instructions retired: %.8x%.8x\n", valh,vall);
		}
		// Time
		{
			unsigned int timh = E32ReadMemMappedCSR(core, CSR_TIMEH);
			unsigned int timl = E32ReadMemMappedCSR(core, CSR_TIME);
			printf("time elapsed: %.8x%.8x\n", timh,timl);
		}
		// CPU cycles
		{
			unsigned int cych = E32ReadMemMappedCSR(core, CSR_CYCLEH);
			unsigned int cycl = E32ReadMemMappedCSR(core, CSR_CYCLE);
			printf("cycles elapsed: %.8x%.8x\n", cych,cycl);
		}
	}

	// Attempt a crash

	// Illegal instruction
	{
		printf("\nCrash test\n");
		asm volatile( ".word 0x00000000;");
	}

    return 0;
}