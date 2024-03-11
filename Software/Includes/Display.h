/** @file Display.h
 * Control an OLED display using a SS1309 controller through a 4-wire SPI bus.
 * @author Adrien RICCIARDI
 */
#ifndef H_DISPLAY_H
#define H_DISPLAY_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** The amount of horizontal pixels in the display area. */
#define DISPLAY_COLUMNS_COUNT 128
/** The amount of vertical pixels in the display area. */
#define DISPLAY_ROWS_COUNT 64

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the display controller, clear its data RAM and turn the display on. */
void DisplayInitialize(void);

/** Convert on-the-fly a Chip-8 frame buffer (with 8 horizontal consecutive pixels per byte) to the display 8 vertical pixels per byte format, and send the picture to the display memory.
 * @param Pointer_Buffer The Chip-8 frame buffer.
 */
void DisplayShowBuffer(void *Pointer_Buffer);

#endif
