#include <stdio.h>
#include <string.h>
#include "csrmem.h"
#include "bus.h"
#include "uart.h"

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
	m_csrmem[CSR_WATERMARK] = 0x00000000; // NOTE: Always preserves contents past soft reset
	m_csrmem[CSR_PROGRAMCOUNTER] = 0x00000000;
	m_csrmem[CSR_HWSTATE] = 0x00000000;

	m_timecmp = 0xFFFFFFFFFFFFFFFF;
	m_cycle = 0x0000000000000000;
	m_wallclocktime = 0x0000000000000000;
}

void CCSRMem::Tick(CBus* bus)
{
	m_cycle += 5; // Hardware clocks at about ~5 cycles per instruction (sometimes less, sometimes more)
	m_wallclocktime = m_cycle/15; // main clock is 150MHz and wallclock is 10MHz, therefore /15

	CRV32* cpu = bus->GetCPU(m_hartid);
	CUART* uart = bus->GetUART();

	// Detect reset request
	if (m_cpuresetreq)
	{
		m_cpuresetreq = 0;
		if (cpu)
			cpu->m_pendingCPUReset = true;
	}

	uint32_t uartirq = uart ? uart->m_uartirq : 0;
	uint32_t gpioirq = 0; // Ignoring GPIO from/to ESP32 pins for now
	uint32_t keyirq = 0; // Ignoring sdcard insert/remove signal for now
	uint32_t usbirq = 0; // Ignoring USB for now

	uint32_t ie = m_mstatusshadow & 0x8;

	// Timer interrupt
	uint32_t timerInterrupt = ((m_mieshadow & 0x080 ? 1 : 0) && ie && (m_wallclocktime >= m_timecmp)) ? 1 : 0;

	// Machine external interrupts
	uint32_t hwInterrupt = ((m_mieshadow & 0x800 ? 1 : 0) && ie && (uartirq || gpioirq || keyirq || usbirq)) ? 1 : 0;

	if (cpu)
	{
		// Software interrupt
		cpu->m_sie = (m_mieshadow & 0x008 ? 1 : 0) && ie;
		cpu->m_irq = (timerInterrupt << 1) | (hwInterrupt);
	}

	// IRQ state shadow
	m_irqstate = (uartirq << 3) | (gpioirq << 2) | (keyirq << 1) | (usbirq);
}

void CCSRMem::Read(uint32_t address, uint32_t& data)
{
	uint32_t csrindex = (address >> 2) & 0xFFF;
	//if (csrindex == CSR_MSCRATCH)
	//	__debugbreak();

	if (csrindex == CSR_HWSTATE)
		data = m_irqstate;
	else if (csrindex == CSR_CPURESET)
		data = m_cpuresetreq;
	else if (csrindex == CSR_TIMELO)
		data = (uint32_t)(m_wallclocktime & 0x00000000FFFFFFFFU);
	else if (csrindex == CSR_TIMEHI)
		data = (uint32_t)((m_wallclocktime & 0xFFFFFFFF00000000U) >> 32);
	else if (csrindex == CSR_CYCLELO)
		data = (uint32_t)(m_cycle & 0x00000000FFFFFFFFU);
	else if (csrindex == CSR_CYCLEHI)
		data = (uint32_t)((m_cycle & 0xFFFFFFFF00000000U) >> 32);
	else if (csrindex == CSR_TIMECMPLO)
		data = (uint32_t)(m_timecmp & 0x00000000FFFFFFFFU);
	else if (csrindex == CSR_TIMECMPHI)
		data = (uint32_t)((m_timecmp & 0xFFFFFFFF00000000U) >> 32);
	else if (csrindex == CSR_MSTATUS)
		data = m_mstatusshadow;
	else if (csrindex == CSR_MIE)
		data = m_mieshadow;
	else if (csrindex == CSR_MHARTID)
		data = m_hartid;
	else if (csrindex == CSR_RETILO)
		data = (uint32_t)(m_retired & 0x00000000FFFFFFFFU);
	else if (csrindex == CSR_RETIHI)
		data = (uint32_t)((m_retired & 0xFFFFFFFF00000000U) >> 32);
	else if (csrindex == CSR_PROGRAMCOUNTER)
		data = m_pc;
	else
		data = m_csrmem[csrindex];
}

void CCSRMem::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	uint32_t csrindex = (address>>2) & 0xFFF;
	//if (csrindex == CSR_MSCRATCH)
	//	__debugbreak();

	if (csrindex == CSR_CPURESET)
		m_cpuresetreq = word & 1 ? 1 : 0;
	else if (csrindex == CSR_TIMELO)
		;
	else if (csrindex == CSR_TIMEHI)
		;
	else if (csrindex == CSR_CYCLELO)
		;
	else if (csrindex == CSR_CYCLEHI)
		;
	else if (csrindex == CSR_TIMECMPLO)
		m_timecmp = (m_timecmp & 0xFFFFFFFF00000000U) | ((uint64_t)word);
	else if (csrindex == CSR_TIMECMPHI)
		m_timecmp = ((uint64_t)word << 32) | (m_timecmp & 0x00000000FFFFFFFFU);
	else if (csrindex == CSR_MSTATUS)
		m_mstatusshadow = word;
	else if (csrindex == CSR_MIE)
		m_mieshadow = word; // Only timer, software, and external interrupt state is shadowed
	else if (csrindex == CSR_MHARTID)
		; // noop
	else
		m_csrmem[csrindex] = word;
}
