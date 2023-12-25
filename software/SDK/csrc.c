#include "basesystem.h"
#include "csrc.h"

volatile uint32_t *CSRMEM0 = (volatile uint32_t* ) DEVICE_CSR0; // HART#0
