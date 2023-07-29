#include "basesystem.h"
#include "leds.h"
#include "encoding.h"

// When started at boot time, this code will have
// no access to base facilities therefore it'll
// have raw access to all of the available hardware.
// This also means no runtime library, to syscalls
// and absolutely nothing to hold your hand.
//
// If started as a regular executable, the only
// way to detect that we have an active ROM is
// by querying the interrupt states since ROM
// enables some for hardware events and to run
// its scheduler.

int main()
{
    int counter = 0;

    // Check hardware interrupt state
    uint32_t state = read_csr(mie);

    if ((state & (MIP_MSIP | MIP_MEIP | MIP_MTIP)) != 0)
    {
        // Launched from ROM? Abort
        return 0;
    }
    else
    {
        // Direct access to hardware? Spin
        *LEDSTATE = counter++;
    }

    while(1){
        asm volatile("nop;");
    }
}
