/** @file Interpreter.h
 * A Chip-8 / Super Chip-8 virtual machines interpreter.
 * @author Adrien RICCIARDI
 */
#ifndef H_INTERPRETER_H
#define H_INTERPRETER_H

#include <FAT.h>

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** TODO */
unsigned char InterpreterLoadProgramFromFile(TFATFileInformation *Pointer_File_Information);

/** TODO */
unsigned char InterpreterRunProgram(void);

#endif
