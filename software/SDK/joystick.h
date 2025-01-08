#include <stdint.h>

#define JOYSTICK_AXIS6_PACKET_SIZE 10
#define JOYSTICK_BUTTON_PACKET_SIZE 2

void ReadAxisState(uint8_t *scandata);
void ProcessAxisState(uint8_t *scandata);
void ReadButtonState(uint8_t *scandata);
void ProcessButtonState(uint8_t *scandata);

void JoystickReadState(float *_axisData, uint16_t* _buttonData);