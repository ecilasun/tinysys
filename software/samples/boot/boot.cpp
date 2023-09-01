#include "basesystem.h"
#include "leds.h"
#include "encoding.h"
#include <stdio.h>

int main()
{
	// Check hardware interrupt state
	uint32_t state = read_csr(mie);

	if ((state & (MIP_MSIP | MIP_MEIP | MIP_MTIP)) != 0)
	{
		// Launched from ROM? Abort
		return 0;
	}
	else
	{
		// Direct access to hardware? Copy ROM image and branch to it
		/*for (uint32_t i=0;i<sizeof(payload);++i)
		{
			// Copy to ROM starting at 0x0ffe0000
		}*/

		// TODO: So that we don't go into an endless loop, we need to write something
		// special to a memory location or CSR register that will stop a second load
		// of the ROM from storage, until a hard reset occurs.
		asm volatile("call 0x0ffe0000;");
	}
}
