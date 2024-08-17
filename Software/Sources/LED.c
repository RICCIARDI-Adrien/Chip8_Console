/** @file LED.c
 * See LED.h for description.
 * @author Adrien RICCIARDI
 */
#include <LED.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void LEDInitialize(void)
{
	ANSELBbits.ANSELB0 = 0;
	LED_SET_ENABLED(0);
	TRISBbits.TRISB0 = 0;
}
