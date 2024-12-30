/**
 * @file leds.c
 * 
 * @brief LED control functions
 * 
 * This file contains functions to control the debug LEDs on the FPGA.
 */

#include "basesystem.h"
#include "leds.h"

volatile uint32_t *LEDSTATE = (volatile uint32_t* ) DEVICE_LEDS;

/**
 * @brief Get the current state of the debug LEDs
 * 
 * The debug LEDs are mapped to the lower 4 bits of the LED state register.
 * 
 * @return Current state of the debug LEDs
 */
uint32_t LEDGetState()
{
	return *LEDSTATE;
}

/**
 * @brief Set the state of the debug LEDs
 * 
 * The debug LEDs are mapped to the lower 4 bits of the LED state register.
 *
 * @param state New state of the debug LEDs
 */
void LEDSetState(const uint32_t state)
{
	*LEDSTATE = state;
}
