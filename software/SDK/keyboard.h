#include <stdint.h>

void ReadKeyState(uint8_t *scandata);
void ProcessKeyState(uint8_t *scandata);

uint8_t KeyboardScanCodeToASCII(uint8_t scanCode, uint8_t lowercase);