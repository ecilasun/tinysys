#include "keyboard.h"
#include "serialinringbuffer.h"
#include "keyringbuffer.h"

static int s_lowercase = 1;

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

	// TODO: Track modifier state
	s_lowercase = (modifiers & 0x01) ? 0 : 1;

	// Check for CTRL+C down (either of left or right control keys)
	if (scancode == 0x06 && (modifiers & 0x00C0) && state == 1)
	{
		// Write CTRL+C to key buffer
		uint8_t ascii = 0x03;
		KeyRingBufferWrite(&ascii, 1);
	}
	else
	{
		// Not CTRL+C, push to key buffer as ASCII character
		if (state == 1 && scancode<256)
		{
			uint8_t ascii = KeyboardScanCodeToASCII(scancode, s_lowercase);
			KeyRingBufferWrite(&ascii, 1);
		}
	}
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
 * @param lowercase Whether the key is in lowercase or uppercase
 * @return The ASCII character corresponding to the scan code
 */
uint8_t KeyboardScanCodeToASCII(uint8_t scanCode, uint8_t lowercase)
{
	uint8_t ascii;
	if (s_lowercase)
		ascii = scantoasciitable_lowercase[scanCode];
	else
		ascii = scantoasciitable_uppercase[scanCode];
	return ascii;
}