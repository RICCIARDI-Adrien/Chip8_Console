/** @file Sound.h
 * Control the board buzzer.
 * @author Adrien RICCIARDI
 */
#ifndef H_SOUND_H
#define H_SOUND_H

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure a PWM module to generate the square wave frequency that drives the buzzer. */
void SoundInitialize(void);

/** Ring the buzzer as long as the CHIP-8 sound timer is greater than zero.
 * @param Duration The duration of the sound in units of 1/60Hz = 16.7ms.
 */
void SoundPlay(unsigned char Duration);

#endif
