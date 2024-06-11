#include <stdio.h>
#include <string.h>
#include "csrmem.h"

static const uint32_t quadexpand[] = {
	0x00000000, 0x000000FF, 0x0000FF00, 0x0000FFFF,
	0x00FF0000, 0x00FF00FF, 0x00FFFF00, 0x00FFFFFF,
	0xFF000000, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF,
	0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF,
};

CCSRMem::CCSRMem()
{
	m_csrmem = (uint32_t*)malloc(4 * 1024 * sizeof(uint32_t));
}

CCSRMem::~CCSRMem()
{
	free(m_csrmem);
}

void CCSRMem::Reset()
{
	// Clear memory
	memset(m_csrmem, 0, 4 * 1024 * sizeof(uint32_t));

	// Set default CSR contents
	m_csrmem[CSR_TIMECMPLO] = 0xFFFFFFFF;
	m_csrmem[CSR_TIMECMPHI] = 0xFFFFFFFF;
	m_csrmem[CSR_MISA] = 0x00801100;
	m_csrmem[CSR_MARCHID] = 0x80000000;
}

void CCSRMem::Tick(CClock& cpuclock)
{

}

void CCSRMem::Read(uint32_t address, uint32_t& data)
{
	uint32_t csrindex = (address >> 2) & 0xFFF;
	data = m_csrmem[csrindex];
	printf("CSR0[%d]->data\n", csrindex);
}

void CCSRMem::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	uint32_t csrindex = (address>>2) & 0xFFF;
	m_csrmem[csrindex] = word;
	printf("CSR0[%d]<-0x%.8x\n", csrindex, word);
}
