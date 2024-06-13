#include <stdio.h>
#include <string.h>
#include "csrmem.h"

CCSRMem::CCSRMem()
{
	m_csrmem = (uint32_t*)malloc(4096 * sizeof(uint32_t));
}

CCSRMem::~CCSRMem()
{
	free(m_csrmem);
}

void CCSRMem::Reset()
{
	// Clear memory
	memset(m_csrmem, 0, 4096 * sizeof(uint32_t));

	// Set default CSR contents
	m_csrmem[CSR_TIMECMPLO] = 0xFFFFFFFF;
	m_csrmem[CSR_TIMECMPHI] = 0xFFFFFFFF;
	m_csrmem[CSR_MISA] = 0x00801100;
	m_csrmem[CSR_MARCHID] = 0x80000000;

	m_csrmem[CSR_CPURESET] = 0x00000000;
	m_csrmem[CSR_WATERMARK] = 0x00000000; // NOTE: Always preserve contents past soft reset
	m_csrmem[CSR_PROGRAMCOUNTER] = 0x00000000;
	m_csrmem[CSR_HWSTATE] = 0x00000000;
}

uint32_t CCSRMem::Tick(CRV32* cpu, uint32_t* sie)
{
	// Detect reset request
	if (m_cpuresetreq)
	{
		m_cpuresetreq = 0;
		if (cpu)
		{
			cpu->m_pendingCPUReset = true;
			cpu->m_resetvector = m_csrmem[CSR_MSCRATCH];
		}
	}

	// TODO: Detect interrupt requests here as with real hardware
	// uartirq, gpioirq, keyirq, usbirq

	uint64_t wallclocktime = (uint64_t)m_csrmem[CSR_TIMELO] | ((uint64_t)m_csrmem[CSR_TIMEHI] << 32);
	uint64_t timecmp = (uint64_t)m_csrmem[CSR_TIMECMPLO] | ((uint64_t)m_csrmem[CSR_TIMECMPHI] << 32);
	int timerirq = wallclocktime >= timecmp ? 1 : 0;
	uint32_t uartirq = 0;
	uint32_t gpioirq = 0;
	uint32_t keyirq = 0;
	uint32_t usbirq = 0;

	// Software interrupt
	if (sie)
		*sie = (m_mieshadow & 0x008 ? 1:0) & m_mstatusIEshadow;
	// Timer interrupt
	uint32_t timerInterrupt = ((m_mieshadow & 0x080 ? 1 : 0) & m_mstatusIEshadow & timerirq) ? 1 : 0;
	// Machine external interrupts
	uint32_t hwInterrupt = ((m_mieshadow & 0x800 ? 1 : 0) & m_mstatusIEshadow & (uartirq | gpioirq | keyirq | usbirq)) ? 1 : 0;

	//if (timerInterrupt)
	//	printf("TIRQ @%llu >= @%llu\n", wallclocktime, timecmp);
	//if (hwInterrupt)
	//	printf("HIRQ\n");

	// Hardware interrupts fire from here, software interrupts fire from fetch
	return (timerInterrupt<<1) | (hwInterrupt);
}

void CCSRMem::Read(uint32_t address, uint32_t& data)
{
	uint32_t csrindex = (address >> 2) & 0xFFF;

	if (csrindex == CSR_HWSTATE)
		data = 0; // TODO: {28'd0, uartirq, gpioirq, keyirq, usbirq};
	else
		data = m_csrmem[csrindex];

	//if (csrindex == CSR_MEPC || csrindex == 0x8a0)
	//	__debugbreak();

	//printf("CSR0[%d]->data\n", csrindex);
}

void CCSRMem::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	uint32_t csrindex = (address>>2) & 0xFFF;

	if (csrindex == CSR_CPURESET)
		m_cpuresetreq = word&1 ? 1:0;
	else
	{
		m_csrmem[csrindex] = word;

		// Global interrupt enable (MIE) bit
		if (csrindex == CSR_MSTATUS)
			m_mstatusIEshadow = (word & 8) ? 1 : 0;

		// Only store MEIE, MTIE and MSIE bits
		if (csrindex == CSR_MIE)
			m_mieshadow = word & 0x888;

		//if (csrindex == CSR_MEPC || csrindex == 0x8a0)
		//	__debugbreak();
	}

	//printf("CSR0[%d]<-0x%.8x\n", csrindex, word);
}
