/** @file LED.h
 * Control the board LED.
 * @author Adrien RICCIARDI
 */
#ifndef H_LED_H
#define H_LED_H

#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
/** Turn the LED on or off.
 * @param Is_Enabled Set to 1 to light the LED or set to 0 to turn the LED off.
 */
#define LED_SET_ENABLED(Is_Enabled) LATBbits.LATB0 = Is_Enabled

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the LED pin and turn the LED off. */
void LEDInitialize(void);

#endif
