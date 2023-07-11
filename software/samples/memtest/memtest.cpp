#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "basesystem.h"
#include "usbserial.h"
#include "memtest.h"

int main()
{
    USBSerialWrite("\nTesting DDR3 on AXI4 bus\n");

    uint8_t *testbuffer = (uint8_t*)malloc(0x02000000);

    USBSerialWrite("Clearing memory\n");
    uint64_t startclock = E32ReadTime();
    for (uint32_t m=0; m<0x02000000; m+=4)
        *((volatile uint32_t*)testbuffer) = 0x00000000;

    uint64_t endclock = E32ReadTime();
    uint32_t deltams = ClockToMs(endclock-startclock);
    USBSerialWrite("Clearing 32Mbytes took ");
    USBSerialWriteDecimal((unsigned int)deltams);
    USBSerialWrite(" ms\n");

    int rate = (1024*32*1024) / deltams;
    USBSerialWrite("Zero-write rate is ");
    USBSerialWriteDecimal(rate);
    USBSerialWrite(" Kbytes/sec\n");

    USBSerialWrite("\n-------------MemTest--------------\n");
    USBSerialWrite("Copyright (c) 2000 by Michael Barr\n");
    USBSerialWrite("----------------------------------\n");

    USBSerialWrite("Data bus test...");
    int failed = 0;
    volatile datum* tbuf = (volatile datum*)testbuffer;
    for (uint32_t i=0; i<262144; i+=4)
    {
        failed += memTestDataBus(tbuf);
        tbuf++;
    }
    USBSerialWrite(failed == 0 ? "passed (" : "failed (");
    USBSerialWriteDecimal(failed);
    USBSerialWrite(" failures)\n");

    USBSerialWrite("Address bus test (0x0B000000-0x0B03FFFF)...");
    int errortype = 0;
    datum* res = memTestAddressBus((volatile datum*)testbuffer, 262144, &errortype);
    USBSerialWrite(res == NULL ? "passed\n" : "failed\n");
    if (res != NULL)
    {
        if (errortype == 0)
            USBSerialWrite("Reason: Address bit stuck high at 0x");
        if (errortype == 1)
            USBSerialWrite("Reason: Address bit stuck low at 0x");
        if (errortype == 2)
            USBSerialWrite("Reason: Address bit shorted at 0x");
        USBSerialWriteHex((unsigned int)res);
        USBSerialWrite("\n");
    }

    USBSerialWrite("Memory device test (0x0C000000-0x0C03FFFF)...");
    datum* res2 = memTestDevice((volatile datum *)testbuffer, 262144);
    USBSerialWrite(res2 == NULL ? "passed\n" : "failed\n");
    if (res2 != NULL)
    {
        USBSerialWrite("Reason: incorrect value read at 0x");
        USBSerialWriteHex((unsigned int)res2);
        USBSerialWrite("\n");
    }

    if ((failed != 0) | (res != NULL) | (res2 != NULL))
      USBSerialWrite("Memory device does not appear to be working correctly.\n");

    free(testbuffer);

    return 0;
}
