/** @file Keyboard.h
 * Manage all console key switches.
 * @author Adrien RICCIARDI
 */
#ifndef H_KEYBOARD_H
#define H_KEYBOARD_H

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** All available keys. */
typedef enum
{
    KEYBOARD_KEY_UP = 0x80,
    KEYBOARD_KEY_DOWN = 0x40,
    KEYBOARD_KEY_LEFT = 0x20,
    KEYBOARD_KEY_RIGHT = 0x10,
    KEYBOARD_KEY_A = 0x08,
    KEYBOARD_KEY_B = 0x04,
    KEYBOARD_KEY_C = 0x02,
    KEYBOARD_KEY_D = 0x01
} TKeyboardKey;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the pins connected to the switches. */
void KeyboardInitialize(void);

/** Read the state of all keys without blocking.
 * @return A bit mask telling which keys are pressed at the moment the keys are sampled. A set bit tells that a key is pressed.
 */
unsigned char KeyboardReadKeysMask(void);

#endif