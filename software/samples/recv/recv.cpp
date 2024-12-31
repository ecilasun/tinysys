/** \file
 * File reception utility for TinyOS
 * \ingroup TinyOS
 * This command should be placed in sys/bin and will be initiated remotely to kick off a file transfer.
 */

#include "basesystem.h"
#include "core.h"
#include "uart.h"
#include "vpu.h"
#include "task.h"
#include "encoding.h"
#include "mini-printf.h"
#include "serialinringbuffer.h"
#include "lz4.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void DrawProgress(struct EVideoContext* osVideoContext, const uint32_t bytesWritten, const uint32_t filebytesize, const char* filename, uint8_t* framebuffer, int state)
{
	uint32_t progress = (256 * bytesWritten / filebytesize);

	framebuffer[191 + 236*640] = 0x0F;
	framebuffer[191 + 237*640] = 0x0F;
	framebuffer[191 + 238*640] = 0x0F;
	framebuffer[191 + 239*640] = 0x0F;
	framebuffer[191 + 240*640] = 0x0F;
	framebuffer[191 + 241*640] = 0x0F;
	framebuffer[191 + 242*640] = 0x0F;
	framebuffer[191 + 243*640] = 0x0F;

	for (uint32_t i=192; i<192+progress; ++i)
	{
		framebuffer[i + 236*640] = 0x0F;
		framebuffer[i + 237*640] = 0x0C;
		framebuffer[i + 238*640] = 0x0C;
		framebuffer[i + 239*640] = 0x0C;
		framebuffer[i + 240*640] = 0x0C;
		framebuffer[i + 241*640] = 0x0C;
		framebuffer[i + 242*640] = 0x0C;
		framebuffer[i + 243*640] = 0x0F;
	}
	for (uint32_t i=192+progress; i<448; ++i)
	{
		framebuffer[i + 236*640] = 0x0F;
		framebuffer[i + 237*640] = 0x00;
		framebuffer[i + 238*640] = 0x00;
		framebuffer[i + 239*640] = 0x00;
		framebuffer[i + 240*640] = 0x00;
		framebuffer[i + 241*640] = 0x00;
		framebuffer[i + 242*640] = 0x00;
		framebuffer[i + 243*640] = 0x0F;
	}

	framebuffer[448 + 236*640] = 0x0F;
	framebuffer[448 + 237*640] = 0x0F;
	framebuffer[448 + 238*640] = 0x0F;
	framebuffer[448 + 239*640] = 0x0F;
	framebuffer[448 + 240*640] = 0x0F;
	framebuffer[448 + 241*640] = 0x0F;
	framebuffer[448 + 242*640] = 0x0F;
	framebuffer[448 + 243*640] = 0x0F;

	char msg[96];
	if (state == 0)
		mini_snprintf(msg, 96, "Receiving %s", filename);
	else if (state == 1)
		mini_snprintf(msg, 96, "Decompressing %s", filename);
	else if (state == 2)
		mini_snprintf(msg, 96, "Writing %s", filename);
	VPUPrintString(osVideoContext, 0x00, 0x0F, 48, 28, msg, strlen(msg));

	// Kernel isn't double buffered, flush cache so we can see the progress
	CFLUSH_D_L1;
}

void cleanupComms()
{
	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 0;
}

