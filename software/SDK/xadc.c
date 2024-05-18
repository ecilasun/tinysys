/** @file xadc.c
 * 
 *  @brief A/DC interface
 *
 *  This file contains interfaces to the temperature sensor on board.
*/

#include "basesystem.h"
#include "xadc.h"

// Device temperature from the on-chip ADC
volatile uint32_t *XADCTEMP = (volatile uint32_t* )DEVICE_XADC;

/**
 * @brief Get the raw temperature from the on-chip ADC
 * 
 * @return uint32_t The raw temperature value
 */
uint32_t ADCGetRawTemperature()
{
    return *XADCTEMP;
}

/**
 * @brief Convert the raw temperature value to Centigrade
 * 
 * @param rawtemp The raw temperature value
 * @return float The temperature in centigrade
 */
float ADCRawTemperatureToCentigrade(uint32_t rawtemp)
{
    return (rawtemp*503.975f)/4096.f-273.15f;
}
