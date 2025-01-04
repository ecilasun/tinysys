/** \file
 * File reception utility for TinyOS
 * \ingroup TinyOS
 * This command should be placed in sys/bin and will be initiated remotely to kick off a file transfer.
 */

#include "basesystem.h"
#include "core.h"
#include "task.h"
#include "serialinringbuffer.h"
#include "keyboard.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void cleanupComms()
{
	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 0;
}

int main()
{
	atexit(cleanupComms);

	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 1;

	// Wait for UART chatter to finish
	E32Sleep(HUNDRED_MILLISECONDS_IN_TICKS);

	printf("This sample won't respond to CTRL+C\nPlease press ESC to quit\n");

	int done = 0;
	while(!done)
	{
		uint8_t byte;
		// Wait for a scan code packet
		if (SerialInRingBufferRead(&byte, 1))
		{
			if (byte == '^')
			{
				uint8_t scandata[3];
				// Read a 3-byte key scan packet from the serial input buffer
				ReadKeyState(scandata);
				// Emit the ASCII character corresponding to the scan code
				uint8_t ascii = KeyboardScanCodeToASCII(scandata[0], 0);
				printf("scancode:%d ASCII:%c\n", scandata[0], ascii);

				if (ascii == 27) // ESC
				{
					done = 1;
				}
			}
			else
			{
				// This is a non-scan code packet
				printf("Non-scan code packet, exiting\n");
				done = 1;
			}
			
		}
	}

	taskctx->interceptUART = 0;

    return 0;
}
