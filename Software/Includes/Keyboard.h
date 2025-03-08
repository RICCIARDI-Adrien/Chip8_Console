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
typedef enum : unsigned short
{
    KEYBOARD_KEY_MENU = 0x0100,
    KEYBOARD_KEY_UP = 0x0080,
    KEYBOARD_KEY_DOWN = 0x0040,
    KEYBOARD_KEY_LEFT = 0x0020,
    KEYBOARD_KEY_RIGHT = 0x0010,
    KEYBOARD_KEY_A = 0x0008,
    KEYBOARD_KEY_B = 0x0004,
    KEYBOARD_KEY_C = 0x0002,
    KEYBOARD_KEY_D = 0x0001
} TKeyboardKey;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the pins connected to the switches. */
void KeyboardInitialize(void);

/** Read the state of all "game" keys (excluding the menu key) without blocking.
 * @return A bit mask telling which keys are pressed at the moment the keys are sampled. A set bit tells that a key is pressed.
 */
unsigned char KeyboardReadKeysMask(void);

/** Tell whether the menu key has been pressed.
 * @note The pressed status is cleared every time this function is called.
 * @return 1 if the menu key has been pressed since the last time this function was called, 0 otherwise.
 */
unsigned char KeyboardIsMenuKeyPressed(void);

/** Block until one of the keys of the specified mask is pressed.
 * @param Keys_Mask A mask of the keys to wait for.
 * @return A mask of the pressed keys.
 */
TKeyboardKey KeyboardWaitForKeys(TKeyboardKey Keys_Mask);

#endif
