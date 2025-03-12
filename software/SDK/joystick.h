#include <stdint.h>

#define JOYSTICK_AXIS6_PACKET_SIZE 10
#define JOYSTICK_BUTTON_PACKET_SIZE 2

struct SJoystickState
{
	uint32_t count;
	float axis[6];
	uint16_t buttons;
};

void ReadAxisState(uint8_t *scandata);
void ProcessAxisState(uint8_t *scandata);
void ReadButtonState(uint8_t *scandata);
void ProcessButtonState(uint8_t *scandata);
void UpdateJoystickState();

volatile struct SJoystickState* JoystickGetState();