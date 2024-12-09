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

static const uint8_t base64lookup[256] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 0-15
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 16-31
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63, // 32-47
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64, // 48-63
    64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, // 64-79
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64, // 80-95
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96-111
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64, // 112-127
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 128-143
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 144-159
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 160-175
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 176-191
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 192-207
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 208-223
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, // 224-239
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64  // 240-255
};

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

int main()
{
	const char* NACK = "!";
	const char* ACK = "+";

	// // We have 65536-4 bytes of scratchpad memory to work with
	// uint32_t tempMem = E32GetScratchpad();
	// uint32_t *transferSize = (uint32_t*)tempMem;
	// uint32_t *transferBuffer = (uint32_t*)(tempMem + 4);
	// *transferBuffer = drain;
	// *transferSize += 1;

	// Emulator won't have a UART therefore we can't receive a file
	uint32_t isEmulator = read_csr(0xF12) & 0x80000000 ? 0 : 1; // CSR_MARCHID is 0x80000000 for read hardware, 0x00000000 for emulator
	if (isEmulator)
	{
		UARTPrintf("Emulator detected, cannot receive files\n");
		return 0;
	}

	struct EVideoContext* osVideoContext = VPUGetKernelGfxContext();
	uint8_t* osFramebuffer = (uint8_t*)osVideoContext->m_cpuWriteAddressCacheAligned;

	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 1;

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
			printf("ERROR: can't create file %s\n", fileName);
	}
	else
	{
		printf("ERROR: decompression failed (received:%d, unpacked:%d, originalpacked:%d, originalunpacked:%d)\n", (int)bytesReceived, (int)unpacked, (int)encodedLen, (int)decodedLen);
	}

	delete [] sourceBuffer;
	delete [] targetBuffer;

    return 0;
}
