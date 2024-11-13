/** \file
 * USB descriptor dump example
 *
 * \ingroup examples
 * This example demonstrates how to dump the USB descriptor of the device to the serial port.
 * The USB descriptor is stored in the kernel temporary memory by the USB stack.
 * The descriptor is then read from the memory and printed to the serial port and to the console.
 * The USB descriptor can be decoded using the USB descriptor parser tool available at https://eleccelerator.com/usbdescreqparser/
 * Please note that if there is no UART connection to a host PC, this code might hang waiting for a connection.
 */

#include "basesystem.h"
#include "uart.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    printf("USB descriptor dump (serial and console)\n");
	printf("NOTE: USB serial port must be connected!\n");
	printf("Check https://eleccelerator.com/usbdescreqparser/ to decode\n\n");

	uint32_t *desclen = (uint32_t*)(KERNEL_TEMP_MEMORY + 4096);
	uint8_t *devdesc = (uint8_t*)(KERNEL_TEMP_MEMORY + 4100);

	printf("In-memory device descriptor size: %ld\n", *desclen);
	if (*desclen != 0)
	{
		for (uint32_t i=0; i<*desclen; ++i)
		{
			printf("%.2x", devdesc[i]);
			UARTWriteHex(devdesc[i]);
		}
		UARTWrite("\n");
		printf("\n");
	}

    return 0;
}
