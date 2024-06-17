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

	m_timecmp = 0xFFFFFFFFFFFFFFFF;
	m_wallclocktime = 0x0000000000000000;
}

uint32_t CCSRMem::Tick(CRV32* cpu, CUART* uart, uint32_t* sie)
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

	uint32_t uartirq = uart ? uart->m_uartirq : 0;
	uint32_t gpioirq = 0;
	uint32_t keyirq = 0;
	uint32_t usbirq = 0;

	uint32_t ie = m_mstatusshadow & 0x8;

	// Software interrupt
	if (sie)
		*sie = (m_mieshadow & 0x008 ? 1 : 0) && ie;
	// Timer interrupt
	uint32_t timerInterrupt = ((m_mieshadow & 0x080 ? 1 : 0) && ie && (m_wallclocktime >= m_timecmp)) ? 1 : 0;
	// Machine external interrupts
	uint32_t hwInterrupt = ((m_mieshadow & 0x800 ? 1 : 0) && ie && (uartirq || gpioirq || keyirq || usbirq)) ? 1 : 0;

	// IRQ state shadow
	m_irqstate = (uartirq<<3) | (gpioirq<<2) | (keyirq<<1) | (usbirq);

	// Hardware interrupts fire from here, software interrupts fire from fetch
	return (timerInterrupt<<1) | (hwInterrupt);
}

void CCSRMem::Read(uint32_t address, uint32_t& data)
{
	uint32_t csrindex = (address >> 2) & 0xFFF;
	//if (csrindex == CSR_MEPC || csrindex == 0x8a0)
	//	__debugbreak();

	if (csrindex == CSR_HWSTATE)
		data = m_irqstate;
	else if (csrindex == CSR_CPURESET)
		data = m_cpuresetreq;
	else if (csrindex == CSR_TIMELO)
		data = (uint32_t)(m_wallclocktime & 0x00000000FFFFFFFF);
	else if (csrindex == CSR_TIMEHI)
		data = (uint32_t)((m_wallclocktime & 0xFFFFFFFF00000000) >> 32);
	else if (csrindex == CSR_TIMECMPLO)
		data = (uint32_t)(m_timecmp & 0x00000000FFFFFFFF);
	else if (csrindex == CSR_TIMECMPHI)
		data = (uint32_t)((m_timecmp & 0xFFFFFFFF00000000) >> 32);
	else if (csrindex == CSR_MSTATUS)
		data = m_mstatusshadow;
	else if (csrindex == CSR_MIE)
		data = m_mieshadow;
	else
		data = m_csrmem[csrindex];
}

void CCSRMem::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	uint32_t csrindex = (address>>2) & 0xFFF;
	//if (csrindex == CSR_MEPC || csrindex == 0x8a0)
	//	__debugbreak();

	if (csrindex == CSR_CPURESET)
		m_cpuresetreq = word & 1 ? 1 : 0;
	else if (csrindex == CSR_TIMELO)
		m_wallclocktime = (m_wallclocktime & 0xFFFFFFFF00000000) | ((uint64_t)word);
	else if (csrindex == CSR_TIMEHI)
		m_wallclocktime = ((uint64_t)word << 32) | (m_wallclocktime & 0x00000000FFFFFFFF);
	else if (csrindex == CSR_TIMECMPLO)
		m_timecmp = (m_timecmp & 0xFFFFFFFF00000000) | ((uint64_t)word);
	else if (csrindex == CSR_TIMECMPHI)
		m_timecmp = ((uint64_t)word << 32) | (m_timecmp & 0x00000000FFFFFFFF);
	else if (csrindex == CSR_MSTATUS)
		m_mstatusshadow = word;
	else if (csrindex == CSR_MIE)
		m_mieshadow = word; // Only timer, software, and external interrupt state is shadowed
	else
		m_csrmem[csrindex] = word;
}
