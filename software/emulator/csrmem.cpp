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
	// NOTE: The highest bit is clear when we're running under emulation
	// This is done to help ROM and user software identify the runtime environment
	m_csrmem[CSR_MARCHID] = 0x00000000; // Hardware sets this to 0x80000000

	m_csrmem[CSR_CPURESET] = 0x00000000;
	m_csrmem[CSR_WATERMARK] = 0x00000000; // NOTE: Always preserves contents past soft reset
	m_csrmem[CSR_PROGRAMCOUNTER] = 0x00000000;
	m_csrmem[CSR_HWSTATE] = 0x00000000;

	m_timecmpshadow = 0xFFFFFFFFFFFFFFFF;
	m_mepcshadow = 0;
	m_mieshadow = 0;
	m_mtvecshadow = 0;

	m_cycle = 0x0000000000000000;
	m_wallclocktime = 0x0000000000000000;
}

void CCSRMem::UpdateTime(uint64_t wallclock, uint32_t executeCount)
{
	m_cycle = executeCount;
	m_wallclocktime = wallclock; // 10MHz clock for wallclock
}

void CCSRMem::Tick(CBus* bus)
{
	CUART* uart = bus->GetUART();
	CVPU* vpu = bus->GetVPU();

	// Detect reset request
	if (m_cpuresetreq)
	{
		m_cpuresetreq = 0;
		m_pendingCPUReset = true;
	}

	uint32_t uartirq = uart ? uart->m_uartirq : 0;
	uint32_t keyirq = bus->GetSDCard()->HasSwitchEvents() ? 1 : 0; // Pending SDCard switch events in queue
	uint32_t usbirq = 0; // Ignoring USB for now

	// Software interrupt
	m_sie = ((m_mieshadow & 0x1) ? 1 : 0) && m_mstatusieshadow;

	// Timer interrupt
	uint32_t timerInterrupt = ((m_mieshadow & 0x2 ? 1 : 0) && m_mstatusieshadow && (m_wallclocktime >= m_timecmpshadow)) ? 1 : 0;

	// IRQ state shadow
	m_irqstate = (uartirq << 2) | (keyirq << 1) | (usbirq);

	// Machine external interrupts
	uint32_t hwInterrupt = ((m_mieshadow & 0x4 ? 1 : 0) && m_mstatusieshadow && m_irqstate) ? 1 : 0;

	m_irq = (timerInterrupt << 1) | (hwInterrupt);
}

void CCSRMem::Read(uint32_t address, uint32_t& data)
{
	uint32_t csrindex = (address >> 2) & 0xFFF;
	//if (csrindex == CSR_MSCRATCH)
	//	__debugbreak();

	if (csrindex == CSR_MHARTID)
		data = m_hartid;
	else if (csrindex == CSR_RETIHI)
		data = (uint32_t)((m_retired & 0xFFFFFFFF00000000U) >> 32);
	else if (csrindex == CSR_TIMEHI)
		data = (uint32_t)((m_wallclocktime & 0xFFFFFFFF00000000U) >> 32);
	else if (csrindex == CSR_CYCLEHI)
		data = (uint32_t)((m_cycle & 0xFFFFFFFF00000000U) >> 32);
	else if (csrindex == CSR_RETILO)
		data = (uint32_t)(m_retired & 0x00000000FFFFFFFFU);
	else if (csrindex == CSR_TIMELO)
		data = (uint32_t)(m_wallclocktime & 0x00000000FFFFFFFFU);
	else if (csrindex == CSR_CYCLELO)
		data = (uint32_t)(m_cycle & 0x00000000FFFFFFFFU);
	else if (csrindex == CSR_HWSTATE)
		data = m_irqstate;
	else if (csrindex == CSR_PROGRAMCOUNTER)
		data = m_pc;
	else
		data = m_csrmem[csrindex];
}

void CCSRMem::Write(uint32_t address, uint32_t word, uint32_t wstrobe)
{
	uint32_t csrindex = (address >> 2) & 0xFFF;
	//if (csrindex == CSR_MSCRATCH)
	//	__debugbreak();

	m_csrmem[csrindex] = word;

	// Shadow
	if (csrindex == CSR_CPURESET)
		m_cpuresetreq = word&1;
	else if (csrindex == CSR_TIMECMPLO)
		m_timecmpshadow = (m_timecmpshadow & 0xFFFFFFFF00000000U) | ((uint64_t)word);
	else if (csrindex == CSR_TIMECMPHI)
		m_timecmpshadow = ((uint64_t)word << 32) | (m_timecmpshadow & 0x00000000FFFFFFFFU);
	else if (csrindex == CSR_MEPC)
		m_mepcshadow = word;
	else if (csrindex == CSR_MIE)
		m_mieshadow = (SelectBitRange(word, 11, 11) << 2) | (SelectBitRange(word, 7, 7) << 1) | SelectBitRange(word, 3, 3); // Only timer, software, and external interrupt state is shadowed
	else if (csrindex == CSR_MSTATUS)
		m_mstatusieshadow = SelectBitRange(word, 3, 3); // Only ie bit is shadowed
	else if (csrindex == CSR_MTVEC)
		m_mtvecshadow = word;
}
