/** @file INI_Parser.h
 * A simple INI file format parser.
 * @note The buffer containing the INI data must be followed by 2 bytes set to the value 0x03 (to delimit the INI end).
 * @note INI keys data are limited to 255 characters.
 * @author Adrien RICCIARDI
 */
#ifndef H_INI_PARSER_H
#define H_INI_PARSER_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** Allow to terminate the accessed keys data with a NUL character, making the strings directly readable by the application without needing to copy them to terminate them. */
#define INI_PARSER_END_CHARACTER 0x03 // Corresponds to the End-of-text ASCII character

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Locate the beginning of the previous section in the INI buffer.
 * @param Pointer_String_Buffer_Start The start if the INI buffer, to know when the beginning of the buffer has been reached.
 * @param Pointer_String_Current_Section On function first call, initialize Pointer_String_Current_Section to the end of the INI buffer. On successive calls, this buffer can point to whatever location in an existing section.
 * @return NULL if the beginning of the buffer was reached before finding the previous section,
 * @return A pointer on the beginning of the previous section (if such section is existing), pointing to the character right after the '['.
 */
char *INIParserFindPreviousSection(char *Pointer_String_Buffer_Start, char *Pointer_String_Current_Section);

/** Locate the beginning of the next section in the INI buffer.
 * @param Pointer_String_Current_Section On function first call, initialize Pointer_String_Current_Section to the beginning of the INI buffer. On successive calls, this buffer can point to whatever location in an existing section.
 * @return NULL if the end of the buffer was reached before finding the next section,
 * @return A pointer on the beginning of the next section (if such section is existing), pointing to the character right after the '['.
 */
char *INIParserFindNextSection(char *Pointer_String_Current_Section);

/** Retrieve a string value associated to a key in the specified section.
 * @param Pointer_String_Section Pointer to the beginning of the section.
 * @param Pointer_String_Key_Name The key name. It is case sensitive.
 * @return NULL if the end of the buffer was reached before finding the key,
 * @return A pointer to the beginning of the string if it was found. The string is zero-terminated and can be read as-is. Do not modify the string or you will break the INI content.
 */
char *INIParserReadString(char *Pointer_String_Section, const char *Pointer_String_Key_Name);

/** Retrieve an unsigned 8-bit value associated to a key in the specified section.
 * @param Pointer_String_Section Pointer to the beginning of the section.
 * @param Pointer_String_Key_Name The key name. It is case sensitive.
 * @return The integer value converted to binary,
 * @return 0 if the key value was not convertible to a number.
 */
unsigned char INIParserRead8BitInteger(char *Pointer_String_Section, const char *Pointer_String_Key_Name);

#endif
