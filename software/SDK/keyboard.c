#include "keyboard.h"
#include "serialinringbuffer.h"
#include "keyringbuffer.h"

static int s_control = 0;
static int s_alt = 0;

// Scan code to lower case ASCII conversion table
char scantoasciitable_lowercase[] = {
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	  0,    0,    0,    0,  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l', // 0
	'm',  'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '1',  '2', // 1
	'3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',   10,   27,    8,    9,  ' ',  '-',  '=',  '[', // 2
	']', '\\',  '#',  ';', '\'',  '^',  ',',  '.',  '/',    0,    0,    0,    0,    0,    0,    0, // 3
	  0,    0,    0,    0,    0,    0,    0,    0,    0,  '.',    0,    0,    0,    0,    0,    0, // 4
	  0,    0,    0,    0,  '/',  '*',  '-',  '+',   13,  '1',  '2',  '3',  '4',  '5',  '6',  '7', // 5
	'8',  '9',  '0',  '.', '\\',    0,    0,  '=',    0,    0,    0,    0,    0,    0,    0,    0, // 6
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 7
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 8
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 9
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // A
	  0,    0,    0,    0,    0,    0,  '(',  ')',  '{',  '}',    8,    9,    0,    0,    0,    0, // B
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // C
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // D
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // E
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0  // F
};


// Scan code to upper case ASCII conversion table
char scantoasciitable_uppercase[] = {
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
	  0,    0,    0,    0,  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L', // 0
	'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '!',  '@', // 1
	'#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',   10,   27,    8,    9,  ' ',  '_',  '+',  '{', // 2
	'}',  '|',  '~',  ':',  '"',  '~',  '<',  '>',  '?',    0,    0,    0,    0,    0,    0,    0, // 3
	  0,    0,    0,    0,    0,    0,    0,    0,    0,  '.',    0,    0,    0,    0,    0,    0, // 4
	  0,    0,    0,    0,  '/',  '*',  '-',  '+',   13,  '1',  '2',  '3',  '4',  '5',  '6',  '7', // 5
	'8',  '9',  '0',  '.', '\\',    0,    0,  '=',    0,    0,    0,    0,    0,    0,    0,    0, // 6
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 7
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 8
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // 9
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // A
	  0,    0,    0,    0,    0,    0,  '(',  ')',  '{',  '}',    8,    9,    0,    0,    0,    0, // B
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // C
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // D
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // E
	  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0  // F
};

/**
 * Process a 3-byte key scan packet
 * 
 * @param scandata Pointer to a 3-byte buffer containing the scan packet
 */
void ProcessKeyState(uint8_t *scandata)
{
	uint8_t scancode = scandata[KEYBOARD_SCANCODE_INDEX];
	uint8_t state = scandata[KEYBOARD_STATE_INDEX];

	uint8_t modifiers_lower = scandata[KEYBOARD_MODIFIERS_LOWER_INDEX];
	uint8_t modifiers_upper = scandata[KEYBOARD_MODIFIERS_UPPER_INDEX];
	uint32_t modifiers = (modifiers_upper << 8) | modifiers_lower;

	// Ignore some unused keys
	if (scancode == 0xE3 || scancode == 0x65 || scancode == 0x39 || scancode == 0xE1 || scancode == 0xE5) // Windows, Menu key, Caps Lock, Shift keys
	{
		// Ignore
		return;
	}

	int isUppercase = (modifiers & 0x2003) ? 1 : 0; // Check if Caps Lock or either shift is down

	// Left or Rigth Control key
	if (scancode == 0xE0 || scancode == 0xE4)
	{
		// Toggle control state
		s_control = state ? 1 : 0;
		return;
	}

	// Left or Right Alt key
	if (scancode == 0xE2 || scancode == 0xE6)
	{
		// Toggle alt state
		s_alt = state ? 1 : 0;
		return;
	}

	// NOTE: Remote side handles trapping CTRL+C and ~ keys so we don't neeed to be concerned with them here
	if (state == 1 && scancode<256)
	{
		if (scancode == 0x52) // Up arrow
		{
			uint8_t sequence[2] = { 224, 72 };
			KeyRingBufferWrite(&sequence, 2);
			return;
		}
		if (scancode == 0x51) // Down arrow
		{
			uint8_t sequence[2] = { 224, 80 };
			KeyRingBufferWrite(&sequence, 2);
			return;
		}
		if (scancode == 0x50) // Left arrow
		{
			uint8_t sequence[2] = { 224, 75 };
			KeyRingBufferWrite(&sequence, 2);
			return;
		}
		if (scancode == 0x4F) // Right arrow
		{
			uint8_t sequence[2] = { 224, 77 };
			KeyRingBufferWrite(&sequence, 2);
			return;
		}
		if (scancode == 0x48) // Pause, consider same as CTRL+C
		{
			uint8_t sequence[2] = { 3 };
			KeyRingBufferWrite(&sequence, 1);
			return;
		}
		uint8_t ascii = KeyboardScanCodeToASCII(scancode, isUppercase);
		KeyRingBufferWrite(&ascii, 1);
	}
}

/**
 * Save the keyboard state to a buffer
 * 
 * @param scandata Pointer to a KEYBOARD_PACKET_SIZE byte buffer from which the input comes
 */
void UpdateKeyboardState(uint8_t *scandata)
{
	volatile struct SKeyboardState* keys = (volatile struct SKeyboardState*)KERNEL_INPUTBUFFER;

	keys->scancode = scandata[KEYBOARD_SCANCODE_INDEX];
	keys->state = scandata[KEYBOARD_STATE_INDEX];
	uint8_t modifiers_lower = scandata[KEYBOARD_MODIFIERS_LOWER_INDEX];
	uint8_t modifiers_upper = scandata[KEYBOARD_MODIFIERS_UPPER_INDEX];
	keys->modifiers = (modifiers_upper << 8) | modifiers_lower;
	int isUppercase = (keys->modifiers & 0x2003) ? 1 : 0;
	keys->ascii = KeyboardScanCodeToASCII(keys->scancode, isUppercase);

	keys->count = keys->count + 1;
}

/**
 * Get the current keyboard state
 * 
 * @return Pointer to the current keyboard state
 */
volatile struct SKeyboardState* KeyboardGetState()
{
	return (struct SKeyboardState*)KERNEL_INPUTBUFFER;
}

/**
 * Read a 3-byte key scan packet from the serial input buffer
 * 
 * @param scandata Pointer to a KEYBOARD_PACKET_SIZE byte buffer to store the scan packet
 */
void ReadKeyState(uint8_t *scandata)
{
	uint32_t scancursor = 0;
	uint8_t drain;
	while (scancursor != KEYBOARD_PACKET_SIZE)
	{
		if (SerialInRingBufferRead(&drain, 1))
		{
			scandata[scancursor] = drain;
			scancursor++;
		}
	}
}

/**
 * Convert a key scan code to an ASCII character
 * 
 * @param scanCode The key scan code
 * @param uppercase Whether the key is in lowercase or uppercase
 * @return The ASCII character corresponding to the scan code
 */
uint8_t KeyboardScanCodeToASCII(uint8_t scanCode, uint8_t uppercase)
{
	uint8_t ascii;
	if (uppercase)
		ascii = scantoasciitable_uppercase[scanCode];
	else
		ascii = scantoasciitable_lowercase[scanCode];
	return ascii;
}