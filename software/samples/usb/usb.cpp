#include "basesystem.h"
#include "usbserial.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    printf("USB descriptor dump (serial and console)\n");
	printf("Use https://eleccelerator.com/usbdescreqparser/ to decode\n");

	uint32_t *desclen = (uint32_t*)(KERNEL_TEMP_MEMORY + 8192);
	uint8_t *devdesc = (uint8_t*)(KERNEL_TEMP_MEMORY + 8196);

	printf("In-memory device descriptor size: %d\n", *desclen);
	if (*desclen != 0)
	{
		for (uint32_t i=0; i<*desclen; ++i)
		{
			printf("%.2x", devdesc[i]);
			USBSerialWriteHexByte(devdesc[i]);
		}
		USBSerialWrite("\n");
		printf("\n");
	}

    return 0;
}
