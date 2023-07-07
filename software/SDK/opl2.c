#include "opl2.h"
#include "basesystem.h"

volatile uint8_t *IO_OPL2REG = (volatile uint8_t*) DEVICE_OPL2;
volatile uint8_t *IO_OPL2VAL = (volatile uint8_t*) (DEVICE_OPL2+4);

void OPL2WriteReg(uint8_t reg)
{
    *IO_OPL2REG = reg;
	E32Sleep(200);	// Need to give time for internal processing
}

void OPL2WriteVal(uint8_t val)
{
	*IO_OPL2VAL = val;
	E32Sleep(200);	// Need to give time for internal processing
}

void OPL2Stop()
{
	for (int channel = 0x0; channel <= 0x8; ++channel)
	{
		OPL2WriteReg(0xB0 + channel);
		OPL2WriteVal(0x00);
	}
}