/** @file Localized_String.c
 * See Localized_String.h
 * @author Adrien RICCIARDI
 */
#include <Display.h>
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
		"A. Jeux\nB. Lecteur vid" DISPLAY_CHARACTER_E_ACUTE "o\nC. Param" DISPLAY_CHARACTER_E_GRAVE "tres\nD. Informations\n\nBatterie : %u%%"
	},

	// LOCALIZED_STRING_ID_SETTINGS_MENU_VIEW_TITLE
	{
		"- Settings -",
		"- Param" DISPLAY_CHARACTER_E_GRAVE "tres -"
	},
	// LOCALIZED_STRING_ID_SETTINGS_MENU_VIEW_CONTENT
	{
		"Sound (A) : %s\nBrightness (B) : %sLanguage (C): English\n\nMenu : back.",
		"Son (A) : %s\nLuminosit" DISPLAY_CHARACTER_E_ACUTE " (B) : %sLangue (C) : Fran" DISPLAY_CHARACTER_C_CEDILLA "ais\n\nMenu : retour."
	},
	// LOCALIZED_STRING_ID_SETTINGS_MENU_SOUND_DISABLED
	{
		"disabled",
		"coup" DISPLAY_CHARACTER_E_ACUTE
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
	},

	// LOCALIZED_STRING_ID_INFORMATION_MENU_VIEW_TITLE
	{
		"- Information -",
		"- Informations -"
	},
	// LOCALIZED_STRING_ID_INFORMATION_MENU_VIEW_CONTENT
	{
		"Firmware : V%sDate : %s\nTime : %s\n\n\nMenu : back.",
		"Logiciel : V%sDate : %s\nHeure : %s\n\n\nMenu : retour."
	},

	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_TITLE
	{
		"SD card",
		"Carte SD"
	},
	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_PROBE_ERROR_CONTENT
	{
		"Failed to probe the\nSD card.\nInsert another SD\ncard and press Menu.",
		"Erreur de d" DISPLAY_CHARACTER_E_ACUTE "tection\nde la carte SD.\nRemplacez-la puis\nappuyez sur Menu."
	},
	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_INSERT_SD_CARD_CONTENT
	{
		"Please insert a SD\ncard.",
		"Veuillez ins" DISPLAY_CHARACTER_E_ACUTE "rer\nune carte SD."
	},
	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_MBR_READ_ERROR_CONTENT
	{
		"Failed to read the SDcard MBR block.\nInsert another SD\ncard and press Menu.",
		"Erreur de lecture du\nMBR de la carte SD.\nRemplacez-la puis\nappuyez sur Menu."
	},
	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_NO_VALID_PARTITION_ERROR_CONTENT
	{
		"No valid partition\ncould be found.\nInsert another SD\ncard and press Menu.",
		"Aucune partition\nvalide d" DISPLAY_CHARACTER_E_ACUTE "tect" DISPLAY_CHARACTER_E_ACUTE "e.\nRemplacez la carte\nSD puis appuyez sur\nMenu."
	},
	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_CONFIGURATION_FILE_LOADING_ERROR_CONTENT
	{
		"Failed to load the\nconfiguration file.\nReplace SD card and\npress Menu.",
		"Erreur de chargement\ndu fichier de\nconfiguration.\nRemplacez la carte SDpuis appuyez sur\nMenu."
	},
	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_NO_CONFIGURATION_FILE_FOUND_CONTENT
	{
		"No configuration filefound. Replace the SDcard and press Menu.",
		"Fichier de configu-\nration introuvable.\nRemplacez la carte SDpuis appuyez sur\nMenu."
	},
	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_GAME_LOADING_ERROR_CONTENT
	{
		"Failed to load the\ngame. Replace the SD\ncard and press Menu.",
		"Erreur lors du\nchargement du jeu.\nRemplacez la carte SDpuis appuyez sur\nMenu."
	},
	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_NO_GAME_FOUND_IN_CONFIGURATION_ERROR_CONTENT
	{
		"No game found in the\nconfiguration file.\nReplace the SD card\nand press Menu.",
		"Jeu introuvable dans\nle fichier de\nconfiguration.\nRemplacez la carte SDpuis appuyez sur\nMenu."
	},
	// LOCALIZED_STRING_ID_SD_CARD_MESSAGE_NO_VIDEO_FILE_FOUND_ERROR_CONTENT
	{
		"No video file found.\nReplace the SDcard\nand press Menu.",
		"Fichier vid" DISPLAY_CHARACTER_E_ACUTE "o non\ntrouv" DISPLAY_CHARACTER_E_ACUTE ". Remplacez la\ncarte SD puis appuyezsur Menu."
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
