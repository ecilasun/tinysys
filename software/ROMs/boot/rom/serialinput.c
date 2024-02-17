#include <string.h>

#include "basesystem.h"
#include "usbserial.h"
#include "serialinput.h"
#include "rombase.h"
#include "serialinringbuffer.h"
#include "task.h"
#include "keyringbuffer.h"
#include <stdlib.h>

static uint32_t s_fileTransferMode = 0;
static uint32_t s_filesize = 0;
static uint32_t s_readlen = 0;
static uint32_t s_packetCursor = 0;
static char s_filename[64];

FIL s_outfp;

void progress(const int A, const int B)
{
	int cx, cy;
	kgetcursor(&cx, &cy);
	ksetcursor(0, cy);
	kprintf("%d/%d", A, B);
}

void HandleFileTransfer(uint8_t input)
{
	const uint32_t packetSize = 512; // NOTE: Match this to riscvtool packet size.

	if (s_fileTransferMode == 1)
	{
		if (input == '!') // Wait for name header
		{
			s_readlen = 0;
			kprintf("Receiving file\n");
			s_fileTransferMode = 2;
		}
	}
	else if (s_fileTransferMode == 2) // Read name up to and including zero terminator
	{
		char *nametemp = (char*)(KERNEL_TEMP_MEMORY + 4096);
		nametemp[s_readlen++] = input;
		if (input == 0)
		{
			s_readlen = 0;
			strcpy(s_filename, "sd:/");
			strcat(s_filename, nametemp);
			kprintf("name = %s\nsize = ", s_filename);
			// Ack name received
			USBSerialWrite("!");
			s_fileTransferMode = 3;
		}
	}
	else if (s_fileTransferMode == 3)
	{
		if (input == '!') // Wait for file size header
			s_fileTransferMode = 4;
	}
	else if (s_fileTransferMode == 4) // Read file size up to and including zero terminator
	{
		char *sizetemp = (char*)(KERNEL_TEMP_MEMORY + 4096);
		sizetemp[s_readlen++] = input;
		if (input == 0)
		{
			s_filesize = atoi(sizetemp);
			s_readlen = 0;

			// Remove any previous instance (file does not have to exist)
			f_unlink(s_filename);

			// Open file for write
			FRESULT re = f_open(&s_outfp, s_filename, FA_CREATE_ALWAYS | FA_WRITE);
			if (re != FR_OK)
			{
				// Abort
				kprintf(" file open failed\n");
				s_fileTransferMode = 0;
			}
			else
			{
				kprintf("%d bytes\n", s_filesize);
				s_fileTransferMode = 5;
			}

			// Ack size received
			USBSerialWrite("!");
		}
	}
	else if (s_fileTransferMode == 5)
	{
		if (input == '#') // Wait for file block ready signal
		{
			USBSerialWrite("#");
			s_packetCursor = 0;
			s_fileTransferMode = 6;
		}
	}
	else if (s_fileTransferMode == 6)
	{
		// Read blocksize bytes
		uint8_t *filetemp = (uint8_t*)(KERNEL_TEMP_MEMORY + 4096);
		filetemp[s_packetCursor++] = input;

		s_readlen++;

		int ack = 0;
		if (s_packetCursor == packetSize)
		{
			// Dump the packetSize bytes to disk
			unsigned int written = 0;
			f_write(&s_outfp, filetemp, packetSize, &written);
			f_sync(&s_outfp);
			progress(s_readlen, s_filesize);
			ack = 1;
			s_fileTransferMode = 5; // Go to next block
		}

		// End of file
		if (s_readlen == s_filesize)
		{
			// Dump any leftover bytes
			uint32_t leftover = s_filesize%packetSize;
			if (leftover != 0)
			{
				unsigned int written;
				f_write(&s_outfp, filetemp, leftover, &written);
				f_sync(&s_outfp);
				progress(s_readlen, s_filesize);
			}

			ack = 1;
			s_readlen = 0;
			s_fileTransferMode = 0; // File done
			f_close(&s_outfp);
			kprintf("\nFile transfer complete.\n");
		}

		// Accept packet or acknowledge transfer complete
		if (ack)
			USBSerialWrite("!");
	}
}

void HandleSerialInput()
{
	// Pull more incoming data
	uint8_t drain;
	while (SerialInRingBufferRead(&drain, 1))
	{
		// Handle serial data transfer
		if (s_fileTransferMode != 0)
			HandleFileTransfer(drain);

		// Incoming data goes to input buffer instead if we're not receiving a debug package
		if (s_fileTransferMode == 0)
		{
			if (drain == '~') // Enter serial file transfer mode
			{
				USBSerialWrite("~");
				s_fileTransferMode = 1;
			}
			else
			{
				uint32_t fakeKey = drain;
				KeyRingBufferWrite(&fakeKey, sizeof(uint32_t));
				// Echo to serial port
				USBSerialWriteN((const char*)&drain, 1);
			}
		}
	}
}
