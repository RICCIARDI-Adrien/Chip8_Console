/** @file EEPROM.h
 * Simple byte read and write access to the microcontroller internal EEPROM.
 * @author Adrien RICCIARDI
 */
#ifndef H_EEPROM_H
#define H_EEPROM_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** How many bytes the EEPROM can store. */
#define EEPROM_SIZE 1024

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** All mapped EEPROM memory locations. */
typedef enum
{
	EEPROM_ADDRESS_IS_MEMORY_CONTENT_INITIALIZED, //!< Tell whether the EEPROM contains valid data.
	EEPROM_ADDRESS_SOUND_LEVEL_PERCENTAGE, //!< The buzzer sound level.
	EEPROM_ADDRESS_DISPLAY_BRIGHTNESS, //!< The display pixels brightness.
	EEPROM_ADDRESS_INTERPRETER_FLAG_REGISTER_0,
	EEPROM_ADDRESS_INTERPRETER_FLAG_REGISTER_1,
	EEPROM_ADDRESS_INTERPRETER_FLAG_REGISTER_2,
	EEPROM_ADDRESS_INTERPRETER_FLAG_REGISTER_3,
	EEPROM_ADDRESS_INTERPRETER_FLAG_REGISTER_4,
	EEPROM_ADDRESS_INTERPRETER_FLAG_REGISTER_5,
	EEPROM_ADDRESS_INTERPRETER_FLAG_REGISTER_6,
	EEPROM_ADDRESS_INTERPRETER_FLAG_REGISTER_7
} TEEPROMAddress;

/** All possible sound level percentages, can be provided as-is to the SoundSetLevel() function. */
typedef enum
{
	EEPROM_SOUND_LEVEL_PERCENTAGE_OFF = 0,
	EEPROM_SOUND_LEVEL_PERCENTAGE_LOW = 8,
	EEPROM_SOUND_LEVEL_PERCENTAGE_MEDIUM = 25,
	EEPROM_SOUND_LEVEL_PERCENTAGE_HIGH = 100
} TEEPROMSoundLevelPercentage;

/** All possible brightness values, can be provided as-is to the DisplaySetBrightness() function. */
typedef enum
{
	EEPROM_DISPLAY_BRIGHTNESS_LOW = 0,
	EEPROM_DISPLAY_BRIGHTNESS_MEDIUM = 63,
	EEPROM_DISPLAY_BRIGHTNESS_HIGH = 127
} TEEPROMDisplayBrightness;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Read a byte from the internal EEPROM.
 * @param Address The byte address. If an invalid address is provided, the function immediately returns with the value 0.
 * @return The read byte content.
 */
unsigned char EEPROMReadByte(unsigned short Address);

/** Write a byte of data into the EEPROM area.
 * @param Address The byte address. If an invalid address is provided, the function immediately returns.
 * @param Value The byte content.
 */
void EEPROMWriteByte(unsigned short Address, unsigned char Value);

#endif
