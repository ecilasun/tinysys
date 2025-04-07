#pragma once

#include <inttypes.h>

// tinysys runs wall clock at 10MHz
// These are the number of clock ticks for various time intervals
#define ONE_SECOND_IN_TICKS						10000000
#define HALF_SECOND_IN_TICKS					5000000
#define TWO_HUNDRED_FIFTY_MILLISECONDS_IN_TICKS	2500000
#define TWO_HUNDRED_MILLISECONDS_IN_TICKS		2000000
#define HUNDRED_MILLISECONDS_IN_TICKS			1000000
#define TEN_MILLISECONDS_IN_TICKS				100000
#define TWO_MILLISECONDS_IN_TICKS				20000
#define ONE_MILLISECOND_IN_TICKS				10000
#define HALF_MILLISECOND_IN_TICKS				5000
#define QUARTER_MILLISECOND_IN_TICKS			2500
#define ONE_MICROSECOND_IN_TICKS				10

// Control and status registers for tinysy specific features
#define CSR_CPURESET					0xFEE
#define CSR_WATERMARK					0xFF0
#define CSR_PROGRAMCOUNTER				0xFFC

// Registers for heap management
#define CSR_HEAPSTART					0x900
#define CSR_HEAPEND						0x901
#define CSR_BRKPOS						0x902
#define CSR_BRKLOCK						0x903

// Physical address map for no-MMU raw mode at boot time
#define APPMEM_START					0x00000000 // Top of RAM
#define HEAP_START						0x00001000 // Application heap starts here
// 240 MBytes of application space
#define HEAP_END						0x0F000000 // Executable heap space above this, we try not to cross this boundary into task stack space

// Console buffers
#define CONSOLE_FRAMEBUFFER_START		0x0F100000 // Console framebuffer == 0x4B000 bytes max at 640*480 resolution, has to be 64K aligned
#define CONSOLE_CHARACTERBUFFER_START	0x0F14D000 // Character store == 80*60(0x12C0) bytes max at 640*480 resolution (leaving a lot of gap here for future use)
#define CONSOLE_COLORBUFFER_START		0x0F14F000 // BG/FG color indices for characters (4 bits each, 1 byte per character) (leaving 8K gap here for future use)
// Kernel memory for temporary operations
#define KERNEL_TEMP_MEMORY				0x0F153000 // Temporary kernel memory (16384 bytes)
// Buffers
#define GENERIC_TEMP_BUFFER				0x0F155000 // General purpose buffer for common use, do not assume contents persist (8192 bytes)
#define ISR_STACK_TOP					0x0F157000 // Interrupt service routine stack space (8192 bytes)
#define UART_OUTPUT_TEMP				0x0F159000 // Temporary work space for UART print command (8192 bytes)
#define GDB_DEBUG_DATA					0x0F15B000 // GDB debug state store (8192 bytes)
#define GDB_RESPONSE_BUFFER				0x0F15D000 // GDB packet response buffer (8192 bytes)
#define GDB_PACKET_BUFFER				0x0F15F000 // GDB packet buffer (8192 bytes)
#define KERNEL_GFX_CONTEXT				0x0F160000 // Kernel terminal graphics context (4096 bytes with free space for future use)
#define KERNEL_INPUTBUFFER				0x0F161000 // Input data buffer for keyboard and joystick (4096 bytes)
#define KERNEL_DEVICECONTROL			0x0F162000 // Device control blocks (4096 bytes)
#define TASKMEM_END_STACK_TOP			0x0F163000 // Top of task stack space
// Task stack space
#define TASKMEM_END_STACK_END			0x0FFD0000 // Tasks stack space above this (~16 Mbytes)
// 4 byte gap
#define ROMSHADOW_START					0x0FFE0000 // Gap above this (4Bytes)
// ROM SHADOW
#define ROMSHADOW_END_MEM_END			0x0FFFFFFF // ROM shadow copy above this (128 KBytes, but normally OS ROM fits in upper 64 KByte half)

// There are only 2 cores in this tinysys version
#define MAX_HARTS 2

// Device address base
#define DEVICE_BASE 0x80000000

// Each device has 64 Kbytes of uncached memory space mapped to it
// - Mailbox device only implements 16 Kbytes of memory (lower half of address space)
// - CSR devices only implement 4 Kbytes of memory and is not byte addressable
// - UART and most devices with a FIFO only implement about 16 bytes of physical memory for data and status registers
// - Scratchpad device implements a 16 Kbytes region of memory (lower half of address space)
#define DEVICE_SPAD (DEVICE_BASE+0x00000)
#define DEVICE_LEDS (DEVICE_BASE+0x10000)
#define DEVICE_VPUC (DEVICE_BASE+0x20000)
#define DEVICE_SPIC (DEVICE_BASE+0x30000)
#define DEVICE_DEV0 (DEVICE_BASE+0x40000)
#define DEVICE_ESPC (DEVICE_BASE+0x50000)
#define DEVICE_APUC (DEVICE_BASE+0x60000)
#define DEVICE_MAIL (DEVICE_BASE+0x70000)
#define DEVICE_UART (DEVICE_BASE+0x80000)
#define DEVICE_CSR0 (DEVICE_BASE+0x90000)
#define DEVICE_CSR1 (DEVICE_BASE+0xA0000)
//#define DEVICE_DEV1 (DEVICE_BASE+0xB0000)
//#define DEVICE_DEV2 (DEVICE_BASE+0xC0000)
//#define DEVICE_DEV3 (DEVICE_BASE+0xD0000)
//#define DEVICE_DEV4 (DEVICE_BASE+0xE0000)
//#define DEVICE_DEV5 (DEVICE_BASE+0xF0000)

uint64_t E32ReadTime();
uint64_t E32ReadCycles();
uint64_t E32ReadRetiredInstructions();
void E32SetTimeCompare(const uint64_t future);

uint32_t ClockToMs(uint64_t clk);
uint32_t ClockToUs(uint64_t clk);
void ClockMsToHMS(uint32_t ms, uint32_t *hours, uint32_t *minutes, uint32_t *seconds);

void E32Sleep(uint64_t ticks);

void E32WriteMemMappedCSR(uint32_t _hart, uint32_t _csr, uint32_t _value);
uint32_t E32ReadMemMappedCSR(uint32_t _hart, uint32_t _csr);

// Reset given hardware thread and start executing the supplied task
void E32SetupCPU(uint32_t hartid, void *workerThread);
void E32ResetCPU(uint32_t hartid);
void E32ClearReset(uint32_t hartid);

void E32BeginCriticalSection();
void E32EndCriticalSection();

uint32_t E32GetScratchpad();