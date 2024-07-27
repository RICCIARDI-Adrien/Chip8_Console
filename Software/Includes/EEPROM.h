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

/** The address in the EEPROM of the byte where the sound generation enabling state is stored. */
#define EEPROM_DATA_ADDRESS_IS_SOUND_ENABLED 0

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
