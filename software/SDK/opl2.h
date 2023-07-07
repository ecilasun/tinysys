#pragma once

#include <inttypes.h>

extern volatile uint8_t *IO_OPL2REG;
extern volatile uint8_t *IO_OPL2VAL;

void OPL2WriteReg(uint8_t reg);
void OPL2WriteVal(uint8_t val);
void OPL2Stop();