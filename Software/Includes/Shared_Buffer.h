/** @file Shared_Buffer.h
 * Minimize RAM usage by reusing some big buffers across different modules.
 * @author Adrien RICCIARDI
 */
#ifndef H_SHARED_BUFFER_H
#define H_SHARED_BUFFER_H

#include <Display.h>
#include <Interpreter.h>

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** Reuse the memory space for features that do not require the memory at the same time. */
typedef union
{
	/** The Chip-8 / SuperChip-8 interpreter memory storing the loaded program. */
	unsigned char Interpreter_Memory[INTERPRETER_MEMORY_SIZE];
	/** The INI configuration loaded from the SD card. */
	char Configuration_File[INTERPRETER_MEMORY_SIZE];
	/** A temporary string with plenty of room. */
	char String_Temporary[INTERPRETER_MEMORY_SIZE];
	/** A temporary buffer with plenty of room. */
	unsigned char Buffer[INTERPRETER_MEMORY_SIZE];
} TSharedBuffers;

//-------------------------------------------------------------------------------------------------
// Variables
//-------------------------------------------------------------------------------------------------
/** The display frame buffer, it can contain incompatible data when used in text mode or in Chip-8/SuperChip-8 graphic mode. */
extern unsigned char Shared_Buffer_Display[DISPLAY_COLUMNS_COUNT * DISPLAY_ROWS_COUNT / 8];

/** The following buffers reuse the same memory space and must not be used at the same time. */
extern TSharedBuffers Shared_Buffers;

#endif
