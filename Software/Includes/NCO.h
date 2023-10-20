/** @file NCO.h
 * Use the Numerically Controlled Oscillator module to generate a 60Hz clock to feed other timers with (the virtual machine sound and delay timers).
 * Use the NCO for that to free as many timers as possible for the other parts of the console (or for more demanding games).
 * @author Adrien RICCIARDI
 */
#ifndef H_NCO_H
#define H_NCO_H

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the NCO module to internally generate a 60Hz square wave. */
void NCOInitialize(void);

#endif
