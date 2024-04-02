/** @file Keyboard.c
 * See Keyboard.h for description.
 * @author Adrien RICCIARDI
 */
#include <Keyboard.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void KeyboardInitialize(void)
{
    // Configure the pins as digital
    ANSELA = 0;
    // Configure the pins as inputs
    TRISA = 0xFF;
}

unsigned char KeyboardReadKeysMask(void)
{
    return ~PORTA; // Key switches are active low
}
