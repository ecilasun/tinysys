#include "basesystem.h"
#include "uart.h"

#include <math.h> // For abs()

// Status register bits
// uartstateregister: {30'd0, fifoFull, inFifohasData};

volatile uint32_t *IO_UARTRX     = (volatile uint32_t* ) DEVICE_UART; // Receive fifo
volatile uint32_t *IO_UARTTX     = (volatile uint32_t* ) (DEVICE_UART+0x4); // Transmit fifo
volatile uint32_t *IO_UARTStatus = (volatile uint32_t* ) (DEVICE_UART+0x8); // Status register
volatile uint32_t *IO_UARTCtl    = (volatile uint32_t* ) (DEVICE_UART+0xC); // Control register

void UARTEnableInterrupt(int enable)
{
    *IO_UARTCtl = enable ? 0x00000010 : 0x00000000;
}

int UARTInputFifoHasData()
{
    // bit0: RX FIFO has valid data
    return ((*IO_UARTStatus)&0x00000001); // inFifohasData
}

void UARTDrainInput()
{
    while (UARTInputFifoHasData()) { asm volatile("nop;"); }
}

void UARTFlushOutput()
{
    // TODO:
    //while (UARTOutputFifoHasData()) { asm volatile("nop;"); }
}

uint8_t UARTRead()
{
    if (UARTInputFifoHasData())
        return (uint8_t)(*IO_UARTRX);
    else
        return 0;
}

void UARTWrite(const char *_message)
{
    // Emit all characters from the input string
    int i = 0;
    while (_message[i]!=0)
        *IO_UARTTX = (uint32_t)_message[i++];
}

void UARTWriteHexByte(const uint8_t i)
{
    const char hexdigits[] = "0123456789ABCDEF";
    char msg[] = "  ";

    msg[0] = hexdigits[((i>>4)%16)];
    msg[1] = hexdigits[(i%16)];

    UARTWrite(msg);
}

void UARTWriteHex(const uint32_t i)
{
    const char hexdigits[] = "0123456789ABCDEF";
    char msg[] = "        ";

    msg[0] = hexdigits[((i>>28)%16)];
    msg[1] = hexdigits[((i>>24)%16)];
    msg[2] = hexdigits[((i>>20)%16)];
    msg[3] = hexdigits[((i>>16)%16)];
    msg[4] = hexdigits[((i>>12)%16)];
    msg[5] = hexdigits[((i>>8)%16)];
    msg[6] = hexdigits[((i>>4)%16)];
    msg[7] = hexdigits[(i%16)];

    UARTWrite(msg);
}

void UARTWriteDecimal(const int32_t i)
{
    const char digits[] = "0123456789";
    char msg[] = "                   ";

    int d = 1000000000;
    uint32_t enableappend = 0;
    uint32_t m = 0;
    if (i<0)
        msg[m++] = '-';
    for (int c=0;c<10;++c)
    {
        uint32_t r = ((i/d)%10)&0x7FFFFFFF;
        // Ignore preceeding zeros
        if ((r!=0) || enableappend || d==1)
        {
            enableappend = 1; // Rest of the digits can be appended
            msg[m++] = digits[r];
        }
        d = d/10;
    }
    msg[m] = 0;

    UARTWrite(msg);
}
