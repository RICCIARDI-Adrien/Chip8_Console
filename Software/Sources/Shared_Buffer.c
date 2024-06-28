/** @file Shared_Buffer.c
 * See Shared_Buffer.h for description.
 * @author Adrien RICCIARDI
 */
#include <Shared_Buffer.h>

//-------------------------------------------------------------------------------------------------
// Public variables
//-------------------------------------------------------------------------------------------------
unsigned char Shared_Buffer_Display[DISPLAY_COLUMNS_COUNT * DISPLAY_ROWS_COUNT / 8];

TSharedBuffers Shared_Buffers;
