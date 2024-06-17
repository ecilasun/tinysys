#include <stdio.h>
#include "emulator.h"

bool CEmulator::Reset(const char* romFile)
{
	if (m_rombin)
		delete []m_rombin;

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
		fpos_t fpos;
		fgetpos(fp, &fpos);
		size_t filesize = (size_t)fpos;
		fseek(fp, 0, SEEK_SET);

		m_rombin = new uint8_t[filesize];
		m_romsize = (uint32_t)filesize;
		fread(m_rombin, 1, filesize, fp);
		fclose(fp);
	}

	m_cpu[0] = new CRV32(0);
	m_cpu[1] = new CRV32(1);
	m_bus.Reset(m_cpu[0]->m_resetvector, m_rombin, m_romsize);
	m_cpu[0]->Reset();
	m_cpu[1]->Reset();

	return true;
}

bool CEmulator::Step()
{
	uint32_t irq0, irq1;
	m_bus.Tick(m_cpu[0], m_cpu[1], irq0, irq1);

	bool ret0 = m_cpu[0]->Tick(m_bus, irq0);
	bool ret1 = true;// m_cpu[1]->Tick(m_bus, irq1);

	m_steps++;

	return ret0 && ret1;
}

void CEmulator::UpdateVideoLink(uint32_t *pixels, int pitch)
{
	m_bus.UpdateVideoLink(pixels, pitch);
}

void CEmulator::QueueBytes(uint8_t *bytes, uint32_t count)
{
	for (uint32_t i = 0; i < count; ++i)
		m_bus.QueueByte(bytes[i]);
}

void CEmulator::QueueByte(uint8_t byte)
{
	m_bus.QueueByte(byte);
}