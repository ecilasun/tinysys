#include <stdint.h>

#define KEYBOARD_SCANCODE_INDEX 0
#define KEYBOARD_STATE_INDEX 1
#define KEYBOARD_MODIFIERS_LOWER_INDEX 2
#define KEYBOARD_MODIFIERS_UPPER_INDEX 3

#define KEYBOARD_PACKET_SIZE 4

struct SKeyboardState
{
	uint32_t count;
	uint32_t scancode;
	uint32_t state;
	uint32_t modifiers;
	uint32_t ascii;
};

void ReadKeyState(uint8_t *scandata);
void ProcessKeyState(uint8_t *scandata);
void UpdateKeyboardState(uint8_t *scandata);

volatile struct SKeyboardState* KeyboardGetState();
uint8_t KeyboardScanCodeToASCII(uint8_t scanCode, uint8_t lowercase);