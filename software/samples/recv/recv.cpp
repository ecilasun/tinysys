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
#include "serialinringbuffer.h"

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

void DrawProgress(struct EVideoContext* osVideoContext, const uint32_t bytesWritten, const uint32_t filebytesize, const char* filename, uint8_t* framebuffer)
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
	sprintf(msg, "Receiving %s", filename);
	VPUPrintString(osVideoContext, 0x00, 0x0F, 48, 28, msg, strlen(msg));

	// Kernel isn't double buffered, flush cache so we can see the progress
	CFLUSH_D_L1;
}

uint32_t Base64Decode(const char* input, const uint32_t inputSize, uint8_t* output)
{
    uint32_t outputSize = 0;
    uint32_t value = 0;
    uint32_t bits = 0;

    for (uint32_t i = 0; i < inputSize; ++i)
	{
        uint8_t c = input[i];
        if (c == '=') {
            break;
        }

        uint8_t decoded = base64lookup[c];
        if (decoded == 64) {
            continue;
        }

        value = (value << 6) | decoded;
        bits += 6;

        if (bits >= 8) {
            bits -= 8;
            output[outputSize++] = (value >> bits) & 0xFF;
        }
    }

    return outputSize;
}

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
	{
		printf("Emulator detected, cannot receive files\n");
		return 0;
	}

	struct EVideoContext* osVideoContext = VPUGetKernelGfxContext();
	uint8_t* osFramebuffer = (uint8_t*)osVideoContext->m_cpuWriteAddressCacheAligned;

	struct STaskContext* taskctx = TaskGetContext(0);
	taskctx->interceptUART = 1;

	DrawProgress(osVideoContext, 0, 100, "...", osFramebuffer);

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
		taskctx->interceptUART = 0;
		return 0;
	}

	uint8_t* decodeBuffer = new uint8_t[packetSize*4+1];
	uint8_t* binaryBuffer = new uint8_t[packetSize+1];

	uint32_t bytesWritten = 0;
	for (uint32_t i=0; i<numPackets; ++i)
	{
		// Let the sender know we're ready for the packet size
		UARTSendBlock((uint8_t*)ACK, 1);

		// Receive packet size
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

		// Let the sender know we're ready for the packet data
		UARTSendBlock((uint8_t*)ACK, 1);

		// Receive encoded bytes
		for (uint32_t j=0; j<encodedLen;)
		{
			uint8_t byte;
			if (SerialInRingBufferRead(&byte, 1))
			{
				decodeBuffer[j] = byte;
				++j;
			}
		}

		// Decode and write to file
		uint32_t decodedLen = Base64Decode((const char*)decodeBuffer, encodedLen, binaryBuffer);
		uint32_t written = (uint32_t)fwrite(binaryBuffer, 1, decodedLen, fp);
		bytesWritten += written;
		DrawProgress(osVideoContext, bytesWritten, filebytesize, fileName, osFramebuffer);
	}

	// Final ACK for partial package
	if (leftoverBytes != 0)
	{
		// Let the sender know we're ready for the packet size
		UARTSendBlock((uint8_t*)ACK, 1);

		// Receive packet size
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

		// Let the sender know we're ready for the packet data
		UARTSendBlock((uint8_t*)ACK, 1);

		// Receive encoded bytes
		for (uint32_t j=0; j<encodedLen;)
		{
			uint8_t byte;
			if (SerialInRingBufferRead(&byte, 1))
			{
				decodeBuffer[j] = byte;
				++j;
			}
		}

		// Decode and write to file
		uint32_t decodedLen = Base64Decode((const char*)decodeBuffer, encodedLen, binaryBuffer);
		uint32_t written = (uint32_t)fwrite(binaryBuffer, 1, decodedLen, fp);
		bytesWritten += written;
		DrawProgress(osVideoContext, bytesWritten, filebytesize, fileName, osFramebuffer);
	}

	fclose(fp);

	taskctx->interceptUART = 0;
    return 0;
}
