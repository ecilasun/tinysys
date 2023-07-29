#include "basesystem.h"
#include "xadc.h"

// Sampled data for analog CH0..CH7 (channel addresses are word aligned)
// Read as ANALOGINPUTS[0] to ANALOGINPUTS[7]
volatile uint32_t *ANALOGINPUTS = (volatile uint32_t* ) DEVICE_XADC;

// Device temperature from the on-chip ADC
volatile uint32_t *XADCTEMP = (volatile uint32_t* ) (DEVICE_XADC+0x20);

uint32_t ADCGetRawTemperature()
{
    return *XADCTEMP;
}

float ADCRawTemperatureToCentigrade(uint32_t rawtemp)
{
    return (rawtemp*503.975f)/4096.f-273.15f;
}

uint32_t ADCGetAnalogInput(const uint8_t _inputIndex)
{
    return ANALOGINPUTS[_inputIndex];
}
