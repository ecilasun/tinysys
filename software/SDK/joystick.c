#include "joystick.h"
#include "serialinringbuffer.h"

static uint16_t s_buttonstate = 0;
static float s_axisstate[6] = {0.0f};

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
	s_buttonstate = (scandata[1] << 8) | scandata[0];
}

/**
 * @brief Process the joystick axis state and convert to ASCII for the key buffer
 * 
 * @param scandata 
 */
void ProcessAxisState(uint8_t *scandata)
{
	s_axisstate[0] = ((scandata[1] << 8 | scandata[0]) - 32768) / 32767.0f;
	s_axisstate[1] = ((scandata[3] << 8 | scandata[2]) - 32768) / 32767.0f;
	s_axisstate[2] = ((scandata[5] << 8 | scandata[4]) - 32768) / 32767.0f;
	s_axisstate[3] = ((scandata[7] << 8 | scandata[6]) - 32768) / 32767.0f;
	s_axisstate[4] = scandata[8] / 255.0f;
	s_axisstate[5] = scandata[9] / 255.0f;
}

/**
 * @brief Read joystick state
 * 
 * @param _axisData - Axis data for thumbsticks and triggers
 * @param _buttonData - Button data including D-Pad and direction buttons
 */
void JoystickReadState(float *_axisData, uint16_t* _buttonData)
{
	_axisData[0] = s_axisstate[0];
	_axisData[1] = s_axisstate[1];
	_axisData[2] = s_axisstate[2];
	_axisData[3] = s_axisstate[3];
	_axisData[4] = s_axisstate[4];
	_axisData[5] = s_axisstate[5];
	*_buttonData = s_buttonstate;
}
