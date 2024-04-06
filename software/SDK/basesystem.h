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

// Physical address map for no-MMU raw mode at boot time
#define APPMEM_START					0x00000000 // Top of RAM
// Keyboard ring buffer (1Kbytes)
#define KEY_RINGBUFFER_BASE				0x00000200
#define KEY_RINGBUFFER_STATE			0x00000600
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
#define SERIN_RINGBUFFER_BASE			0x02050000 // Serial input (16384 bytes)
#define SEROUT_RINGBUFFER_BASE			0x02054000 // Serial output (16384 bytes)
#define GPIO_RINGBUFFER_BASE			0x02058000 // GPIO pin input (16384 bytes)
#define SERIN_RINGBUFFER_STATE			0x0205C010 // Serial input state
#define SEROUT_RINGBUFFER_STATE			0x0205C020 // Serial output state
#define GPIO_RINGBUFFER_STATE			0x0205C030 // GPIO input state
// ETC
#define KERNEL_TASK_CONTEXT				0x0205C040 // Task context memory (81856 bytes)
// Executable
#define HEAP_START_APPMEM_END			0x02070000 // Executable space above this
// Heap
#define HEAP_END_CONSOLEMEM_START		0x0FF00000 // Heap space above this
// Kernel console text+attrib/scratch
#define CONSOLEMEM_END_KERNEL_VRAM_TOP	0x0FF10000 // Console text+attrib+scratch memory above this (64KBytes)
// Kernel VRAM/scratch
#define VRAM_END_TASKMEM_START			0x0FF30000 // Kernel VRAM above this (128KBytes)
// Task stack space
#define TASKMEM_END_STACK_END			0x0FFD0000 // Tasks stack space above this
//  Kernel stack
#define STACK_BASE						0x0FFDFFFC // Kernel stack above this
// 4 byte gap
#define ROMSHADOW_START					0x0FFE0000 // Gap above this (4Bytes)
// ROM SHADOW
#define ROMSHADOW_END_MEM_END			0x0FFFFFFF // ROM shadow copy above this (128KBytes, ROM OS fits in upper 64KBytes)

// Device address base
#define DEVICE_BASE 0x80000000

// Each device has 4 Kbytes of continous, uncached memory region mapped to it
#define DEVICE_GPIO (DEVICE_BASE+0x0000)
#define DEVICE_LEDS (DEVICE_BASE+0x1000)
#define DEVICE_VPUC (DEVICE_BASE+0x2000)
#define DEVICE_SPIC (DEVICE_BASE+0x3000)
#define DEVICE_XADC (DEVICE_BASE+0x4000)
#define DEVICE_DMAC (DEVICE_BASE+0x5000)
#define DEVICE_USBC (DEVICE_BASE+0x6000)
#define DEVICE_APUC (DEVICE_BASE+0x7000)
#define DEVICE_USBA (DEVICE_BASE+0x8000)
#define DEVICE_CSR0 (DEVICE_BASE+0x9000)
#define DEVICE_UART (DEVICE_BASE+0xA000)
// NOTE: Add more devices after this point

uint64_t E32ReadTime();
uint64_t E32ReadCycles();
uint64_t E32ReadRetiredInstructions();
void E32SetTimeCompare(const uint64_t future);

uint32_t ClockToMs(uint64_t clk);
uint32_t ClockToUs(uint64_t clk);
void ClockMsToHMS(uint32_t ms, uint32_t *hours, uint32_t *minutes, uint32_t *seconds);

void E32Sleep(uint64_t ticks);
