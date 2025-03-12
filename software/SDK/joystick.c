#include "joystick.h"
#include "serialinringbuffer.h"

/**
 * @brief Read joystick button packet from the serial input buffer
 * 
 * @param scandata - Joystick button packet
 */
void ReadButtonState(uint8_t *scandata)
{
	uint32_t scancursor = 0;
	uint8_t drain;
	while (scancursor != JOYSTICK_BUTTON_PACKET_SIZE)
	{
		if (SerialInRingBufferRead(&drain, 1))
		{
			scandata[scancursor] = drain;
			scancursor++;
		}
	}
}

/**
 * @brief Read joystick axis packet from the serial input buffer
 * 
 * @param scandata - Joystick axis packet
 */
void ReadAxisState(uint8_t *scandata)
{
	uint32_t scancursor = 0;
	uint8_t drain;
	while (scancursor != JOYSTICK_AXIS6_PACKET_SIZE)
	{
		if (SerialInRingBufferRead(&drain, 1))
		{
			scandata[scancursor] = drain;
			scancursor++;
		}
	}
}

/**
 * @brief Process the joystick button state and convert to ASCII for the key buffer
 * 
 * @param scandata 
 */
void ProcessButtonState(uint8_t *scandata)
{
	volatile struct SJoystickState* joystick = (volatile struct SJoystickState*)(KERNEL_INPUTBUFFER+512);
	joystick->buttons = (scandata[1] << 8) | scandata[0];
}

/**
 * @brief Process the joystick axis state and convert to ASCII for the key buffer
 * 
 * @param scandata 
 */
void ProcessAxisState(uint8_t *scandata)
{
	volatile struct SJoystickState* joystick = (volatile struct SJoystickState*)(KERNEL_INPUTBUFFER+512);
	joystick->axis[0] = ((scandata[1] << 8 | scandata[0]) - 32768) / 32767.0f;
	joystick->axis[1] = ((scandata[3] << 8 | scandata[2]) - 32768) / 32767.0f;
	joystick->axis[2] = ((scandata[5] << 8 | scandata[4]) - 32768) / 32767.0f;
	joystick->axis[3] = ((scandata[7] << 8 | scandata[6]) - 32768) / 32767.0f;
	joystick->axis[4] = scandata[8] / 255.0f;
	joystick->axis[5] = scandata[9] / 255.0f;
}

/**
 * @brief Process the joystick state
 * 
 */
void UpdateJoystickState()
{
	volatile struct SJoystickState* joystick = (volatile struct SJoystickState*)(KERNEL_INPUTBUFFER+512);
	joystick->count = joystick->count + 1;
}

/**
 * @brief 	Get the current joystick state
 * 
 * @return Joystick state struct pointer
 */
volatile struct SJoystickState* JoystickGetState()
{
	return (volatile struct SJoystickState*)(KERNEL_INPUTBUFFER+512);
}
