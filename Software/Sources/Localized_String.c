/** @file Localized_String.c
 * See Localized_String.h
 * @author Adrien RICCIARDI
 */
#include <EEPROM.h>
#include <Localized_String.h>
#include <Log.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define LOCALIZED_STRING_IS_LOGGING_ENABLED 1

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Cache the current configured language. */
static TLocalizedStringLanguageID Localized_String_Current_Language_ID = LOCALIZED_STRING_LANGUAGE_ID_ENGLISH;

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void LocalizedStringInitialize(void)
{
	// Cache the language from the configuration
	Localized_String_Current_Language_ID = EEPROMReadByte(EEPROM_ADDRESS_SYSTEM_LANGUAGE);

	// Make sure that the EEPROM contains a valid value
	if (Localized_String_Current_Language_ID >= LOCALIZED_STRING_LANGUAGE_IDS_COUNT)
	{
		LOG(LOCALIZED_STRING_IS_LOGGING_ENABLED, "The language ID %u stored in EEPROM is invalid, defaulting to English.", Localized_String_Current_Language_ID);
		Localized_String_Current_Language_ID = LOCALIZED_STRING_LANGUAGE_ID_ENGLISH;
		EEPROMWriteByte(EEPROM_ADDRESS_SYSTEM_LANGUAGE, Localized_String_Current_Language_ID);
	}

	LOG(LOCALIZED_STRING_IS_LOGGING_ENABLED, "System language ID is %u.", Localized_String_Current_Language_ID);
}
