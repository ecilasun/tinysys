#include <inttypes.h>

// 1024 bytes
#define RINGBUFFER_BASE 0x00000200
#define RINGBUFFER_END  0x00000600

void RingBufferReset();
uint32_t RingBufferRead(void* pvDest, const uint32_t cbDest);
uint32_t RingBufferWrite(const void* pvSrc, const uint32_t cbSrc);
