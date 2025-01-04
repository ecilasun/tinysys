#include <stdint.h>

#define KEYBOARD_SCANCODE_INDEX 0
#define KEYBOARD_STATE_INDEX 1
#define KEYBOARD_MODIFIERS_LOWER_INDEX 2
#define KEYBOARD_MODIFIERS_UPPER_INDEX 3

#define KEYBOARD_PACKET_SIZE 4

void ReadKeyState(uint8_t *scandata);
void ProcessKeyState(uint8_t *scandata);

uint8_t KeyboardScanCodeToASCII(uint8_t scanCode, uint8_t lowercase);