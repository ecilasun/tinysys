#include <inttypes.h>
#include "basesystem.h"

void SerialOutRingBufferReset();
uint32_t SerialOutRingBufferRead(void* pvDest, const uint32_t cbDest);
uint32_t SerialOutRingBufferWrite(const void* pvSrc, const uint32_t cbSrc);
uint32_t SerialOutRingBufferAvailable();
