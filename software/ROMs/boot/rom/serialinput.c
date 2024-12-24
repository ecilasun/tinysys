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

void SerialInitStatics()
{
	in_gdb_mode = 0;
}

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
				// Read checksum
				int chkcount = 0;
				do
				{
					if (SerialInRingBufferRead(&drain, 1))
						chkcount++;
				} while (chkcount != 2);

				// Parse and respond to GDB packet contents
				GDBStubEndPacket();

				in_gdb_mode = 0;
			}
			else
			{
				// GDB packet body
				GDBStubAddByte(drain);
			}
		}
		else
		{
			if (drain == '$')
			{
				// Start of GDB packet
				GDBStubBeginPacket();
				in_gdb_mode = 1;
			}
			else if (drain == '+')
			{
				// ACK received from GDB
				//GDBAck();
				UARTPrintf("+");
			}
			else if (drain == '-')
			{
				// NACK received from GDB
				//GDBNack();
			}
			else if (drain == 0x03)
			{
				// Break request
				GDBStubBeginPacket();
				GDBStubAddByte(drain);
				GDBStubEndPacket();
			}
			else
				KeyRingBufferWrite(&drain, 1); // Regular keyboard input
		}
	}
}
