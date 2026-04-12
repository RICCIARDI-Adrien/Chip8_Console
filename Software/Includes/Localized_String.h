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

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Load the language configuration from the configuration. */
void LocalizedStringInitialize(void);

#endif
