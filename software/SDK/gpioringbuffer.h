#include <inttypes.h>
#include "basesystem.h"

void GPIORingBufferReset();
uint32_t GPIORingBufferRead(void* pvDest, const uint32_t cbDest);
uint32_t GPIORingBufferWrite(const void* pvSrc, const uint32_t cbSrc);
