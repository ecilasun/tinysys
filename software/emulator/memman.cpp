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
	m_csrmem = (uint32_t*)malloc(4*1024*sizeof(uint32_t));

	// Clear memory
	memset(m_devicemem, 0, 256*1024*1024);
	memset(m_csrmem, 0, 4*1024*sizeof(uint32_t));

	// Set default CSR contents
	m_csrmem[CSR_TIMECMPLO] = 0xFFFFFFFF;
	m_csrmem[CSR_TIMECMPHI] = 0xFFFFFFFF;
	m_csrmem[CSR_MISA] = 0x00801100;
	m_csrmem[CSR_MARCHID] = 0x80000000;
}

CMemMan::~CMemMan()
{
	free(m_csrmem);
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
	// TODO: Return from I$ instead for consistency of simulation
	uint32_t instruction;
	uint32_t *wordmem = (uint32_t*)m_devicemem;
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

		uint32_t dev = (address&0xF000) >> 12;
		switch(dev)
		{
			case 0:
			{
				// DEVICE_TRUC
				printf("<-TRU\n");
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
				// DEVICE_SPIC
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
				// DEVICE_USBC
				printf("<-USB-C\n");
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
				// DEVICE_OPL2
				printf("<-OPL2\n");
				data = 0;
			}
			break;
			case 9:
			{
				// DEVICE_USBA
				printf("<-USB-A\n");
				data = 0;
			}
			break;
			case 10:
			{
				// DEVICE_CSR0
				uint32_t csrindex = address&0xFFF;
				// All CSRs acts like regular memory on emulator
				// CPU fills in the immutable data
				data = m_csrmem[csrindex];
			}
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

		uint32_t dev = (address&0xF000) >> 12;
		switch(dev)
		{
			case 0:
			{
				// DEVICE_TRUC
				printf("TRU:%.8x\n", word);
			}
			break;
			case 1:
			{
				// DEVICE_LEDS
				printf("LED:%.8x\n", word);
			}
			break;
			case 2:
			{
				// DEVICE_VPUC
				printf("VPU:%.8x\n", word);
			}
			break;
			case 3:
			{
				// DEVICE_SPIC
				m_sdcard.Write(address, word);
			}
			break;
			case 4:
			{
				// DEVICE_XADC
				printf("ADC:%.8x\n", word);
			}
			break;
			case 5:
			{
				// DEVICE_DMAC
				printf("DMA:%.8x\n", word);
			}
			break;
			case 6:
			{
				// DEVICE_USBC
				printf("USB-C:%.8x\n", word);
			}
			break;
			case 7:
			{
				// DEVICE_APUC
				printf("APU:%.8x\n", word);
			}
			break;
			case 8:
			{
				// DEVICE_OPL2
				printf("OPL2:%.8x\n", word);
			}
			break;
			case 9:
			{
				// DEVICE_USBA
				printf("USB-A:%.8x\n", word);
			}
			break;
			case 10:
			{
				// DEVICE_CSR0
				uint32_t csrindex = address&0xFFF;
				// All CSRs acts like regular memory on emulator
				// CPU fills in the immutable data
				m_csrmem[csrindex] = word;
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
	}
}
