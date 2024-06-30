/** @file Interpreter.h
 * A Chip-8 / Super Chip-8 virtual machines interpreter.
 * @author Adrien RICCIARDI
 */
#ifndef H_INTERPRETER_H
#define H_INTERPRETER_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** The interpreter memory size in bytes. */
#define INTERPRETER_MEMORY_SIZE 4096

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the interpreter timers. */
void InterpreterInitialize(void);

/** Load a ROM file from the SD card and prepare the Chip-8 virtual machine.
 * @param Pointer_String_Game_INI_Section The configuration file INI section containing the game information.
 * @return 1 if an error occurred,
 * @return 0 on success.
 */
unsigned char InterpreterLoadProgramFromFile(char *Pointer_String_Game_INI_Section);

/** Run the Chip-8 program until completion or the user presses the Menu key.
 * @return 1 if an error occurred,
 * @return 0 on success.
 */
unsigned char InterpreterRunProgram(void);

#endif
