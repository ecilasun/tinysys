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
	uint8_t key = scandata[0];
	uint8_t state = scandata[1];
	uint8_t mod = scandata[2];

	// TODO: Track modifier state
	s_lowercase = (mod & 0x01) ? 0 : 1;

	if (state == 1 && key<256) // Key down, only care about printable ASCII
	{
		uint8_t ascii;
		if (s_lowercase)
			ascii = scantoasciitable_lowercase[key];
		else
			ascii = scantoasciitable_uppercase[key];
		KeyRingBufferWrite(&ascii, 1);
	}
}

/**
 * Read a 3-byte key scan packet from the serial input buffer
 * 
 * @param scandata Pointer to a 3-byte buffer to store the scan packet
 */
void ReadKeyState(uint8_t *scandata)
{
	uint32_t scancursor = 0;
	uint8_t drain;
	while (scancursor != 3)
	{
		if (SerialInRingBufferRead(&drain, 1))
		{
			scandata[scancursor] = drain;
			scancursor++;
		}
	}
}
