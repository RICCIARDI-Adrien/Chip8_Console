/** @file NCO.h
 * Use the Numerically Controlled Oscillator module to generate a 60Hz clock to feed other timers with (the virtual machine sound and delay timers).
 * Use the NCO for that to free as many timers as possible for the other parts of the console (or for more demanding games).
 * @author Adrien RICCIARDI
 */
#ifndef H_NCO_H
#define H_NCO_H

#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Constants and macros
//-------------------------------------------------------------------------------------------------
/** The internal counter value to achieve a 60Hz tick frequency. */
#define NCO_TICK_FREQUENCY_60HZ 3932 // Theoretical effective value is 59.998Hz

/** The NCO module is configured to generate a specific frequency. Its interrupt flag is then toggling at this frequency too. Use it as a rendering tick here. */
#define NCO_IS_TICK_ELAPSED() (PIR4bits.NCO1IF == 1)

/** Clear the interrupt flag at an appropriate moment chosen by the user (the interrupt flag must be manually cleared). */
#define NCO_CLEAR_TICK_INTERRUPT_FLAG() PIR4bits.NCO1IF = 0

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Enable the NCO module power and clocks. */
void NCOInitialize(void);

/** Configure the NCO module to internally generate a square wave of the specified frequency. The NCO is stopped, reconfigured then started again.
 * @param Tick_Frequency Use a constant from the NCO_TICK_FREQUENCY_xx pool to select the tick frequency.
 */
void NCOConfigure(unsigned short Tick_Frequency);

#endif
