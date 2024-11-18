/** \file
 * File reception utility for TinyOS
 * \ingroup TinyOS
 * This command should be placed in sys/bin and will be initiated remotely to kick off a file transfer.
 */

#include "basesystem.h"
#include "uart.h"
#include "vpu.h"
#include "encoding.h"
#include "serialinringbuffer.h"

#include <stdio.h>

int main()
{
	const char* NACK = "!";
	const char* ACK = "~";

	// // We have 65536-4 bytes of scratchpad memory to work with
	// uint32_t tempMem = E32GetScratchpad();
	// uint32_t *transferSize = (uint32_t*)tempMem;
	// uint32_t *transferBuffer = (uint32_t*)(tempMem + 4);
	// *transferBuffer = drain;
	// *transferSize += 1;

	// Emulator won't have a UART therefore we can't receive a file
	uint32_t isEmulator = read_csr(0xF12) & 0x80000000 ? 0 : 1; // CSR_MARCHID is 0x80000000 for read hardware, 0x00000000 for emulator
	if (isEmulator)
		return 0;

	uint8_t* framebuffer = VPUAllocateBuffer(320*240);
	struct EVideoContext vx;
	vx.m_vmode = EVM_320_Wide;
	vx.m_cmode = ECM_8bit_Indexed;
	VPUSetVMode(&vx, EVS_Enable);
	VPUSetWriteAddress(&vx, (uint32_t)framebuffer);
	VPUSetScanoutAddress(&vx, (uint32_t)framebuffer);
	VPUClear(&vx, 0x03030303);

	// At startup, acknowledge the sender so that it can start sending the file header
	UARTSendBlock((uint8_t*)ACK, 1);

	// Grab the file size
	uint32_t filebytesize = 0;
	for (uint32_t i=0;i<4;)
	{
		uint8_t byte;
		if (SerialInRingBufferRead(&byte, 1))
		{
			filebytesize |= (byte << (i*8));
			++i;
		}
	}
	UARTSendBlock((uint8_t*)ACK, 1);

	// Grab the file name length
	uint32_t fileNameLen = 0;
	for (uint32_t i=0;i<4;)
	{
		uint8_t byte;
		if (SerialInRingBufferRead(&byte, 1))
		{
			fileNameLen |= (byte << (i*8));
			++i;
		}
	}
	if (fileNameLen > 63)
	{
		// Respond to the sender with an error code as the file name is too long
		UARTSendBlock((uint8_t*)NACK, 1);
		return 0;
	}
	else
		UARTSendBlock((uint8_t*)ACK, 1);

	// Grab the file name
	char fileName[64];
	for (uint32_t i=0;i<fileNameLen;)
	{
		if (SerialInRingBufferRead(&fileName[i], 1))
			++i;
	}
	// Null-terminate the file name
	fileName[fileNameLen] = 0;

	uint32_t packetSize = 512;
	uint32_t numPackets = filebytesize / packetSize;
	uint32_t leftoverBytes = filebytesize % packetSize;

	// Grab the file data
	FILE *fp = fopen(fileName, "wb");
	if (fp)
		UARTSendBlock((uint8_t*)ACK, 1);
	else
	{
		// Respond to the sender with an error code as we couldn't open the file
		UARTSendBlock((uint8_t*)NACK, 1);
		return 0;
	}

	// Print the file name to the screen
	VPUPrintString(&vx, 0x00, 0x0F, 0, 0, fileName, fileNameLen);

	uint32_t bytesWritten = 0;
	for (uint32_t i=0; i<numPackets; ++i)
	{
		// Let the sender know we're ready for the next packet
		UARTSendBlock((uint8_t*)ACK, 1);

		// Receive 512 bytes
		for (uint32_t j=0; j<512;)
		{
			uint8_t byte;
			if (SerialInRingBufferRead(&byte, 1))
			{
				uint32_t written = (uint32_t)fwrite(&byte, 1, 1, fp);
				bytesWritten += written;
				++j;
				// Progress bar
				framebuffer[bytesWritten%76800] = 0x0C;
			}
		}
	}

	// Final ACK for partial package
	if (leftoverBytes != 0)
	{
		// Let the sender know we're ready for the next packet
		UARTSendBlock((uint8_t*)ACK, 1);

		// Receive 512 bytes
		for (uint32_t j=0; j<leftoverBytes;)
		{
			uint8_t byte;
			if (SerialInRingBufferRead(&byte, 1))
			{
				uint32_t written = (uint32_t)fwrite(&byte, 1, 1, fp);
				bytesWritten += written;
				++j;
				// Progress bar
				framebuffer[bytesWritten%76800] = 0x0C;
			}
		}
	}

	fclose(fp);

    return 0;
}
