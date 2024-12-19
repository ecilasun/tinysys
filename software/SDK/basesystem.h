#pragma once

#include <inttypes.h>

// tinysys runs wall clock at 10MHz
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

#define CSR_CPURESET					0xFEE
#define CSR_WATERMARK					0xFF0
#define CSR_PROGRAMCOUNTER				0xFFC

// Physical address map for no-MMU raw mode at boot time
#define APPMEM_START					0x00000000 // Top of RAM
// Unused buffer and state (1Kbytes)
#define UNUSED_BUFFER_BASE3				0x00000200
#define UNUSED_STATE3					0x00000600
// Keyboard input map (512 bytes)
#define KEYBOARD_KEYSTATE_BASE			0x00000800
#define KEYBOARD_KEYSTATE_END			0x00000A00
// Keyboard state tracking data (512 bytes)
#define KEYBOARD_KEYTRACK_BASE			0x00000A20
#define KEYBOARD_KEYTRACK_END			0x00000C20
#define KEYBOARD_INPUT_GENERATION		0x00000C24
// USB host status
#define USB_HOST_STATE					0x00000D20
// Mouse x/y and button data - 12 bytes
#define MOUSE_POS_AND_BUTTONS			0x00000D24
// Joystick x/y and buttons - 16 bytes
#define JOYSTICK_POS_AND_BUTTONS		0x00000D30
// Temp file upload location
#define TEMP_FILE_UPLOAD_START			0x00100000
// Console buffer
#define CONSOLE_FRAMEBUFFER_START		0x02000000 // Console framebuffer == 0x4B000 bytes max at 640*480 resolution
#define CONSOLE_CHARACTERBUFFER_START	0x0204B000 // Character store == 80*60 bytes max at 640*480 resolution
#define CONSOLE_COLORBUFFER_START		0x0204C2C0 // BG/FG color indices for characters (4 bits each, 1 byte per character)
// Temp memory
#define KERNEL_TEMP_MEMORY				0x0204D580 // Temporary kernel memory (10880 bytes)
// Serial buffers (first words are counters)
#define ISR_STACK_TOP					0x02050000 // Interrupt service routine stack space (16384 bytes)
#define UNUSED_BUFFER_BASE0				0x02054000 // Reserved for future (16384 bytes)
#define GDB_RESPONSE_BUFFER				0x02058000 // GDB packet response buffer (16384 bytes)
#define GDB_PACKET_BUFFER				0x0205C000 // GDB packet buffer (16384 bytes)
#define UNUSED_STATE4					0x02060000 // Reserved for future (16 bytes)
#define UNUSED_STATE0					0x02060010 // Reserved for future (16 bytes)
#define UNUSED_STATE1					0x02060020 // Reserved for future (16 bytes)
#define UNUSED_STATE2					0x02060030 // Reserved for future (16 bytes)
#define KERNEL_GFX_CONTEXT				0x02060040 // Kernel terminal graphics context
// Executable
#define HEAP_START_APPMEM_END			0x02080000 // Executable space above this
// Heap
#define HEAP_END_CONSOLEMEM_START		0x0FF00000 // Heap space above this
// Task stack space
#define TASKMEM_END_STACK_END			0x0FFD0000 // Tasks stack space above this
//  Kernel stacks for all cores, 256 bytes each
#define STACK_BASE_HART1				0x0FFDFEF0 // Kernel stack above these (65264 bytes)
#define STACK_BASE_HART0				0x0FFDFFF0
// 4 byte gap
#define ROMSHADOW_START					0x0FFE0000 // Gap above this (4Bytes)
// ROM SHADOW
#define ROMSHADOW_END_MEM_END			0x0FFFFFFF // ROM shadow copy above this (128 KBytes, but normally OS ROM fits in upper 64 KByte half)

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
#define DEVICE_DMAC (DEVICE_BASE+0x40000)
#define DEVICE_USBA (DEVICE_BASE+0x50000)
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

void E32BeginCriticalSection();
void E32EndCriticalSection();

uint32_t E32GetScratchpad();