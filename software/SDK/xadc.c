#include "basesystem.h"
#include "xadc.h"

// Device temperature from the on-chip ADC
volatile uint32_t *XADCTEMP = (volatile uint32_t* )DEVICE_XADC;

uint32_t ADCGetRawTemperature()
{
    return *XADCTEMP;
}

float ADCRawTemperatureToCentigrade(uint32_t rawtemp)
{
    return (rawtemp*503.975f)/4096.f-273.15f;
}
