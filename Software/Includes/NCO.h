/** @file NCO.h
 * Use the Numerically Controlled Oscillator module to generate a 60Hz clock to feed other timers with (the virtual machine sound and delay timers).
 * Use the NCO for that to free as many timers as possible for the other parts of the console (or for more demanding games).
 * @author Adrien RICCIARDI
 */
#ifndef H_NCO_H
#define H_NCO_H

#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Macros
//-------------------------------------------------------------------------------------------------
/** The NCO module is configured to generate a 60Hz frequency. Its interrupt flag is then toggling at 60Hz too. Use it as a rendering tick here. */
#define NCO_60HZ_TICK() (PIR4bits.NCO1IF == 1)

/** Clear the interrupt flag at an appropriate moment chosen by the user (the interrupt flag must be manually cleared). */
#define NCO_CLEAR_TICK_INTERRUPT_FLAG() PIR4bits.NCO1IF = 0

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the NCO module to internally generate a 60Hz square wave. */
void NCOInitialize(void);

#endif
