#include <stdio.h>
#include <string.h>
#include "memman.h"

static const uint32_t quadexpand[] = {
	0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF,
	0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
	0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
	0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF,
};

CMemMan::CMemMan()
{
	// Warning! Allocating 256Mbytes in one go!
	m_devicemem = malloc(256*1024*1024);
	m_csrmem[0] = (uint32_t*)malloc(4 * 1024 * sizeof(uint32_t));
	m_csrmem[1] = (uint32_t*)malloc(4 * 1024 * sizeof(uint32_t));

	// Clear memory
	memset(m_devicemem, 0, 256 * 1024 * 1024);
	memset(m_csrmem[0], 0, 4 * 1024 * sizeof(uint32_t));
	memset(m_csrmem[1], 0, 4 * 1024 * sizeof(uint32_t));

	// Set default CSR contents
	m_csrmem[0][CSR_TIMECMPLO] = 0xFFFFFFFF;
	m_csrmem[0][CSR_TIMECMPHI] = 0xFFFFFFFF;
	m_csrmem[0][CSR_MISA] = 0x00801100;
	m_csrmem[0][CSR_MARCHID] = 0x80000000;
	m_csrmem[1][CSR_TIMECMPLO] = 0xFFFFFFFF;
	m_csrmem[1][CSR_TIMECMPHI] = 0xFFFFFFFF;
	m_csrmem[1][CSR_MISA] = 0x00801100;
	m_csrmem[1][CSR_MARCHID] = 0x80000000;
}

CMemMan::~CMemMan()
{
	free(m_csrmem[0]);
	free(m_csrmem[1]);
	free(m_devicemem);
}

void CMemMan::Tick(CClock& cpuclock)
{

}

void CMemMan::CopyROM(uint32_t resetvector, uint8_t *bin, uint32_t size)
{
	uint32_t *ddr3 = (uint32_t*)m_devicemem;
	uint32_t *rom = (uint32_t*)bin;
	// Convert from cache byte order to memory byte order
	uint32_t base = resetvector>>2;
	for (uint32_t i=0; i<size/4; ++i)
		ddr3[base+i] = rom[i];
	printf("ROM @%.8x (%.8x bytes)\n", resetvector, size);
}

uint32_t CMemMan::FetchInstruction(uint32_t address)
{
	uint32_t instruction;
	uint32_t *wordmem = (uint32_t*)m_devicemem;

	// TODO: Handle interrupts and route to different code path as with the actual hardware

	instruction = wordmem[address>>2];
	return instruction;
}

uint32_t CMemMan::FetchDataWord(uint32_t address)
{
	uint32_t data;
	if (address&0x80000000)
	{
		// Memory mapped devices
		// See header file for device map

		uint32_t dev = (address&0xF0000) >> 16;
		switch(dev)
		{
			case 0:
			{
				// DEVICE_GPIO
				printf("<-GPIO\n");
				data = 0;
			}
			break;
			case 1:
			{
				// DEVICE_LEDS
				printf("<-LED\n");
				data = 0;
			}
			break;
			case 2:
			{
				// DEVICE_VPUC
				printf("<-VPU\n");
				data = 0;
			}
			break;
			case 3:
			{
				// DEVICE_SDCC
				data = m_sdcard.Read(address);
			}
			break;
			case 4:
			{
				// DEVICE_XADC
				printf("<-ADC\n");
				data = 0;
			}
			break;
			case 5:
			{
				// DEVICE_DMAC
				printf("<-DMA\n");
				data = 0;
			}
			break;
			case 6:
			{
				// DEVICE_USBA
				printf("<-USB-A\n");
				data = 0;
			}
			break;
			case 7:
			{
				// DEVICE_APUC
				printf("<-APU\n");
				data = 0;
			}
			break;
			case 8:
			{
				// DEVICE_MAIL
				printf("<-MAIL\n");
				data = 0;
			}
			break;
			case 9:
			{
				// DEVICE_UART
				printf("<-UART\n");
				data = 0;
			}
			break;
			case 0xA:
			{
				// All CSRs acts like regular memory on emulator
				// CPU fills in the immutable data
				uint32_t csrindex = (address>>2) & 0xFFF;
				data = m_csrmem[0][csrindex];
				printf("0x%.8x<-CSR0[%d]\n", data, csrindex);
			}
			break;
			case 0xB:
			{
				// DEVICE_CSR1
				uint32_t csrindex = (address>>2) & 0xFFF;
				data = m_csrmem[1][csrindex];
				printf("0x%.8x<-CSR1[%d]\n", data, csrindex);
			}
			break;
			break;
		}
	}
	else
	{
		// TODO: Return from D$ instead for consistency of simulation
		uint32_t *wordmem = (uint32_t*)m_devicemem;
		data = wordmem[address>>2];
	}
	return data;
}

void CMemMan::WriteDataWord(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	if (address&0x80000000)
	{
		// Memory mapped devices
		// See header file for device map
		// Memory mapped devices
		// See header file for device map

		uint32_t dev = (address&0xF0000) >> 16;
		switch(dev)
		{
			case 0:
			{
				// DEVICE_GPIO
				printf("GPIO@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 1:
			{
				// DEVICE_LEDS
				printf("LEDS@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 2:
			{
				// DEVICE_VPUC
				printf("VPUC@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 3:
			{
				// DEVICE_SDCC
				m_sdcard.Write(address, word);
				printf("SDCC@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 4:
			{
				// DEVICE_XADC
				printf("XADC@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 5:
			{
				// DEVICE_DMAC
				printf("DMAC@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 6:
			{
				// DEVICE_USBA
				printf("USB-A@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 7:
			{
				// DEVICE_APUC
				printf("APUC@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 8:
			{
				// DEVICE_MAIL
				printf("MAIL@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 9:
			{
				// DEVICE_UART
				printf("UART@0x%.8X<-0x%.8x\n", address, word);
			}
			break;
			case 0xA:
			{
				// All CSRs acts like regular memory on emulator
				// CPU fills in the immutable data
				uint32_t csrindex = (address>>2) & 0xFFF;
				m_csrmem[0][csrindex] = word;
				printf("CSR0[%d]<-0x%.8x\n", csrindex, word);
			}
			break;
			case 0xB:
			{
				// All CSRs acts like regular memory on emulator
				// CPU fills in the immutable data
				uint32_t csrindex = (address>>2) & 0xFFF;
				m_csrmem[1][csrindex] = word;
				printf("CSR1[%d]<-0x%.8x\n", csrindex, word);
			}
			break;
		}
	}
	else
	{
		// TODO: Use D$ instead for consistency of simulation
		uint32_t olddata;
		uint32_t *wordmem = (uint32_t*)m_devicemem;
		olddata = wordmem[address>>2];

		// Expand the wstrobe
		uint32_t fullmask = quadexpand[wstrobe];
		uint32_t invfullmask = ~fullmask;

		// Mask and mix incoming and old data
		wordmem[address>>2] = (olddata&invfullmask) | (word&fullmask);

		printf("RAM@0x%.8X<-0x%.8x\n", address, word);
	}
}
