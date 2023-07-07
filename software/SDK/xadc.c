#include "basesystem.h"
#include "xadc.h"

// Sampled data for analog CH0..CH7 (channel addresses are word aligned)
// Read as ANALOGINPUTS[0] to ANALOGINPUTS[7]
volatile uint32_t *ANALOGINPUTS = (volatile uint32_t* ) DEVICE_XADC;

// Device temperature from the on-chip ADC
volatile uint32_t *XADCTEMP = (volatile uint32_t* ) (DEVICE_XADC+0x20);

// Individual inputs
/*
volatile uint32_t *ANALOGCH1 = (volatile uint32_t* )0x80005000;
volatile uint32_t *ANALOGCH2 = (volatile uint32_t* )0x80005004;
volatile uint32_t *ANALOGCH3 = (volatile uint32_t* )0x80005008;
volatile uint32_t *ANALOGCH4 = (volatile uint32_t* )0x8000500C;
volatile uint32_t *ANALOGCH5 = (volatile uint32_t* )0x80005010;
volatile uint32_t *ANALOGCH6 = (volatile uint32_t* )0x80005014;
volatile uint32_t *ANALOGCH7 = (volatile uint32_t* )0x80005018;
volatile uint32_t *ANALOGCH0 = (volatile uint32_t* )0x80005020;*/