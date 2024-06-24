/** @file Shared_Buffer.h
 * Minimize RAM usage by reusing some big buffers across different modules.
 * @author Adrien RICCIARDI
 */
#ifndef H_SHARED_BUFFER_H
#define H_SHARED_BUFFER_H

#include <Display.h>

//-------------------------------------------------------------------------------------------------
// Variables
//-------------------------------------------------------------------------------------------------
/** The display frame buffer, it can contain incompatible data when used in text mode or in Chip-8/SuperChip-8 graphic mode. */
extern unsigned char Shared_Buffer_Display[DISPLAY_COLUMNS_COUNT * DISPLAY_ROWS_COUNT / 8];

#endif
