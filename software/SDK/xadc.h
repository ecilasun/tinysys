#pragma once

#include <inttypes.h>

uint32_t ADCGetRawTemperature();

float ADCRawTemperatureToCentigrade(uint32_t rawtemp);
