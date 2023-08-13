#include <inttypes.h>
#include "basesystem.h"

void RingBufferReset();
uint32_t RingBufferRead(void* pvDest, const uint32_t cbDest);
uint32_t RingBufferWrite(const void* pvSrc, const uint32_t cbSrc);
