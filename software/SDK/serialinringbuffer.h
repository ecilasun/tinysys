#include <inttypes.h>
#include "basesystem.h"

void SerialInRingBufferReset();
uint32_t SerialInRingBufferRead(void* pvDest, const uint32_t cbDest);
uint32_t SerialInRingBufferWrite(const void* pvSrc, const uint32_t cbSrc);
