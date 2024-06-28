/** @file INI_Parser.c
 * See INI_Parser.h for description.
 * @author Adrien RICCIARDI
 */
#include <INI_Parser.h>
#include <stddef.h>

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

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
char *INIParserFindNextSection(char *Pointer_String_Current_Section)
{
	return INIParserSearchCharacter(Pointer_String_Current_Section, '[');
}
