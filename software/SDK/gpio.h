#pragma once

#include <inttypes.h>

extern volatile uint32_t *GPIO_DATA;			// Data I/O
extern volatile uint32_t *GPIO_INPUTENABLE;		// Bit mask for reads
extern volatile uint32_t *GPIO_OUTPUTENABLE;	// Bit mask for writes
extern volatile uint32_t *GPIO_FIFOHASDATA;		// FIFO status

// ESP UI pin names to bit index (0 is LSB)
// NOTE: The IO indices will not match bit indices
// as these are remapped by hardware on FPGA pin side

#define ESP32_IO11	0
#define ESP32_IO10	1
#define ESP32_IO1	2
#define ESP32_IO0	3
#define ESP32_IO7	4
#define ESP32_IO6	5
#define ESP32_IO5	6
#define ESP32_IO4	7
#define ESP32_IO19	8
#define ESP32_IO20	9
#define ESP32_IO18	10
#define ESP32_IO22	11
#define ESP32_IO21	12
#define ESP32_IO15	13
#define ESP32_IO23	14
#define ESP32_TXD0	15
#define ESP32_RXD0	16
#define ESP32_IO2	17
#define ESP32_IO3	18
