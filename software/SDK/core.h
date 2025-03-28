#pragma once

#include <inttypes.h>
#include <sys/types.h>

#include <encoding.h>

#if defined(BUILDING_ROM)

// syscall handlers for ROM
uint32_t core_brk(uint32_t brkptr);
uint32_t core_sbrk(uint32_t incr);
uint32_t core_memavail();
void set_elf_heap(uint32_t heaptop);

#else // Non-ROM

#ifdef __cplusplus
extern "C" {
#endif
	char *getcwd(char *buf, size_t size);
	int chdir(const char *path);
#ifdef __cplusplus
}
#endif

#endif

#define E32AlignUp(_x_, _align_) ((_x_ + (_align_ - 1)) & (~(_align_ - 1)))

// Flush data cache to memory
#define CFLUSH_D_L1 asm volatile( ".word 0xFC000073;" : : : "memory" )
// Discard data cache contents
#define CDISCARD_D_L1 asm volatile( ".word 0xFC200073;" : : : "memory" )
// Invalidate instruction cache
#define FENCE_I asm volatile( "fence.i;" )
