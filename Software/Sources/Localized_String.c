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

/** Store all localized versions of each string. */
static const char *Localized_String_Pointer_Strings[LOCALIZED_STRING_IDS_COUNT][LOCALIZED_STRING_LANGUAGE_IDS_COUNT] =
{
	// LOCALIZED_STRING_ID_MAIN_MENU_VIEW_TITLE
	{
		"- Main menu -",
		"- Menu principal -"
	},
	// LOCALIZED_STRING_ID_MAIN_MENU_VIEW_CONTENT
	{
		"A. Games\nB. Video player\nC. Settings\nD. Information\n\nBattery charge : %u%%",
		"A. Jeux\nB. Lecteur video\nC. Parametres\nD. Informations\n\nBatterie : %u%%"
	},

	// LOCALIZED_STRING_ID_SETTINGS_MENU_VIEW_TITLE
	{
		"- Settings -",
		"- Parametres -"
	},
	// LOCALIZED_STRING_ID_SETTINGS_MENU_VIEW_CONTENT
	{
		"Sound (A) : %s\nBrightness (B) : %sLanguage (C): English\n\nMenu : back.",
		"Son (A) : %s\nLuminosite (B) : %sLangue (C) : Francais\n\nMenu : retour."
	},
	// LOCALIZED_STRING_ID_SETTINGS_MENU_SOUND_DISABLED
	{
		"disabled",
		"coupe"
	},
	// LOCALIZED_STRING_ID_SETTINGS_MENU_SOUND_LOW
	{
		"low",
		"bas"
	},
	// LOCALIZED_STRING_ID_SETTINGS_MENU_SOUND_MEDIUM
	{
		"medium",
		"moyen"
	},
	// LOCALIZED_STRING_ID_SETTINGS_MENU_SOUND_HIGH
	{
		"high",
		"fort"
	}
};

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

const char *LocalizedStringGet(TLocalizedStringID ID)
{
	// Display this message only in debug mode
	if (ID >= LOCALIZED_STRING_IDS_COUNT) LOG(LOCALIZED_STRING_IS_LOGGING_ENABLED, "Error : invalid localized string ID %u.", ID);

	return Localized_String_Pointer_Strings[ID][Localized_String_Current_Language_ID];
}

void LocalizedStringSelectNextLanguage(void)
{
	Localized_String_Current_Language_ID++;

	// Go back to the first language when the last has been reached
	if (Localized_String_Current_Language_ID >= LOCALIZED_STRING_LANGUAGE_IDS_COUNT) Localized_String_Current_Language_ID = LOCALIZED_STRING_LANGUAGE_ID_ENGLISH;

	// Update the configuration
	EEPROMWriteByte(EEPROM_ADDRESS_SYSTEM_LANGUAGE, Localized_String_Current_Language_ID);
}
