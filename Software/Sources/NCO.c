/** @file NCO.c
 * See NCO.h for description.
 * @author Adrien RICCIARDI
 */
#include <NCO.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** The increment value to put in the xxxINCy registers, its value is (Desired_Frequency * 2^20) / 32000 = 1966.08 with a desired frequency of 60Hz.
 * With the rounded value of 1966, the real frequency is 32000 * 1966 / 2^20 = 59.99755859375Hz, which is pretty close.
 * The MFINTOSC/4 oscillator is really clocked at 32KHz (the generated frequency is roughly measured with an analog oscilloscope to 59.5Hz).
 */
#define NCO_INCREMENT_VALUE (1966 * 2UL) // Multiply times two because the timer will toggle its output pin on each counter overflow, so the counter needs to overflows twice faster than the desired frequency

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void NCOInitialize(void)
{
	NCO1CLK = 0x04; // Use MFINTOSC/4 (32KHz) to clock the module

	// Set the increment value
	NCO1INCL = (unsigned char) NCO_INCREMENT_VALUE;
	NCO1INCH = NCO_INCREMENT_VALUE >> 8;
	NCO1INCU = NCO_INCREMENT_VALUE >> 16;

	// Make sure the accumulator registers are cleared
	NCO1ACCL = 0;
	NCO1ACCH = 0;
	NCO1ACCU = 0;

	NCO1CON = 0x80; // Enable the module, do not invert the output signal, operate in fixed duty cycle mode
}
