/**
 * @file core.c
 * 
 * @brief Core system interface.
 *
 * This file provides functions for interacting with the core system.
 * It includes functions for setting the ELF heap, getting and setting the current memory break position.
 */

#include "core.h"
#include "basesystem.h"

#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#if defined(BUILDING_ROM)

/**
 * @brief Sets the ELF heap to the provided address.
 *
 * This function sets the start of the heap to the provided address.
 *
 * @param heaptop The top address of the heap.
 */
void set_elf_heap(uint32_t heaptop)
{
	// Both CPUs will use CPU#0's CSR registers to store this information
	E32WriteMemMappedCSR(0, CSR_BRKPOS, heaptop); // __breakpos
	E32WriteMemMappedCSR(0, CSR_HEAPSTART, heaptop); // __heap_start
}

/**
 * @brief Returns the amount of available memory.
 *
 * This function returns the amount of available memory by subtracting the current break position from the end of the heap.
 *
 * @return The amount of available memory.
 */
uint32_t core_memavail()
{
	uint32_t breakpos = E32ReadMemMappedCSR(0, CSR_BRKPOS);
	uint32_t heapend = E32ReadMemMappedCSR(0, CSR_HEAPEND);
	return (uint32_t)(heapend - breakpos);
}

/**
 * @brief Sets the current memory break position.
 *
 * This function sets the break position to the provided address, or returns the current break position if the provided address is zero.
 * The break position is aligned to the next multiple of 4 bytes.
 * If the provided address is out of bounds, the function sets errno to ENOMEM and returns -1.
 * Please note that only one core has access to this at a given time
 *
 * @param brkptr The new memory break position.
 * 
 * @return 0 on success, -1 on failure.
 */
uint32_t core_brk(uint32_t brkptr)
{
	// Address set to zero will query current break position
	if (brkptr == 0)
		return E32ReadMemMappedCSR(0, CSR_BRKPOS); // __breakpos

	// NOTE: The break address is aligned to the next multiple of 4 bytes
	uint32_t alignedbrk = E32AlignUp(brkptr, 4);

	// Out of bounds will return all ones (-1)
	if (alignedbrk<E32ReadMemMappedCSR(0, CSR_HEAPSTART) || alignedbrk>E32ReadMemMappedCSR(0, CSR_HEAPEND)) // __heap_end
	{
		errno = ENOMEM;
		return 0xFFFFFFFF;
	}

	// Set new break position and return 0 (success) in all other cases
	E32WriteMemMappedCSR(0, CSR_BRKPOS, alignedbrk);
	return 0;
}

uint32_t core_sbrk(uint32_t incr)
{
	uint32_t old_heapstart = core_brk(0);
	uint32_t res = core_brk(old_heapstart + incr);
	return res != 0xFFFFFFFF ? old_heapstart : 0x00000000;
}

#else

#endif

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Get the current working directory.
 * 
 * This function gets the current working directory and stores it in the provided buffer.
 * 
 * @param buf Buffer to store the current working directory.
 * @param size Size of the buffer.
 * 
 * @return The current working directory.
 */
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

/** @brief Change the current working directory.
 * 
 * This function changes the current working directory to the provided path.
 * 
 * @param path The path to change to.
 * 
 * @return 0 on success, -1 on failure.
 */
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
/** @brief Set the break position of the heap.
 * 
 * @note This function is only available when building the ROM.
 * 
 * @param addr The new break position.
 * 
 * @return 0 on success, -1 on failure.
 */
int _brk(void *addr)
{
	return core_brk((uint32_t)addr);
}
#else
/** @brief Set the break position of the heap.
 * 
 * @param addr The new break position.
 * 
 * @return 0 on success, -1 on failure.
 */
int _brk(void *addr)
{
	// Wait for any other core to leave brk()
	while (E32ReadMemMappedCSR(0, CSR_BRKLOCK) != 0) { }
	E32WriteMemMappedCSR(0, CSR_BRKLOCK, 1); // Set the brk lock to indicate we're in brk()

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

	E32WriteMemMappedCSR(0, CSR_BRKLOCK, 0); // Clear the brk lock to indicate we're done with brk()
	return retval;
}
#endif

/** @brief Get the current break position of the heap.
 * 
 * @param incr The increment to add to the break position.
 * 
 * @return The new break position.
 */
void *_sbrk(intptr_t incr)
{
	// Wait for any other core to leave brk()
	while (E32ReadMemMappedCSR(0, CSR_BRKLOCK) != 0) { }
	E32WriteMemMappedCSR(0, CSR_BRKLOCK, 1); // Set the brk lock to indicate we're in brk()

	uint32_t brkinc = (uint32_t)incr;
	int retval = 0;
	asm (
		"li a7, 16391;"
		"mv a0, %1;"
		"ecall;"
		"mv %0, a0;" :
		// Return values
		"=r" (retval) :
		// Input parameters
		"r" (brkinc) :
		// Clobber list
		"a0", "a7"
	);

	E32WriteMemMappedCSR(0, CSR_BRKLOCK, 0); // Clear the brk lock to indicate we're done with brk()
	return (void*)retval;
}

#ifdef __cplusplus
}
#endif
