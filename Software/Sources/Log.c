/** @file Log.c
 * See Log.h for description.
 * @author Adrien RICCIARDI
 */
#include <Log.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void LogInitialize(void)
{
	#ifdef LOG_IS_ENABLED
		// Initialize the timer 0 to count at a 1us period
		T0CON0 = 0x10; // Do not enable the timer yet, select the 16-bit mode, do not use a postscaler
		T0CON1 = 0x44; // Use Fosc/4 as clock source, synchronize to Fosc/4, use a 1:16 prescaler

		// Display the following message only if the serial port logging feature is enabled
		LOG(1, "Serial port logging is enabled.");
	#endif
}

#ifdef LOG_IS_ENABLED
	void LogTimingStart(void)
	{
		// Make sure that the timer is stopped
		T0CON0bits.EN = 0;

		// Clear the timer interrupt, it is used as an overflow flag
		PIR3bits.TMR0IF = 0;

		// Reset the timer value
		TMR0H = 0; // TMR0H is latched and written only when TMR0L is written
		TMR0L = 0;

		// Start counting
		T0CON0bits.EN = 1;
	}

	unsigned short LogTimingStop(void)
	{
		unsigned short Delay;

		// Stop the timer
		T0CON0bits.EN = 0;

		// Did the timer overflow ?
		if (PIR3bits.TMR0IF) return 0xFFFF;

		// Retrieve the timer value, starting by reading TMR0L to latch the value in TMR0H
		Delay = TMR0L;
		Delay |= (unsigned short) (TMR0H << 8);

		return Delay;
	}
#endif
