#pragma once

#include <inttypes.h>

uint32_t ADCGetRawTemperature();
uint32_t ADCGetAnalogInput(const uint8_t _inputIndex);

float ADCRawTemperatureToCentigrade(uint32_t rawtemp);
