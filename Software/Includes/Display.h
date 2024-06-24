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

/** Convert on-the-fly a Chip-8 frame buffer (with 8 horizontal consecutive pixels per byte) to the display 8 vertical pixels per byte format, scale it to the full display resolution, then send the picture to the display memory.
 * @param Pointer_Buffer The Chip-8 frame buffer (64x32 pixels).
 */
void DisplayDrawHalfSizeBuffer(void *Pointer_Buffer);

/** Convert on-the-fly a SuperChip-8 frame buffer (with 8 horizontal consecutive pixels per byte) to the display 8 vertical pixels per byte format, and send the picture to the display memory.
 * @param Pointer_Buffer The SuperChip-8 frame buffer (128x64 pixels).
 */
void DisplayDrawFullSizeBuffer(void *Pointer_Buffer);

/** Set the cursor position in text mode.
 * @param X The horizontal coordinate.
 * @param Y The vertical coordinate.
 */
void DisplaySetTextCursor(unsigned char X, unsigned char Y);

/** Render an ASCII character at the current cursor location.
 * @param Pointer_Buffer The buffer to render the character to.
 * @param Character The character ASCII code.
 */
void DisplayWriteCharacter(void *Pointer_Buffer, unsigned char Character);

/** Render a ASCII string at the current cursor location into the frame buffer.
 * @param Pointer_Buffer The buffer to render the string to.
 * @param Pointer_String The ASCII string to render.
 * @note The '\n' character is supported to put the cursor at the beginning of the next line.
 */
void DisplayWriteString(void *Pointer_Buffer, const char *Pointer_String);

/** Send the frame buffer to the display without any conversion, because in text mode the frame buffer encoding is the same than the display controller (1 byte corresponds to 8 consecutive vertical pixels).
 * @param Pointer_Buffer The frame buffer to display (128x64 pixels).
 */
void DisplayDrawTextBuffer(void *Pointer_Buffer);

/** Show a text message made of a title and text.
 * @param Pointer_Buffer The frame buffer in which the message will be rendered. The frame buffer will be automatically cleared by this function.
 * @param Pointer_String_Title The message title, it will be automatically centered and needs to fit on a single display line.
 * @param Pointer_String_Message The message text content, it can use several up to 6 lines.
 */
void DisplayDrawTextMessage(void *Pointer_Buffer, const char *Pointer_String_Title, const char *Pointer_String_Message);

#endif
