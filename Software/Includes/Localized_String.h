/** @file Localized_String.h
 * Gather all user interface strings that can be dynamically queried according to the configured language.
 * @author Adrien RICCIARDI
 */
#ifndef H_LOCALIZED_STRING_H
#define H_LOCALIZED_STRING_H

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** All supported languages. */
typedef enum : unsigned char
{
	LOCALIZED_STRING_LANGUAGE_ID_ENGLISH,
	LOCALIZED_STRING_LANGUAGE_ID_FRENCH,
	LOCALIZED_STRING_LANGUAGE_IDS_COUNT
} TLocalizedStringLanguageID;

/** All localized strings. */
typedef enum
{
	LOCALIZED_STRING_ID_MAIN_MENU_VIEW_TITLE,
	LOCALIZED_STRING_ID_MAIN_MENU_VIEW_CONTENT,

	LOCALIZED_STRING_ID_SETTINGS_MENU_VIEW_TITLE,
	LOCALIZED_STRING_ID_SETTINGS_MENU_VIEW_CONTENT,
	LOCALIZED_STRING_ID_SETTINGS_MENU_SOUND_DISABLED,
	LOCALIZED_STRING_ID_SETTINGS_MENU_SOUND_LOW,
	LOCALIZED_STRING_ID_SETTINGS_MENU_SOUND_MEDIUM,
	LOCALIZED_STRING_ID_SETTINGS_MENU_SOUND_HIGH,

	LOCALIZED_STRING_ID_INFORMATION_MENU_VIEW_TITLE,
	LOCALIZED_STRING_ID_INFORMATION_MENU_VIEW_CONTENT,

	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_TITLE,
	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_PROBE_ERROR_CONTENT,
	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_INSERT_SD_CARD_CONTENT,
	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_MBR_READ_ERROR_CONTENT,
	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_NO_VALID_PARTITION_ERROR_CONTENT,
	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_CONFIGURATION_FILE_LOADING_ERROR_CONTENT,
	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_NO_CONFIGURATION_FILE_FOUND_CONTENT,
	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_GAME_LOADING_ERROR_CONTENT,
	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_NO_GAME_FOUND_IN_CONFIGURATION_ERROR_CONTENT,
	LOCALIZED_STRING_ID_SD_CARD_MESSAGE_NO_VIDEO_FILE_FOUND_ERROR_CONTENT,
	LOCALIZED_STRING_IDS_COUNT
} TLocalizedStringID;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Load the language configuration from the configuration. */
void LocalizedStringInitialize(void);

/** Retrieve the string corresponding to the specified ID according to the selected language.
 * @param ID The string identifier.
 * @return A pointer to a string, located in code memory.
 */
const char *LocalizedStringGet(TLocalizedStringID ID);

/** Cycle to the next available language, going back to the first one when the last one has been reached. */
void LocalizedStringSelectNextLanguage(void);

#endif
