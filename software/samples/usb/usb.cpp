#include "basesystem.h"
#include "usbhost.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    printf("USB probe\n");
	if (argc<=1)
	{
		printf("Usage: usb deviceaddress\n");
		return 0;
	}

	uint8_t *descdump = new uint8_t[16384];
	uint32_t dumplen = 0;
	uint8_t hidClass = 0;
	uint8_t rcode = USBGetDeviceDescriptor(0, 0, &hidClass, descdump, &dumplen);
	if (rcode != 0)
		USBErrorString(rcode);
	else
	{
		printf("Device descriptor:\n");
		for (uint32_t i=0; i<dumplen; ++i)
			printf("%.2x", descdump[i]);
		printf("\n");
	}

    return 0;
}
