// Embedded utils

#include "core.h"
#include "basesystem.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
//#include <unistd.h>

#if defined(BUILDING_ROM)

// TODO: __heap_start should be set to __bss_end and __heap_end to __heap_start+__heap_size for loaded executables
// This also means we have to get rid of any addresses between bss and heap as they might get overwritten
static uint8_t* __heap_start = (uint8_t*)HEAP_START_APPMEM_END;
static uint8_t* __heap_end = (uint8_t*)HEAP_END_CONSOLEMEM_START;
static uint8_t* __breakpos = (uint8_t*)HEAP_START_APPMEM_END;

void set_elf_heap(uint32_t heaptop)
{
	__breakpos = (uint8_t*)heaptop;
	__heap_start = (uint8_t*)heaptop;
}

uint32_t core_memavail()
{
	return (uint32_t)(__heap_end - __breakpos);
}

uint32_t core_brk(uint32_t brkptr)
{
	// Address set to zero will query current break position
	if (brkptr == 0)
		return (uint32_t)__breakpos;

	// NOTE: The break address is aligned to the next multiple of 4 bytes
	uint32_t alignedbrk = E32AlignUp(brkptr, 4);

	// Out of bounds will return all ones (-1)
	if (alignedbrk<(uint32_t)__heap_start || alignedbrk>(uint32_t)__heap_end)
	{
		errno = ENOMEM;
		return 0xFFFFFFFF;
	}

	// Set new break position and return 0 (success) in all other cases
	__breakpos = (uint8_t*)alignedbrk;
	return 0;
}

#else

#endif

#ifdef __cplusplus
extern "C" {
#endif

	char *getcwd(char *buf, size_t size)
	{
		uint32_t fdsc = (uint32_t)buf;
		uint32_t fsta = (uint32_t)size;
		char* retval = 0;
		asm (
			"li a7, 17;"
			"mv a0, %1;"
			"mv a1, %2;"
			"ecall;"
			"mv %0, a0;" :
			// Return values
			"=r" (retval) :
			// Input parameters
			"r" (fdsc), "r" (fsta) :
			// Clobber list
			"a0", "a1", "a7"
		);
		return retval;
	}

	int chdir(const char *path)
	{
		uint32_t fdsc = (uint32_t)path;
		int retval = 0;
		asm (
			"li a7, 50;"
			"mv a0, %1;"
			"ecall;"
			"mv %0, a0;" :
			// Return values
			"=r" (retval) :
			// Input parameters
			"r" (fdsc) :
			// Clobber list
			"a0", "a7"
		);
		return retval;
	}


#if defined(BUILDING_ROM)
	// ROM should not be allocating memory on the fly but if it does, not via ecalls
	int _brk(void *addr)
	{
		return core_brk((uint32_t)addr);
	}
#else
	int _brk(void *addr)
	{
		uint32_t brkaddr = (uint32_t)addr;
		int retval = 0;
		asm (
			"li a7, 214;"
			"mv a0, %1;"
			"ecall;"
			"mv %0, a0;" :
			// Return values
			"=r" (retval) :
			// Input parameters
			"r" (brkaddr) :
			// Clobber list
			"a0", "a7"
		);
		return retval;
	}
#endif

	void *_sbrk(intptr_t incr)
	{
		uint8_t *old_heapstart = (uint8_t *)_brk(0);
		uint32_t res = _brk(old_heapstart + incr);
		return res != 0xFFFFFFFF ? old_heapstart : NULL;
	}

#ifdef __cplusplus
}
#endif
