/** @file INI_Parser.c
 * See INI_Parser.h for description.
 * @author Adrien RICCIARDI
 */
#include <INI_Parser.h>
#include <Serial_Port.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define INI_PARSER_IS_LOGGING_ENABLED 0

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Increment the string pointer until a specific character is found.
 * @param Pointer_String The location to start searching from.
 * @param Character The searched character.
 * @return NULL if the INI end has been reached before finding the character,
 * @return A pointer to the found character in the INI buffer.
 */
static char *INIParserSearchCharacter(char *Pointer_String, char Character)
{
	char Current_Character;

	while (1)
	{
		// Cache the current character
		Current_Character = *Pointer_String;

		// Exit if the end of the data is reached
		if (Current_Character == INI_PARSER_END_CHARACTER) return NULL;
		// Exit if the character has been found
		if (Current_Character == Character) return Pointer_String;

		// Update the pointer at the end so the returned string points on the searched character
		Pointer_String++;
	}
}

/** Increment the string pointer while a space or space-like separation character is found.
 * @param Pointer_String The location to start from.
 * @return NULL if the INI end has been reached,
 * @return A pointer to the first found character in the INI buffer.
 */
static char *INIParserDiscardWhiteSpace(char *Pointer_String)
{
	char Character;

	while (1)
	{
		// Cache the current character
		Character = *Pointer_String;

		// Exit if the end of the data is reached
		if (Character == INI_PARSER_END_CHARACTER) return NULL;
		// Exit if this is not a separation character
		if ((Character != ' ') && (Character != '\t') && (Character != '\n') && (Character != 0)) return Pointer_String;

		// Update the pointer at the end so the returned string points on the first non-space character
		Pointer_String++;
	}
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
char *INIParserFindPreviousSection(char *Pointer_String_Buffer_Start, char *Pointer_String_Current_Section)
{
	// Search for the ']' character of the previous section
	while (1)
	{
		// Is the beginning of the buffer reached ?
		if (Pointer_String_Current_Section < Pointer_String_Buffer_Start) return NULL;

		if (*Pointer_String_Current_Section == ']') break;
		Pointer_String_Current_Section--;
	}

	// The end of the previous section name has been found, go up to the beginning of the section name, so the returned string is the same than the one returned by INIParserFindNextSection()
	while (1)
	{
		// Is the beginning of the buffer reached ?
		if (Pointer_String_Current_Section < Pointer_String_Buffer_Start) return NULL;

		if (*Pointer_String_Current_Section == '[') return Pointer_String_Current_Section + 1; // Point to the first character of the section name
		Pointer_String_Current_Section--;
	}
}

char *INIParserFindNextSection(char *Pointer_String_Current_Section)
{
	char *Pointer_String;

	Pointer_String = INIParserSearchCharacter(Pointer_String_Current_Section, '[');
	if (Pointer_String == NULL) return NULL;

	// Bypass the '[' character, so a next call to INIParserFindNextSection() will return the next section without needed to modify the pointer
	Pointer_String++;
	return Pointer_String;
}

char *INIParserReadString(char *Pointer_String_Section, const char *Pointer_String_Key_Name)
{
	char *Pointer_String, *Pointer_String_Key_Beginning, Character;
	unsigned char Length, Searched_Key_Length;

	// Go to the end of the section name
	Pointer_String = INIParserSearchCharacter(Pointer_String_Section, ']');
	if (Pointer_String == NULL)
	{
		SERIAL_PORT_LOG(INI_PARSER_IS_LOGGING_ENABLED, "Error : unterminated section \"%s\".", Pointer_String_Section);
		return NULL;
	}

	// Search for the first key character
	Pointer_String++; // Bypass the section closing ']'
	Pointer_String = INIParserDiscardWhiteSpace(Pointer_String);
	if (Pointer_String == NULL)
	{
		SERIAL_PORT_LOG(INI_PARSER_IS_LOGGING_ENABLED, "Error : no keys present in the section \"%s\".", Pointer_String_Section);
		return NULL;
	}

	// Search through all keys
	Searched_Key_Length = (unsigned char) strlen(Pointer_String_Key_Name);
	while (1)
	{
		// Right after the first loop, point just after the key name to be able to search for the next key
		Pointer_String_Key_Beginning = Pointer_String;

		// Find the key name end
		while (1)
		{
			Character = *Pointer_String;
			if ((Character == INI_PARSER_END_CHARACTER) || (Character == '\n') || (Character == 0))
			{
				SERIAL_PORT_LOG(INI_PARSER_IS_LOGGING_ENABLED, "No more '=' character found in the keys area of this section.");
				return NULL;
			}
			if (Character == '=') break;
			Pointer_String++; // Increment the pointer at the end to point on the '=' on exit
		}

		Length = (unsigned char) (Pointer_String - Pointer_String_Key_Beginning);
		Pointer_String++; // Bypass the '=' character
		// Is it the researched key name ?
		if ((Length == Searched_Key_Length) && (memcmp(Pointer_String_Key_Beginning, Pointer_String_Key_Name, Length) == 0))
		{
			// Convert to an ASCIIZ string, so the string can be directly used without needing to be copied
			Pointer_String_Key_Beginning = Pointer_String;
			while (1)
			{
				Character = *Pointer_String;
				if ((Character == INI_PARSER_END_CHARACTER) || (Character == '\n') || (Character == 0))
				{
					// The last byte of the buffer can be set to 0 without risking to overflow the next time the file will be parsed because there are appositely two terminating characters at the end
					*Pointer_String = 0;
					return Pointer_String_Key_Beginning;
				}
				Pointer_String++;
			}
		}

		// Go to this key value end (this can be delimited by a new line or a 0 if the key has already been read)
		while (1)
		{
			Character = *Pointer_String;
			if (Character == INI_PARSER_END_CHARACTER)
			{
				SERIAL_PORT_LOG(INI_PARSER_IS_LOGGING_ENABLED, "This was the last key of the INI file.");
				return NULL;
			}
			if ((Character == '\n') || (Character == 0)) break;
			Pointer_String++;
		}

		// Go to the first character of the next key name
		Pointer_String = INIParserDiscardWhiteSpace(Pointer_String);
		if (Pointer_String == NULL)
		{
			SERIAL_PORT_LOG(INI_PARSER_IS_LOGGING_ENABLED, "This was the last key of the INI file.");
			return NULL;
		}

		// Stop if the beginning of another section is found
		if (*Pointer_String == '[')
		{
			SERIAL_PORT_LOG(INI_PARSER_IS_LOGGING_ENABLED, "Found the beginning of the next section, stopping.");
			return NULL;
		}
	}
}

unsigned char INIParserRead8BitInteger(char *Pointer_String_Section, const char *Pointer_String_Key_Name)
{
	char *Pointer_String_Value;

	// Find the string containing the key data
	Pointer_String_Value = INIParserReadString(Pointer_String_Section, Pointer_String_Key_Name);
	if (Pointer_String_Value == NULL)
	{
		SERIAL_PORT_LOG(INI_PARSER_IS_LOGGING_ENABLED, "Could not find the key named \"%s\".", Pointer_String_Key_Name);
		return 1;
	}

	return (unsigned char) atoi(Pointer_String_Value);
}
