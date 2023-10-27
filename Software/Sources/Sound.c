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
	PWM5CON = 0; // Do not enable the PWM for now, do not invert the output signal TODO invert the output signal later as the buzzer will be driver by a transistor that will invert the polarity

	// Configure the timer that clocks the PWM
	T2CLK = 0x01; // The datasheet tells that the timer must be clocked by Fosc/4 for proper PWM operations
	T2PR = SOUND_PWM_PERIOD;
	T2HLT = 0xA0; // Prescaler output is synchronized with Fosc/4, also synchronize the ON bit with the timer clock input, select the free running period mode with period pulse and software gate
	T2CON = 0x50; // Do not enable the timer yet, configure a 1:32 prescaler and no postscaler as it is not used by the PWM module

	// Configure the timer that is used as the CHIP-8 sound timer
	T4CLK = 0x09; // Select the NCO as clock source (the NCO is outputting a clean 60Hz clock)
	T4HLT = 0xA8; // Prescaler output is synchronized with Fosc/4, also synchronize the ON bit with the timer clock input, select the one-shot mode with one-shot operation and software start
	T4CON = 0; // Do not enable the timer yet, do not enable any prescaler or postscaler (the timer is already clocked at the desired frequency)

	// Use two configurable logic cells to automatically stop the sound when the timer has elapsed
	// This will save an interrupt during the game execution and also make use some of the transistors packed in the chip
	// Below is the logic diagram :
	//                                          +-------+
	//      2KHz PWM5_out ----------------------|       |
	//                                          |   &   |---- Output pin
	//                                     +----|       |
	//                                     |    +-------+
	//                        +-------+    |      CLC1
	//           TMR4_out ----| R     |    |
	//                        |     Q |----+
	// Sofware controlled ----| S     |
	//                        +-------+
	//                          CLC2
	//
	// Configure the Configurable Logic Cell 2 as a S-R Latch, it is set by software and reset when the timer elapses (the timer generates a pulse when it reaches the programmed counter value)
	// Select the input signals (inputs 1 and 2 are connected to the "set" input, inputs 3 and 4 are connected to the "reset" input)
	CLC2SEL0 = 0; // Triggered by software
	CLC2SEL1 = 0; // Not used
	CLC2SEL2 = 0x10; // The TMR4_out signal
	CLC2SEL3 = 0; // Not used
	// Enable the input signals through the gates
	CLC2GLS0 = 0; // Prepare to put a logic 1 by software
	CLC2GLS1 = 0; // Second input is not used, prepare to output a logic 0
	CLC2GLS2 = 0x20; // Only non-inverted data 3 is gated into gate 2
	CLC2GLS3 = 0; // Fourth input is not used, prepare to output a logic 0
	// Select each gate polarity
	CLC2POL = 0; // Do not invert the output of the logic cell, do not invert the input signals
	// Configure the logic function
	CLC2CON = 0x83; // Enable the logic cell, select a S-R latch function

	// Use a Configurable Logic Cell to AND the buzzer PWM output signal with the gating S-R latch, so the sound is turned off when the latch is cleared
	// Select the input signals
	CLC1SEL0 = 0x18; // The PWM5_out signal
	CLC1SEL1 = 0x10; // The CLC2_out signal
	CLC1SEL2 = 0; // Not used
	CLC1SEL3 = 0; // Not used
	// Enable the input signals through the gates
	CLC1GLS0 = 0x02; // Only non-inverted data 1 is gated into gate 0
	CLC1GLS1 = 0x04; // Only non-inverted data 2 is gated into gate 1
	CLC1GLS2 = 0; // Third input is not used, prepare to output a logic 1
	CLC1GLS3 = 0; // Fourth input is not used, prepare to output a logic 1
	// Select each gate polarity
	CLC1POL = 0x0C; // Do not invert the output of the logic cell, invert the gates 2 and 3 outputs to generate a logic one for these 2 inputs, do not invert the output of the used signals
	// Configure the logic function
	CLC1CON = 0x82; // Enable the logic cell, select a 4-input AND function

	// Configure the pin that drives the buzzer
	RC2PPS = 0x01; // Select the RC2 pin as the Configurable Logic Cell 1 output
	ANSELCbits.ANSELC2 = 0; // Configure the pin as digital
	TRISCbits.TRISC2 = 0; // Set the pin as output

	// Enable the waveform generation
	PWM5CONbits.EN = 1; // Start the PWM
	T2CONbits.ON = 1; // Start the PWM clocking timer
}

void SoundPlay(unsigned char Duration)
{
	// Stop the timer in case it is already running
	T4CONbits.ON = 0;

	// Set the target duration
	T4PR = Duration;

	// Reset the timer counter
	T4TMR = 0;

	// Set the S-R latch to enable the PWM signal on the output pin
	CLC2POLbits.G1POL = 1; // Generate a logic "1" to set the latch
	CLC2POLbits.G1POL = 0; // The latch is set, no need to drive its input anymore

	// Restart the timer
	T4CONbits.ON = 1;
}
