#include <string.h>

#include "basesystem.h"
#include "uart.h"
#include "serialinput.h"
#include "rombase.h"
#include "serialinringbuffer.h"
#include "task.h"
#include "keyringbuffer.h"
#include "gdbstub.h"
#include <stdlib.h>

static uint32_t in_gdb_mode = 0;

void HandleSerialInput()
{
	// Pull more incoming data
	uint8_t drain;
	while (SerialInRingBufferRead(&drain, 1))
	{
		if (in_gdb_mode)
		{
			if (drain == '#')
			{
				// End of GDB command, process and respond
				GDBStubEndCommand();

				// Read checksum
				int chkcount = 0;
				while(SerialInRingBufferRead(&drain, 1) && chkcount!=2) chkcount++;
				in_gdb_mode = 0;
			}
			else
			{
				// GDB command body
				GDBStubAddByte(drain);
				kprintf("%c", drain);
			}
		}
		else
		{
			if (drain == '$')
			{
				// Start of GDB command
				GDBStubBeginCommand();
				in_gdb_mode = 1;
			}
			else
				KeyRingBufferWrite(&drain, 1); // Regular keyboard input
		}
	}
}
