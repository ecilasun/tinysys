/**
 * @file basesystem.c
 * 
 * @brief Base system interface.
 *
 * This file provides functions for interacting with the base system.
 * It includes functions for converting clock milliseconds to hours, minutes, and seconds,
 * setting time compare, reading and writing memory-mapped CSR, and setting up and resetting CPU.
 */

#include "basesystem.h"
#include "encoding.h"
#include <stdlib.h>

// Utilities

/**
 * @brief Returns the current time of calling CPU
 * 
 * This function reads the current time from the timer CSR and returns it as a 64-bit unsigned integer in ticks.
 * To read another CPU's time, access the memory-mapped CSR file for that specific CPU instead.
 *
 * @return The current time in ticks.
 */
uint64_t E32ReadTime()
{
   uint32_t clockhigh, clocklow, tmp;
   asm volatile(
      "1:\n"
      "rdtimeh %0\n"
      "rdtime %1\n"
      "rdtimeh %2\n"
      "bne %0, %2, 1b\n"
      : "=&r" (clockhigh), "=&r" (clocklow), "=&r" (tmp)
   );

   uint64_t now = ((uint64_t)(clockhigh)<<32) | clocklow;
   return now;
}

/**
 * @brief Returns the current cycle count of calling CPU.
 * 
 * This function reads the current CPU cycle count from the cycle CSR and returns it as a 64-bit unsigned integer.
 * To read another CPU's cycle counter, access the memory-mapped CSR file for that specific CPU instead.
 *
 * @return The current cycle count.
 */
uint64_t E32ReadCycles()
{
   uint32_t cyclehigh, cyclelow, tmp;
   asm volatile(
      "1:\n"
      "rdcycleh %0\n"
      "rdcycle %1\n"
      "rdcycleh %2\n"
      "bne %0, %2, 1b\n"
      : "=&r" (cyclehigh), "=&r" (cyclelow), "=&r" (tmp)
   );

   uint64_t now = ((uint64_t)(cyclehigh)<<32) | cyclelow;
   return now;
}

/**
 * @brief Sleeps for the specified number of ticks.
 * 
 * This function sleeps for the specified number of ticks.
 * It is a busy-wait sleep function, therefore will not yield to other tasks unless the scheduler does so.
 *
 * @param ticks The number of ticks to sleep for.
 */
void E32Sleep(uint64_t ticks)
{
   // Start time is now in ticks
   uint64_t tstart = E32ReadTime();
   // End time is now plus ms in ticks
   uint64_t tend = tstart + ticks;
   while (E32ReadTime() < tend) { }
}

/**
 * @brief Reads the retired instructions count from the retired instructions CSR and returns it as a 64-bit unsigned integer.
 *
 * @return The retired instructions count.
 */
uint64_t E32ReadRetiredInstructions()
{
   uint32_t retihigh, retilow;

   asm (
      "rdinstreth %0;"
      "rdinstret %1;"
      : "=&r" (retihigh), "=&r" (retilow)
   );

   uint64_t reti = ((uint64_t)(retihigh)<<32) | retilow;

   return reti;
}

/**
 * @brief Converts clock from ticks to milliseconds.
 *
 * @param clk The raw clock value in ticks to convert.
 */
uint32_t ClockToMs(uint64_t clk)
{
   return (uint32_t)(clk / ONE_MILLISECOND_IN_TICKS);
}

/**
 * @brief Converts clock from ticks to microseconds.
 *
 * @param clk The raw clock value in ticks to convert.
 */
uint32_t ClockToUs(uint64_t clk)
{
   return (uint32_t)(clk / ONE_MICROSECOND_IN_TICKS);
}

/**
 * @brief Converts milliseconds to hours, minutes, and seconds.
 *
 * @param ms The milliseconds to convert.
 * @param hours The hours to return.
 * @param minutes The minutes to return.
 * @param seconds The seconds to return.
 */
