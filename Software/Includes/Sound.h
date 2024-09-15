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

/** Allow to turn off the sound generation.
 * @param Is_Enabled Set to 1 to enable sound generation, or set to 0 to mute the sound.
 */
void SoundSetEnabled(unsigned char Is_Enabled);

/** Tell the current sound enabling state.
 * @return 0 if the sound generation is disabled, 1 otherwise.
 */
unsigned char SoundIsEnabled(void);

/** Allow to configure the level of the generated sound.
 * @param Level_Percentage Set to 0 to mute the sound, set to 100 to for maximum sound level.
 */
void SoundSetLevel(unsigned char Level_Percentage);

#endif
