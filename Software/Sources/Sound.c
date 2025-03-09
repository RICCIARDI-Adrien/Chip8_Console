/** @file Sound.c
 * See Sound.h for description.
 * @author Adrien RICCIARDI
 */
#include <EEPROM.h>
#include <Sound.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** The PWM period is a 8-bit value computed as follow : TxPR = ((1 / PWM_Frequency) / (4 * (1 / Fosc) * Prescaler)) - 1.
 * The desired PWM frequency is here 4KHz, with Fosc being 64MHz. By using a 1:16 prescaler, the computed value is an integer and fits into 8 bits.
 */
#define SOUND_PWM_PERIOD 249

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Cache the sound generation enabling state. */
static unsigned char Sound_Is_Enabled;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Interrupt handler for the timer 4 interrupt. */
static void __interrupt(irq(TMR4), high_priority) SoundInterrupt(void)
{
	// Stop the waveform generation
	PWM5CONbits.EN = 0;

	// Clear the interrupt flag
	PIR7bits.TMR4IF = 0;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void SoundInitialize(void)
{
	unsigned char Percentage;

	// Configure the PWM module
	CCPTMRS1 = (CCPTMRS1 & 0xFC) | 0x01; // Select TMR2 clock for PWM5
	PWM5CON = 0; // Do not enable the PWM for now, do not invert the output signal

	// Set the configured sound level
	Percentage = EEPROMReadByte(EEPROM_ADDRESS_SOUND_LEVEL_PERCENTAGE);
	SoundSetLevel(Percentage);

	// Configure the timer that clocks the PWM
	T2CLK = 0x01; // The datasheet tells that the timer must be clocked by Fosc/4 for proper PWM operations
	T2PR = SOUND_PWM_PERIOD;
	T2HLT = 0xA0; // Prescaler output is synchronized with Fosc/4, also synchronize the ON bit with the timer clock input, select the free running period mode with period pulse and software gate
	T2CON = 0xC0; // Enable the timer, configure a 1:16 prescaler and no postscaler as the latter is not used by the PWM module

	// Configure the timer that is used as the CHIP-8 sound timer
	T4CLK = 0x09; // Select the NCO as clock source (the NCO is outputting a clean 60Hz clock)
	T4HLT = 0xA8; // Prescaler output is synchronized with Fosc/4, also synchronize the ON bit with the timer clock input, select the one-shot mode with one-shot operation and software start
	T4CON = 0; // Do not enable the timer yet, do not enable any prescaler or postscaler (the timer is already clocked at the desired frequency)

	// Configure the pin that drives the buzzer
	RC2PPS = 0x0D; // Select the RC2 pin as the PWM5 output
	ANSELCbits.ANSELC2 = 0; // Configure the pin as digital
	TRISCbits.TRISC2 = 0; // Set the pin as output

	// Fire an interrupt when the duration timer overflows
	PIR7bits.TMR4IF = 0; // Make sure the interrupt flag is cleared to avoid triggering a false interrupt
	PIE7bits.TMR4IE = 1; // Enable the interrupt
}

void SoundPlay(unsigned char Duration)
{
	// Do nothing if the sound is muted
	if (!Sound_Is_Enabled) return;

	// Stop the timer in case it is already running
	T4CONbits.ON = 0;

	// Set the target duration
	T4PR = Duration;

	// Reset the timer counter
	T4TMR = 0;

	// Enable the waveform generation
	PWM5CONbits.EN = 1;

	// Restart the timer
	T4CONbits.ON = 1;
}

void SoundStop(void)
{
	// Stop the duration timer
	T4CONbits.ON = 0;

	// Stop the waveform generation
	PWM5CONbits.EN = 0;
}

void SoundSetLevel(unsigned char Level_Percentage)
{
	unsigned short Register_Value;

	// Clamp any percentage higher than 100%
	if (Level_Percentage > 100) Level_Percentage = 100;

	// Assume that the maximum sound level is reached when the duty cycle is 50% : with PWM frequency being 4KHz, the full duty cycle period is 1 / 4000, so for instance 50% duty cycle is 1 / 4000 / 2
	// The pulse width is a 10-bit value computed as follow : PWMxDC = Pulse_Width / ((1 / Fosc) * Prescaler)
	// Here, the oscillator frequency is 64MHz and the prescaler is 1:16, so the register value for a 50% duty cycle is 500
	Register_Value = (500 * Level_Percentage) / (unsigned short) 100; // As the percentage is clamped to 100, the multiplication can't overflow an unsigned 16-bit variable

	// Configure the duty cycle
	PWM5DCL = (unsigned char) ((Register_Value & 0x03) << 6);
	PWM5DCH = (unsigned char) (Register_Value >> 2);

	// Cache the sound enabling state to save some cycles when calling SoundPlay()
	if (Level_Percentage == 0) Sound_Is_Enabled = 0;
	else Sound_Is_Enabled = 1;
}
