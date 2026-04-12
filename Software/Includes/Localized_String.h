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
