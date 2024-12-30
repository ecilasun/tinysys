#pragma once

#include <inttypes.h>

// R/W port for LED status access
extern volatile uint32_t *LEDSTATE;

uint32_t LEDGetState();
void LEDSetState(const uint32_t state);