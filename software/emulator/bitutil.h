#pragma once

#include <stdint.h>

#if defined(CAT_WINDOWS)
#include <immintrin.h> // Include for _bextr_u32 or _bextr_u64
#endif

#if defined(CAT_WINDOWS)
#define FINLINE __forceinline
#else
#define FINLINE inline
#endif

// This will select the bit range and right align it
FINLINE uint32_t SelectBitRange(uint32_t val, uint32_t startbit, uint32_t endbit)
{
	// XOP.LZ.0A 10 /r id	BEXTR	Bit field extract (with immediate)	(src >> start) & ((1 << len) - 1)
	#if defined(CAT_WINDOWS)
		return _bextr_u32(val, endbit, startbit - endbit + 1);
	#else
		// Calculate the number of bits to extract
		uint32_t numBits = (startbit - endbit) + 1;
		// Directly create the mask by shifting 1 left by the number of bits to extract, then subtract 1
		uint32_t mask = (1U << numBits) - 1;
		// Apply the mask after shifting the value to the right by the endbit position
		uint32_t retval = (val >> endbit) & mask;
		return retval;
	#endif
}
