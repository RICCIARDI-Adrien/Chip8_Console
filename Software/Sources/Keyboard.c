/** @file Keyboard.c
 * See Keyboard.h for description.
 * @author Adrien RICCIARDI
 */
#include <Keyboard.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Set to 1 when pressed and cleared when the KeyboardIsMenuKeyPressed() function is called. */
static volatile unsigned char Keyboard_Is_Menu_Key_Pressed = 0;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Interrupt handler for the external interrupt 0. */
static void __interrupt(irq(INT0), high_priority) KeyboardInterrupt(void)
{
	// Keep the pressed state to make it available later on
	Keyboard_Is_Menu_Key_Pressed = 1;

	// Clear the interrupt flag
	PIR1bits.INT0IF = 0;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void KeyboardInitialize(void)
{
	// Configure the pins as digital
	ANSELA = 0;
	ANSELBbits.ANSELB5 = 0;

	// Configure the pins as inputs
	TRISA = 0xFF;
	TRISBbits.TRISB5 = 1;

	// Assign the external interrupt 0 feature to the RB5 pin
	INT0PPS = 0x0D; // Port B, pin 5

	// Configure the external interrupt 0 interrupt
	INTCON0bits.INT0EDG = 0; // Trigger the interrupt on signal falling edge
	PIR1bits.INT0IF = 0; // Make sure the interrupt flag is cleared to avoid triggering a false interrupt
	PIE1bits.INT0IE = 1; // Enable the interrupt
}

unsigned char KeyboardReadKeysMask(void)
{
	return ~PORTA; // Key switches are active low
}

unsigned char KeyboardIsMenuKeyPressed(void)
{
	unsigned char Is_Pressed = Keyboard_Is_Menu_Key_Pressed;

	// Reset the pressing state, not bothering here with disabling the key interrupt, this is a low importance feature
	Keyboard_Is_Menu_Key_Pressed = 0;

	return Is_Pressed;
}

unsigned char KeyboardWaitForKeys(TKeyboardKey Keys_Mask)
{
	unsigned char Read_Keys_Mask;

	// Wait for at least one key to be pressed
	do
	{
		Read_Keys_Mask = KeyboardReadKeysMask();
	} while ((Read_Keys_Mask & Keys_Mask) == 0);

	// Wait for all the keys to be released
	while (KeyboardReadKeysMask() != 0);

	// Add a little debounce timer
	__delay_ms(20);

	return Read_Keys_Mask;
}
