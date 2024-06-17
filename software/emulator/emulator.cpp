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

	m_bus.Reset(m_cpu.m_resetvector, m_rombin, m_romsize);
	m_cpu.Reset();

	return true;
}

bool CEmulator::Step()
{
	uint32_t irq = m_bus.Tick(&m_cpu, &m_cpu.m_sie); // TODO: Pass list of all CPUs on the system
	bool retval = m_cpu.Tick(m_bus, irq);

	return retval;
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