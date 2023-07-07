#include "basesystem.h"
#include "xadc.h"

volatile uint32_t *CSRMEM = (volatile uint32_t* ) DEVICE_CSRF; // HART#0
