/**
 * @file keyboard.c
 * 
 * @brief Keyboard utility functions to convert scancodes to ASCII characters
 */

#include "keyboard.h"
#include "basesystem.h"

/**
 * @brief Keyboard scancode to ASCII table (lowercase)
 * 
 * This table maps keyboard scancodes to ASCII characters when the shift key is not pressed.
 * The table is indexed by the scancode, and the value is the ASCII character.
 * 
 * @see scantoasciitable_uppercase
 */
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

/**
 * @brief Keyboard scancode to ASCII table (uppercase)
 * 
 * This table maps keyboard scancodes to ASCII characters when the shift key is pressed.
 * The table is indexed by the scancode, and the value is the ASCII character.
 * 
 * @see scantoasciitable_lowercase
 */
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
 * @brief Get the keyboard state table
 * 
 * This function returns a pointer to the keyboard state table.
 * The keyboard state table is a 512-byte array of 16-bit values.
 * Each 16-bit value represents the state of a key on the keyboard.
 * Bit 0: Key is down
 * Bit 1: Key is up
 * Index into the table is the scancode of the key.
 * 
 * @return uint16_t* Pointer to the keyboard state table
 */
uint16_t *GetKeyStateTable()
{
	uint16_t* keystates = (uint16_t*)KEYBOARD_KEYSTATE_BASE;
	return keystates;
}

/**
 * @brief Convert a scancode to an ASCII character
 * 
 * This function converts a keyboard scancode to an ASCII character.
 * It selects the correct conversion table based on the state of the shift key.
 * 
 * @return char ASCII character
 */
char KeyScanCodeToASCII(const uint8_t _code, const uint8_t _uppercase)
{
    return _uppercase ? scantoasciitable_uppercase[_code] : scantoasciitable_lowercase[_code];
}

/**
 * @brief Get the generation of the keyboard state table
 * 
 * This function returns the generation of the keyboard state table.
 * The generation is a 32-bit value that is incremented each time the keyboard state table is updated.
 * This allows the caller to determine if the keyboard state table has been updated since the last time it was read.
 * 
 * @return uint32_t Generation of the keyboard state table
 */
uint32_t GetKeyStateGeneration()
{
	uint32_t *generation = (uint32_t*)KEYBOARD_INPUT_GENERATION;
	return *generation;
}

/**
 * @brief Get the state of a key
 * 
 * This function returns the state of a key.
 * The state of a key is a 2-bit value which shows if the key is up or down:
 * Bit 0: Key is down (only if it was in none state before)
 * Bit 1: Key is up (only if it was pressed down before)
 * 
 * If both bits are 0, the key is not pressed.
 * 
 * @return uint16_t State of the key
 */
uint16_t GetKeyState(uint8_t _key)
{
	uint16_t* keystates = (uint16_t*)KEYBOARD_KEYSTATE_BASE;
	return keystates[HKEY_ENTER]&3; // 0:none 1:down 2:up
}
