#include "gpioringbuffer.h"
#include "encoding.h"
#include <string.h>

// Simple ringbuffer
// Adapted from DXUT locklesspipe (c) Microsoft

const static uint32_t cbBufferSizeLog2 = 14; // 16384
const static uint8_t c_cbBufferSizeLog2 = cbBufferSizeLog2 < 31 ? cbBufferSizeLog2 : 31;
const static uint32_t c_cbBufferSize = ( 1 << c_cbBufferSizeLog2 );
const static uint32_t c_sizeMask = c_cbBufferSize - 1;

// These need to persist in same memory location betwen ROM and loaded ELF
// so that we don't read from wrong space (or not read at all)
// NOTE: Ring buffer is normally placed at 0x80000200 in the mailbox
volatile uint32_t *m_gpio_readOffset  = (volatile uint32_t*)(GPIO_RINGBUFFER_STATE+4);
volatile uint32_t *m_gpio_writeOffset = (volatile uint32_t*)(GPIO_RINGBUFFER_STATE+8);

void __attribute__ ((noinline)) GPIORingBufferReset()
{
    *m_gpio_readOffset  = 0;
    *m_gpio_writeOffset = 0;
}

void __attribute__ ((noinline)) GPIORingBufferFlush()
{
	// Disable machine interrupts
	write_csr(mstatus, 0);
	// Reset FIFO
	GPIORingBufferReset();
	// Enable machine interrupts
	write_csr(mstatus, MSTATUS_MIE);
}

uint32_t __attribute__ ((noinline)) GPIORingBufferRead(void* pvDest, const uint32_t cbDest)
{
    uint8_t *ringbuffer = (uint8_t *)GPIO_RINGBUFFER_BASE;

    uint32_t readOffset = *m_gpio_readOffset;
    const uint32_t writeOffset = *m_gpio_writeOffset;

    const uint32_t cbAvailable = writeOffset - readOffset;
    if( cbDest > cbAvailable )
        return 0;

    //EReadWriteBarrier(0);
    asm volatile ("nop;"); // Stop compiler reordering

    uint8_t* pbDest = (uint8_t *)pvDest;
    const uint32_t actualReadOffset = readOffset & c_sizeMask;
    uint32_t bytesLeft = cbDest;

    const uint32_t cbTailBytes = bytesLeft < c_cbBufferSize - actualReadOffset ? bytesLeft : c_cbBufferSize - actualReadOffset;
    __builtin_memcpy( pbDest, ringbuffer + actualReadOffset, cbTailBytes );
    bytesLeft -= cbTailBytes;

    //EAssert(bytesLeft == 0, "Item not an exact multiple of ring buffer, this will cause multiple memcpy() calls during Read()");

    readOffset += cbDest;
    *m_gpio_readOffset = readOffset;

    return 1;
}

uint32_t __attribute__ ((noinline)) GPIORingBufferWrite( const void* pvSrc, const uint32_t cbSrc)
{
    uint8_t *ringbuffer = (uint8_t *)GPIO_RINGBUFFER_BASE;

    const uint32_t readOffset = *m_gpio_readOffset;
    uint32_t writeOffset = *m_gpio_writeOffset;

    const uint32_t cbAvailable = c_cbBufferSize - ( writeOffset - readOffset );
    if( cbSrc > cbAvailable )
        return 0;

    const uint8_t* pbSrc = ( const uint8_t* )pvSrc;
    const uint32_t actualWriteOffset = writeOffset & c_sizeMask;
    uint32_t bytesLeft = cbSrc;

    const uint32_t cbTailBytes = bytesLeft < c_cbBufferSize - actualWriteOffset ? bytesLeft : c_cbBufferSize - actualWriteOffset;
    __builtin_memcpy(ringbuffer + actualWriteOffset, pbSrc, cbTailBytes);
    bytesLeft -= cbTailBytes;

    //EAssert(bytesLeft == 0, "Item not an exact multiple of ring buffer, this will cause multiple memcpy() calls during Write()");

    //EReadWriteBarrier(0);
    asm volatile ("nop;"); // Stop compiler reordering

    writeOffset += cbSrc;
    *m_gpio_writeOffset = writeOffset;

    return 1;
}
