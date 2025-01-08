#include <string.h>

#include "basesystem.h"
#include "uart.h"
#include "serialinput.h"
#include "rombase.h"
#include "serialinringbuffer.h"
#include "task.h"
#include "keyringbuffer.h"
#include "gdbstub.h"
#include "keyboard.h"
#include "joystick.h"
#include <stdlib.h>

//static uint32_t in_gdb_mode = 0;

void SerialInitStatics()
{
	//in_gdb_mode = 0;
}

// Note that this function is called only if the running task has not intercepted the serial input
void HandleSerialInput()
{
	// Pull more incoming data
	uint8_t drain;

	// NOTE: A user task could intercept the serial input and handle its own input processing
	while (SerialInRingBufferRead(&drain, 1))
	{
		if (drain == '^')
		{
			uint8_t scandata[KEYBOARD_PACKET_SIZE];
			// Read a 4-byte key scan packet from the serial input buffer
			ReadKeyState(scandata);
			// Process the key state and convert to ASCII for the key buffer
			ProcessKeyState(scandata);
		}
		else if (drain == '%')
		{
			// Joystick axis packet
			uint8_t axisdata[JOYSTICK_AXIS6_PACKET_SIZE];
			// Read a 7-byte joystick axis packet from the serial input buffer
			ReadAxisState(axisdata);
			// Process the joystick axis state and convert to ASCII for the key buffer
			ProcessAxisState(axisdata);
		}
		else if (drain == '@')
		{
			// Joystick button packet
			uint8_t buttondata[JOYSTICK_BUTTON_PACKET_SIZE];
			// Read a 3-byte joystick button packet from the serial input buffer
			ReadButtonState(buttondata);
			// Process the joystick button state and convert to ASCII for the key buffer
			ProcessButtonState(buttondata);
		}
		else
		{
			// Any other ASCII value is fed directly into key buffer
			KeyRingBufferWrite(&drain, 1);
		}

		// GDB stub attempt for future
		/*if (in_gdb_mode)
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
				KeyRingBufferWrite(&drain, 1); // Feed serial data to key buffer
		}*/
	}
}
