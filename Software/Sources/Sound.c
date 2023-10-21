/** @file Sound.c
 * See Sound.h for description.
 * @author Adrien RICCIARDI
 */
#include <Sound.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** The PWM period is a 8-bit value computed as follow : TxPR = ((1 / PWM_Frequency) / (4 * (1 / Fosc) * Prescaler)) - 1.
 * The desired PWM frequency is here 2KHz, with Fosc being 64MHz. By using a 1:32 prescaler, the computed value is an integer and fits into 8 bits.
 */
#define SOUND_PWM_PERIOD 249

/** The pulse width is a 10-bit value computed as follow : PWMxDC = Pulse_Width / ((1 / Fosc) * Prescaler).
 * The pulse width is here 1 / 2000 / 2 to get a 50% duty cycle.
 */
#define SOUND_PWM_PULSE_WIDTH 500

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void SoundInitialize(void)
{
	// Configure the PWM module
	CCPTMRS1 = (CCPTMRS1 & 0xFC) | 0x01; // Select TMR2 clock for PWM5
	PWM5DCL = (SOUND_PWM_PULSE_WIDTH & 0x03) << 6;
	PWM5DCH = SOUND_PWM_PULSE_WIDTH >> 2;
	PWM5CON = 0; // Do not enable the PWM for now, do not invert the output signal

	// Configure the timer that clocks the PWM
	T2CLK = 0x01; // The datasheet tells that the timer must be clocked by Fosc/4 for proper PWM operations
	T2PR = SOUND_PWM_PERIOD;
	T2HLT = 0xA0; // Prescaler output is synchronized with Fosc/4, also synchronize the ON bit with the timer clock input, select the free running period mode with period pulse and software gate
	T2CON = 0x50; // Do not enable the timer yet, configure a 1:32 prescaler and no postscaler as it is not used by the PWM module

	// Configure the pin that drives the buzzer
	RC2PPS = 0x0D; // Select the RC2 pin for the PWM output
	ANSELCbits.ANSELC2 = 0; // Configure the pin as digital
	TRISCbits.TRISC2 = 0; // Set the pin as output

	// Enable the waveform generation
	PWM5CONbits.EN = 1; // Start the PWM
	T2CONbits.ON = 1; // Start the PWM clocking timer
}
