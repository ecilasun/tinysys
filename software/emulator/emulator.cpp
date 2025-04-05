#include <stdio.h>
#include "config.h"
#include "emulator.h"

CEmulator::~CEmulator()
{
	if (m_rombin)
		delete[] m_rombin;
	if (m_bus)
		delete m_bus;
	if (m_cpu[0]) delete m_cpu[0];
	if (m_cpu[1]) delete m_cpu[1];
}

bool CEmulator::Reset(const char* romFile, uint32_t resetvector)
{
	if (m_rombin)
	{
		delete[]m_rombin;
		m_rombin = nullptr;
	}

	if (romFile)
	{
#if defined(CAT_WINDOWS)
		FILE* fp;
		errno_t x = fopen_s(&fp, romFile, "rb");
#else
		FILE* fp = fopen(romFile, "rb");
#endif
		if (!fp)
			return false;
		fseek(fp, 0, SEEK_END);
		#if defined(CAT_LINUX)
				size_t filesize = ftello(fp);
		#else
			fpos_t fpos;
			fgetpos(fp, &fpos);
			size_t filesize = (size_t)fpos;
		#endif
		fseek(fp, 0, SEEK_SET);

		m_rombin = new uint8_t[filesize];
		m_romsize = (uint32_t)filesize;
		fread(m_rombin, 1, filesize, fp);
		fclose(fp);
	}

	if (m_bus) delete m_bus;
	m_bus = new CBus(resetvector);
	m_bus->Reset(m_rombin, m_romsize);

	if (m_cpu[0]) delete m_cpu[0];
	if (m_cpu[1]) delete m_cpu[1];
	m_cpu[0] = new CRV32(0, resetvector);
	m_cpu[1] = new CRV32(1, resetvector);
	m_cpu[0]->Reset();
	m_cpu[1]->Reset();

	return true;
}

void CEmulator::StepBus()
{
	m_bus->Tick();
}

void CEmulator::Step(uint64_t wallclock, uint32_t _hartid)
{
	++m_steps;
	m_bus->GetCSR(_hartid)->Tick(m_bus);
	m_cpu[_hartid]->Tick(wallclock, m_bus);
}

void CEmulator::UpdateVideoLink(uint32_t *pixels, int pitch)
{
	m_bus->UpdateVideoLink(pixels, pitch);
}

void CEmulator::QueueBytes(uint8_t *bytes, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		m_bus->QueueByte(bytes[i]);
}

void CEmulator::QueueByte(uint8_t byte)
{
	m_bus->QueueByte(byte);
}

void CEmulator::AddBreakpoint(uint32_t isVolatile, uint32_t cpu, uint32_t address)
{
	m_cpu[cpu]->AddBreakpoint(isVolatile, address, m_bus);
}

void CEmulator::RemoveBreakpoint(uint32_t cpu, uint32_t address)
{
	m_cpu[cpu]->RemoveBreakpoint(address, m_bus);
}

void CEmulator::RemoveAllBreakpoints(uint32_t cpu)
{
	m_cpu[cpu]->RemoveAllBreakpoints(m_bus);
}

void CEmulator::Continue(uint32_t cpu)
{
	m_cpu[cpu]->Continue(m_bus);
}

void CEmulator::StepToNext(uint32_t cpu)
{
	m_cpu[cpu]->StepToNext(m_bus);
}
