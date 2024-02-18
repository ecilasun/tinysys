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
static uint64_t s_checksum = 0U;
static char s_buffertemp[64];
static char s_filename[64];

FIL s_outfp;

void progress(const int A, const int B)
{
	int cx, cy;
	kgetcursor(&cx, &cy);
	ksetcursor(0, cy);
	kprintf("%d/%d", A, B);
}

uint64_t AccumulateHash(const uint64_t inhash, const uint8_t byte)
{
	return 16777619U * inhash ^ (uint64_t)byte;
}

void HandleFileTransfer(uint8_t input)
{
	const uint32_t packetSize = 1024; // NOTE: Match this to riscvtool packet size.

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
		s_buffertemp[s_readlen++] = input;
		if (input == 0)
		{
			s_readlen = 0;
			strcpy(s_filename, "sd:/");
			strcat(s_filename, s_buffertemp);
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
		s_buffertemp[s_readlen++] = input;
		if (input == 0)
		{
			s_filesize = atoi(s_buffertemp);
			s_readlen = 0;

			kprintf("%d bytes\n", s_filesize);
			s_fileTransferMode = 5;

			// Ack size received
			USBSerialWrite("!");
		}
	}
	else if (s_fileTransferMode == 5)
	{
		if (input == '#') // Wait for file block ready signal
		{
			USBSerialWrite("#");
			s_checksum = 2166136261U;
			s_packetCursor = 0;
			s_fileTransferMode = 6;
		}
		if (input == '-') // Last packet had a checksum error, abort
		{
			USBSerialWrite("-");
			kprintf("\nFile transfer aborted (CRC error)\n", s_filesize);
			s_fileTransferMode = 0;
		}
	}
	else if (s_fileTransferMode == 6)
	{
		// Read blocksize bytes 16kbytes into app memory
		uint8_t *filetemp = (uint8_t*)(HEAP_START_APPMEM_END + 16384);
		filetemp[s_readlen] = input;
		
		s_checksum = AccumulateHash(s_checksum, input);

		s_readlen++;
		s_packetCursor++;

		if (s_packetCursor == packetSize)
		{
			// Dump the packetSize bytes to disk
			progress(s_readlen, s_filesize);
			s_fileTransferMode = 5; // Go to next block
			USBSerialWriteRawBytes(&s_checksum, 8);
		}

		// End of file
		if (s_readlen == s_filesize)
		{
			// Dump any leftover bytes
			uint32_t leftover = s_filesize%packetSize;
			if (leftover != 0)
			{
				progress(s_readlen, s_filesize);
				USBSerialWriteRawBytes(&s_checksum, 8);
			}

			USBSerialWrite("!");

			s_readlen = 0;
			s_fileTransferMode = 0; // File transfer done

			// Dump entire file contents
			kprintf("\nSaving file...\n");
			FRESULT re = f_open(&s_outfp, s_filename, FA_CREATE_ALWAYS | FA_WRITE);
			if (re == FR_OK)
			{
				unsigned int written;
				re = f_write(&s_outfp, filetemp, s_filesize, &written);
				if (re == FR_OK)
					kprintf("done.\n");
				else
					kprintf("failed.\n");
				f_sync(&s_outfp);
				f_close(&s_outfp);
			}
		}
	}
}

void HandleSerialInput()
{
	// Pull more incoming data
	uint8_t drain;
	while (SerialInRingBufferRead(&drain, 1))
	{
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
		else
			HandleFileTransfer(drain);
	}
}
