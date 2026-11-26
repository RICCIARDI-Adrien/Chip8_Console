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

	// Use MFINTOSC/4 (31.25KHz) to clock the module
	NCO1CLK = 0x04;

	// Use a CLC as a D flip-flop to divide by two the NCO frequency (useful to have a 30Hz rendering tick when the NCO is configured at 60Hz to clock the Chip-8 interpreter timers)
	PMD6bits.CLC1MD = 0; // Clock the CLC1 module

	// Select the input signals
	CLC1SEL0 = 0x1E; // Consider the first input as the flip-flop clock signal, connect it to the NCO output (NCO1_out)
	CLC1SEL1 = 0x24; // Consider the second input as the flip-flop D pin, connect it to the flip-flop output (CLC1_out)

	// Configure the input gates
	CLC1GLS0 = 0x02; // Connect CLCIN0 (the NCO output) to the flip-flop clock signal (gate 0)
	CLC1GLS1 = 0x08; // Connect CLCIN1 (the flip-flop Q output) to the flip-flop D input (gate 1)
	CLC1GLS2 = 0; // In combination with the gate polarity, apply a logic 0 to the flip-flop reset input to disable the signal
	CLC1GLS3 = 0; // In combination with the gate polarity, apply a logic 0 to the flip-flop set input to disable the signal

	// Configure each input gate polarity
	CLC1POL = 0x02; // Do not invert the Q output of the flip-flop, apply a logic 0 to the flip-flop set and reset inputs, invert the flip-flop Q output before applying it to the flip-flop D input
	//CLC1POL = 0x80;

	// Configure the logic cell
	CLC1CON = 0x14; // Do not enable the cell yet, enable the rising edge interrupt flag, select a 1-input D flip-flop with S and R
	CLC1CONbits.EN = 1; // Enable the cell module now that everything is configured
}

void NCOConfigure(unsigned short Tick_Frequency)
{
	// Stop the module
	NCO1CONbits.EN = 0;

	// Set the increment value (use a desired frequency of 60Hz as an example)
	// The increment value to put in the xxxINCy registers, its value is (Desired_Frequency * 2^20) / 31250 * 2 = 2013.27 with a desired frequency of 60Hz
	// With the rounded value of 2013, the real frequency is 31250 * 2013 * 2 / 2^20 = 59.992Hz, which is pretty close
	// The result needs to be multiplied times two because the timer will toggle its output pin on each counter overflow, so the counter needs to overflows twice faster than the desired frequency
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
