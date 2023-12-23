#pragma once

#include <inttypes.h>

// R/W access to memory mapped CSR memory for each HART (4Kbytes)
extern volatile uint32_t *CSRMEM0;