void ClockMsToHMS(uint32_t ms, uint32_t *hours, uint32_t *minutes, uint32_t *seconds)
{
   *hours = ms / 3600000;
   *minutes = (ms % 3600000) / 60000;
   *seconds = ((ms % 360000) % 60000) / 1000;
}

/**
 * @brief Sets the time compare CSR so that the timer interrupt will fire at the specified future time.
 *
 * @param future The future time to compare against.
 */
void E32SetTimeCompare(const uint64_t future)
{
   // NOTE: ALWAYS set high word first to avoid misfires outside timer interrupt
   swap_csr(0x801, ((future&0xFFFFFFFF00000000)>>32));         // CSR_TIMECMPHI
   swap_csr(0x800, ((uint32_t)(future&0x00000000FFFFFFFF)));   // CSR_TIMECMPLO
}

/**
 * @brief Writes a value to a memory-mapped CSR at the specified hart and CSR. The value can be read by other devices with bus access.
 *
 * @param _hart The hart to write to.
 * @param _csr The CSR to write to.
 * @param _value The value to write.
 */
void E32WriteMemMappedCSR(uint32_t _hart, uint32_t _csr, uint32_t _value)
{
	if (_hart >= MAX_HARTS) return;

	uint32_t csrbase[MAX_HARTS] = {DEVICE_CSR0, DEVICE_CSR1};
	*(uint32_t*)(csrbase[_hart] | (_csr<<2)) = _value;
}

/**
 * @brief Reads a value from a memory-mapped CSR and returns it. It can be written by other devices with bus access.
 *
 * @param _hart The hart to read from.
 * @param _csr The CSR to read from.
 * @return The read value.
 */
uint32_t E32ReadMemMappedCSR(uint32_t _hart, uint32_t _csr)
{
	if (_hart >= MAX_HARTS) return 0;
	// Return the address of the CSR for the given hart
	uint32_t csrbase[MAX_HARTS] = {DEVICE_CSR0, DEVICE_CSR1};
	volatile uint32_t *csrAddress = (uint32_t*)(csrbase[_hart] | (_csr<<2));
	return *csrAddress;
}

/**
 * @brief Sets up a CPU in preparation for a reset. Call this before calling E32ResetCPU.
 *
 * @param hartid The ID of the hart to set up.
 * @param workerThread The worker thread to set up.
 */
void E32SetupCPU(uint32_t hartid, void *workerThread)
{
	if (hartid >= MAX_HARTS) return;
	// Set up reset vector as if it's an ISR
	E32WriteMemMappedCSR(hartid, CSR_MSCRATCH, (uint32_t)workerThread);
}

/**
 * @brief Resets a CPU to start executing the supplied task.
 *
 * @param hartid The ID of the hart to reset.
 */
void E32ResetCPU(uint32_t hartid)
{
	if (hartid >= MAX_HARTS) return;
	// This triggers a hardware jump to mscratch after all queued instructions execute
	E32WriteMemMappedCSR(hartid, CSR_CPURESET, 0x1);
}

/**
 * @brief Clears the reset state of a CPU.
 *
 * @param hartid The ID of the hart to clear the reset state for.
 */
void E32ClearReset(uint32_t hartid)
{
	if (hartid >= MAX_HARTS) return;
	// This clears the reset state of the CPU
	E32WriteMemMappedCSR(hartid, CSR_CPURESET, 0x0);
}

/**
 * @brief Begins a critical section.
 * 
 * This function begins a critical section by disabling the machine timer interrupt.
 */
void E32BeginCriticalSection()
{
	clear_csr(mie, MIP_MTIP);
}

/**
 * @brief Ends a critical section.
 * 
 * This function ends a critical section by enabling the machine timer interrupt.
 */
void E32EndCriticalSection()
{
	set_csr(mie, MIP_MTIP);
}

/**
 * @brief Returns the scratchpad memory address.
 * 
 * This function returns the scratchpad memory address, which is a 64 Kbytes uncached memory region shared between all cores.
 * 
 * @return The scratchpad memory address.
 */
uint32_t E32GetScratchpad()
{
	return DEVICE_SPAD;
}