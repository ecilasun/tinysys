# Base System Utilities

The basesystem module contains useful routines to make life easier when developing software on the tinysys.

Most of these helpers relate to accessing the clock, where the rest can directly read or write the CSR registers across cores.

Care must be taken to access the device in this way as there's no protection mechanism against a malformed call causing a CPU hang (apart form sending a reset request over UART).

Following is a list of the safe routines that user can access without causing damage. For the rest of the routines, see the OS ROM code and basesystem.c file.

### Clock functions

`uint64_t E32ReadTime()`

This function will read the wall clock from the shared system clock. Note that the returned value will be in ticks and needs to be converted using one of the following functions to be useful.

---

`uint64_t E32ReadCycles()`

This function returns the number of clock cycles elapsed since boot on the riscv core it's called on.

---

`uint64_t E32ReadRetiredInstructions()`

This function returns the total number of instructions executed to completion on the riscv core it's called on.

---

`uint32_t ClockToMs(uint64_t clk)`

This function converts ticks into milliseconds. Mostly useful if you're timing a code section to report its cost, and the return values are in ticks.

---

`uint32_t ClockToUs(uint64_t clk)`

This function converts ticks into microseconds.

---

`void ClockMsToHMS(uint32_t ms, uint32_t *hours, uint32_t *minutes, uint32_t *seconds)`

This function helps convert from milliseconds to a wall clock format for human readability.

---

`void E32Sleep(uint64_t ticks)`

This function will sleep for approximately given number of ticks. To use milliseconds, multiply your value with the pre-defined `ONE_MILLISECOND_IN_TICKS` value found in the basesystem.h file. There are other pre-built constants in this file that you can use directly, for instance `ONE_SECOND_IN_TICKS` for a one second sleep.

The accuracy of the sleep depends on active interrupts and caching of the function containing the E32Sleep call.

### Scratchpad memory access

`uint32_t* E32GetScratchpad()`

This function returns the scratchpad memory address. You can use this 64Kbyte uncached memory region to store temporary data or act as a shared memory location between the CPU cores. Since there is no caching of reads or writes to this area, they complete in fixed speed, and writes are immediately visible from all cores.

### Back to [SDK Documentation](README.md)
