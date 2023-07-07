#pragma once

#include <inttypes.h>

// R/W port for LED status access
extern volatile uint32_t *LEDSTATE;

// Bit masks for individual LEDs
#define BUTTONMASK_CENTER 0x00000001
#define BUTTONMASK_DOWN 0x00000002
#define BUTTONMASK_LEFT 0x00000004
#define BUTTONMASK_RIGHT 0x00000008
#define BUTTONMASK_UP 0x00000010

uint32_t LEDGetState();
void LEDSetState(const uint32_t state);
