/** @file NCO.c
 * See NCO.h for description.
 * @author Adrien RICCIARDI
 */
#include <NCO.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void NCOInitialize(void)
{
	// Enable the peripheral module
	PMD1bits.NCO1MD = 0;

	// Use MFINTOSC/4 (32KHz) to clock the module
	NCO1CLK = 0x04;
}

void NCOConfigure(unsigned short Tick_Frequency)
{
	// Stop the module
	NCO1CONbits.EN = 0;

	// Set the increment value (use a desired frequency of 60Hz as an example)
	// The increment value to put in the xxxINCy registers, its value is (Desired_Frequency * 2^20) / 32000 = 1966.08 with a desired frequency of 60Hz
	// With the rounded value of 1966, the real frequency is 32000 * 1966 / 2^20 = 59.99755859375Hz, which is pretty close
	// Then multiply the result times two because the timer will toggle its output pin on each counter overflow, so the counter needs to overflows twice faster than the desired frequency
	// The MFINTOSC/4 oscillator is really clocked at 32KHz (the generated frequency is roughly measured with an analog oscilloscope to 59.5Hz)
	NCO1INCL = (unsigned char) Tick_Frequency;
	NCO1INCH = Tick_Frequency >> 8;
	NCO1INCU = 0;

	// Make sure the accumulator registers are cleared
	NCO1ACCL = 0;
	NCO1ACCH = 0;
	NCO1ACCU = 0;

	// Enable the module, do not invert the output signal, operate in fixed duty cycle mode
	NCO1CON = 0x80;
}
