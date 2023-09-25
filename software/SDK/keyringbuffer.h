#include <inttypes.h>
#include "basesystem.h"

void KeyRingBufferReset();
uint32_t KeyRingBufferRead(void* pvDest, const uint32_t cbDest);
uint32_t KeyRingBufferWrite(const void* pvSrc, const uint32_t cbSrc);