int main()
{
	atexit(cleanupComms);

	// NOTE: We have to be absolutely still with the UART chatter as the other side will not tolerate any noise other than ACK/NACK
	const char* NACK = "!";
	const char* ACK = "+";

	// Emulator can still receive files over net/UART proxy
	/*uint32_t isEmulator = read_csr(0xF12) & 0x80000000 ? 0 : 1; // CSR_MARCHID is 0x80000000 for read hardware, 0x00000000 for emulator
	if (isEmulator)
	{
		UARTPrintf("! Emulator detected, cannot receive files\n");
		return 0;
	}*/

	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 1;

	// Wait for UART chatter to finish
	E32Sleep(HUNDRED_MILLISECONDS_IN_TICKS);

	struct EVideoContext* osVideoContext = VPUGetKernelGfxContext();
	uint8_t* osFramebuffer = (uint8_t*)osVideoContext->m_cpuWriteAddressCacheAligned;

	DrawProgress(osVideoContext, 0, 100, "...", osFramebuffer, 0);

	// At startup, acknowledge the sender so that it can start sending the file header
	UARTSendBlock((uint8_t*)ACK, 1);

	// Grab the encoded size
	uint32_t encodedLen = 0;
	for (uint32_t i=0;i<4;)
	{
		uint8_t byte;
		if (SerialInRingBufferRead(&byte, 1))
		{
			encodedLen |= (byte << (i*8));
			++i;
		}
	}
	UARTSendBlock((uint8_t*)ACK, 1);

	// Grab the file size size
	uint32_t decodedLen = 0;
	for (uint32_t i=0;i<4;)
	{
		uint8_t byte;
		if (SerialInRingBufferRead(&byte, 1))
		{
			decodedLen |= (byte << (i*8));
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
		taskctx->interceptUART = 0;
		return 0;
	}
	else
		UARTSendBlock((uint8_t*)ACK, 1);

	// Grab the file name
	char fileName[96];
	for (uint32_t i=0;i<fileNameLen;)
	{
		if (SerialInRingBufferRead(&fileName[i], 1))
			++i;
	}
	// Null-terminate the file name
	fileName[fileNameLen] = 0;

	// Send ready to receive file
	UARTSendBlock((uint8_t*)ACK, 1);

	uint8_t* sourceBuffer = new uint8_t[encodedLen+64];

	uint32_t bytesReceived = 0;
	do
	{
		// Let the sender know we're ready for the packet size
		UARTSendBlock((uint8_t*)ACK, 1);

		// Receive packet size
		uint32_t packetLen = 0;
		for (uint32_t i=0;i<4;)
		{
			uint8_t byte;
			if (SerialInRingBufferRead(&byte, 1))
			{
				packetLen |= (byte << (i*8));
				++i;
			}
		}

		// Let the sender know we're ready for the packet data
		UARTSendBlock((uint8_t*)ACK, 1);

		// Receive encoded bytes
		for (uint32_t j=0; j<packetLen;)
		{
			uint8_t byte;
			if (SerialInRingBufferRead(&byte, 1))
			{
				sourceBuffer[bytesReceived+j] = byte;
				++j;
			}
		}

		bytesReceived += packetLen;
		DrawProgress(osVideoContext, bytesReceived, encodedLen, fileName, osFramebuffer, 0);
	}
	while(bytesReceived < encodedLen);

	// We're done with the UART
	taskctx->interceptUART = 0;

	uint8_t* targetBuffer = new uint8_t[decodedLen+512];
	DrawProgress(osVideoContext, bytesReceived, encodedLen, fileName, osFramebuffer, 1);
	int unpacked = LZ4_decompress_safe((const char*)sourceBuffer, (char*)targetBuffer, bytesReceived, decodedLen+512);
	if (unpacked > 0)
	{
		DrawProgress(osVideoContext, bytesReceived, encodedLen, fileName, osFramebuffer, 2);
		FILE *fp = fopen(fileName, "wb");
		if (fp)
		{
			/*uint32_t written =*/ (uint32_t)fwrite(targetBuffer, 1, decodedLen, fp);
			fclose(fp);
		}
		else
			printf("! ERROR: can't create file %s\n", fileName);
	}
	else
	{
		printf("! ERROR: decompression failed (received:%d, unpacked:%d, originalpacked:%d, originalunpacked:%d)\n", (int)bytesReceived, (int)unpacked, (int)encodedLen, (int)decodedLen);
	}

	delete [] sourceBuffer;
	delete [] targetBuffer;

    return 0;
}
