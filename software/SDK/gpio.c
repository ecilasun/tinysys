/**
 * @file gpio.c
 * 
 * @brief GPIO memory addresses
 *
 * This file contains the addresses for the GPIO module on the FPGA.
 *
 * The GPIO module is used to control the general purpose input/output pins on the FPGA.
 *
 */

#include "basesystem.h"
#include "gpio.h"
#include <stdlib.h>

/**
 * @brief GPIO data register (32-bit)
 * 0x0: Data I/O
 */
volatile uint32_t *GPIO_DATA = (volatile uint32_t* ) (DEVICE_GPIO+0x0);

/**
 * @brief GPIO read control register (32-bit)
 * 0x4: Bit mask for reads
 */
volatile uint32_t *GPIO_INPUTENABLE = (volatile uint32_t* ) (DEVICE_GPIO+0x4);

/**
 * @brief GPIO write control register (32-bit)
 * 0x8: Bit mask for writes
 */
volatile uint32_t *GPIO_OUTPUTENABLE = (volatile uint32_t* ) (DEVICE_GPIO+0x8);

/**
 * @brief GPIO status register (32-bit)
 * 0xC: FIFO status
 */
volatile uint32_t *GPIO_FIFOHASDATA = (volatile uint32_t* ) (DEVICE_GPIO+0xC);
