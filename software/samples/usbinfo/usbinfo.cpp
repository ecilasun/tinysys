#include "basesystem.h"
#include "usbserial.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    printf("USB descriptor dump (serial and console)\n");
	printf("NOTE: USB serial port must be connected!\n");
	printf("Check https://eleccelerator.com/usbdescreqparser/ to decode\n\n");

	uint32_t *desclen = (uint32_t*)(KERNEL_TEMP_MEMORY + 4096);
	uint8_t *devdesc = (uint8_t*)(KERNEL_TEMP_MEMORY + 4100);

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
