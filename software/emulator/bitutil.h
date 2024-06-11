#pragma once

#include <stdint.h>

#if defined(CAT_WINDOWS) || defined(CAT_LINUX)
#define FINLINE __forceinline
#else
#define FINLINE inline
#endif

// This will select the bit range and right align it
FINLINE uint32_t SelectBitRange(uint32_t val, uint32_t startbit, uint32_t endbit)
{
	// XOP.LZ.0A 10 /r id	BEXTR	Bit field extract (with immediate)	(src >> start) & ((1 << len) - 1)

	// get number of bits covered; delta = (start-end)+1
	uint32_t delta = (startbit-endbit)+1;
	// logic shift right by endbit
	uint32_t shifted = val >> endbit;
	// prep inverse mask: 0x80000000 >>> (31-delta)
	uint32_t invmask = ((int32_t)0x80000000) >> (31-delta);
	// prep actual mask
	uint32_t mask = ~invmask;
	// final return value
	uint32_t retval = shifted & mask;
	return retval;
}
