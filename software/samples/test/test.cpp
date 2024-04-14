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
		unsigned int valh = E32ReadMemMappedCSR(0, CSR_INSTRETH);
		unsigned int vall = E32ReadMemMappedCSR(0, CSR_INSTRET);
		printf("instructions retired: %.8x%.8x\n", valh,vall);
	}

    return 0;
}
