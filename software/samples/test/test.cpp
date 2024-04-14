#include "basesystem.h"
#include "encoding.h"
#include <stdio.h>

int main()
{
    printf("Various tests of hardware systems and OS!\n");

	// CSR access test

	// Read watermark register
	{
		unsigned int val = E32ReadMemMappedCSR(0, 0xFF0);
		printf("watermark: %.8x\n", val);
	}
	// Read marchid register
	{
		unsigned int val = E32ReadMemMappedCSR(0, CSR_MARCHID);
		printf("marchid: %.8x\n", val);
	}
	// Read misa register
	{
		unsigned int val = E32ReadMemMappedCSR(0, CSR_MISA);
		printf("misa: %.8x\n", val);
	}
	// Read retired instruction count
	{
		unsigned int val0h = E32ReadMemMappedCSR(0, CSR_INSTRETH);
		unsigned int val0l = E32ReadMemMappedCSR(0, CSR_INSTRET);
		unsigned int val1h = E32ReadMemMappedCSR(1, CSR_INSTRETH);
		unsigned int val1l = E32ReadMemMappedCSR(1, CSR_INSTRET);
		printf("instructions retired on core 0: %.8x%.8x\n", val0h,val0l);
		printf("instructions retired on core 1: %.8x%.8x\n", val1h,val1l);
	}

    return 0;
}
