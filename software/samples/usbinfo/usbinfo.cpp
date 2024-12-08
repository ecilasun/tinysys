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

int main(int argc, char **argv)
{
	UARTPrintf("Check https://eleccelerator.com/usbdescreqparser/ to decode the descriptor dump\n\n");

	uint32_t *desclen = (uint32_t*)(KERNEL_TEMP_MEMORY + 4096);
	uint8_t *devdesc = (uint8_t*)(KERNEL_TEMP_MEMORY + 4100);

	UARTPrintf("Device descriptor size: %d\n", *desclen);
	if (*desclen != 0)
	{
		for (uint32_t i=0; i<*desclen; ++i)
		{
			UARTPrintf("%02x ", devdesc[i]);
			if (i!=0 && (i%16)==0)
				UARTPrintf("\n");
		}
		UARTPrintf("\n");
	}
	else
		UARTPrintf("No device information found\n");

    return 0;
}
