/** @file Display.c
 * See Display.h for description.
 * @author Adrien RICCIARDI
 */
#include <Display.h>
#include <Serial_Port.h>
#include <Shared_Buffer.h>
#include <SPI.h>
#include <string.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define DISPLAY_IS_LOGGING_ENABLED 0

/** The display controller reset pin. */
#define DISPLAY_PIN_RESET LATBbits.LATB1
/** The display controller D/C pin. */
#define DISPLAY_PIN_DC LATBbits.LATB2

/** The D/C pin value to send a command to the display controller. */
#define DISPLAY_DC_MODE_COMMAND 0
/** The D/C pin value to send a data byte to the display controller. */
#define DISPLAY_DC_MODE_DATA 1

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The horizontal position of the cursor used for rendering text. */
static unsigned char Display_Text_Cursor_X = 0;
/** The vertical position of the cursor used for rendering text. */
static unsigned char Display_Text_Cursor_Y = 0;

/** First 128 ASCII characters sprites. */
static const unsigned char Display_Font_Sprites[][DISPLAY_TEXT_CHARACTER_WIDTH] =
{
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Space
	{ 0x00, 0x00, 0x4F, 0x00, 0x00, 0x00 }, // Punctuation '!'
	{ 0x00, 0x07, 0x00, 0x07, 0x00, 0x00 }, // Punctuation '"'
	{ 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00 }, // Punctuation '#'
	{ 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00 }, // Punctuation '$'
	{ 0x23, 0x13, 0x08, 0x64, 0x62, 0x00 }, // Punctuation '%'
	{ 0x36, 0x49, 0x55, 0x22, 0x50, 0x00 }, // Punctuation '&'
	{ 0x00, 0x05, 0x03, 0x00, 0x00, 0x00 }, // Punctuation '''
	{ 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00 }, // Punctuation '('
	{ 0x00, 0x41, 0x22, 0x1C, 0x00, 0x00 }, // Punctuation ')'
	{ 0x14, 0x08, 0x3E, 0x08, 0x14, 0x00 }, // Punctuation '*'
	{ 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00 }, // Punctuation '+'
	{ 0x00, 0x50, 0x30, 0x00, 0x00, 0x00 }, // Punctuation ','
	{ 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 }, // Punctuation '-'
	{ 0x00, 0x60, 0x60, 0x00, 0x00, 0x00 }, // Punctuation '.'
	{ 0x20, 0x10, 0x08, 0x04, 0x02, 0x00 }, // Punctuation '/'
	{ 0x3E, 0x41, 0x49, 0x41, 0x3E, 0x00 }, // Digit '0'
	{ 0x00, 0x04, 0x02, 0x7F, 0x00, 0x00 }, // Digit '1'
	{ 0x62, 0x51, 0x49, 0x49, 0x46, 0x00 }, // Digit '2'
	{ 0x22, 0x49, 0x49, 0x49, 0x36, 0x00 }, // Digit '3'
	{ 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00 }, // Digit '4'
	{ 0x4F, 0x49, 0x49, 0x49, 0x31, 0x00 }, // Digit '5'
	{ 0x3E, 0x49, 0x49, 0x49, 0x32, 0x00 }, // Digit '6'
	{ 0x01, 0x71, 0x09, 0x05, 0x03, 0x00 }, // Digit '7'
	{ 0x36, 0x49, 0x49, 0x49, 0x36, 0x00 }, // Digit '8'
	{ 0x26, 0x49, 0x49, 0x49, 0x3E, 0x00 }, // Digit '9'
	{ 0x00, 0x36, 0x36, 0x00, 0x00, 0x00 }, // Punctuation ':'
	{ 0x00, 0x56, 0x36, 0x00, 0x00, 0x00 }, // Punctuation ';'
	{ 0x08, 0x14, 0x22, 0x41, 0x00, 0x00 }, // Punctuation '<'
	{ 0x14, 0x14, 0x14, 0x14, 0x14, 0x00 }, // Punctuation '='
	{ 0x00, 0x41, 0x22, 0x14, 0x08, 0x00 }, // Punctuation '>'
	{ 0x02, 0x01, 0x51, 0x09, 0x06, 0x00 }, // Punctuation '?'
	{ 0x32, 0x49, 0x79, 0x41, 0x3E, 0x00 }, // Punctuation '@'
	{ 0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00 }, // Uppercase 'A'
	{ 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00 }, // Uppercase 'B'
	{ 0x3E, 0x41, 0x41, 0x41, 0x41, 0x00 }, // Uppercase 'C'
	{ 0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00 }, // Uppercase 'D'
	{ 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00 }, // Uppercase 'E'
	{ 0x7F, 0x09, 0x09, 0x09, 0x01, 0x00 }, // Uppercase 'F'
	{ 0x3E, 0x41, 0x49, 0x49, 0x79, 0x00 }, // Uppercase 'G'
	{ 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00 }, // Uppercase 'H'
	{ 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00 }, // Uppercase 'I'
	{ 0x20, 0x41, 0x41, 0x3F, 0x01, 0x00 }, // Uppercase 'J'
	{ 0x7F, 0x08, 0x14, 0x22, 0x41, 0x00 }, // Uppercase 'K'
	{ 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00 }, // Uppercase 'L'
	{ 0x7F, 0x02, 0x04, 0x02, 0x7F, 0x00 }, // Uppercase 'M'
	{ 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00 }, // Uppercase 'N'
	{ 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00 }, // Uppercase 'O'
	{ 0x7F, 0x09, 0x09, 0x09, 0x06, 0x00 }, // Uppercase 'P'
	{ 0x3E, 0x41, 0x41, 0x21, 0x5E, 0x00 }, // Uppercase 'Q'
	{ 0x7F, 0x09, 0x19, 0x29, 0x46, 0x00 }, // Uppercase 'R'
	{ 0x46, 0x49, 0x49, 0x49, 0x31, 0x00 }, // Uppercase 'S'
	{ 0x01, 0x01, 0x7F, 0x01, 0x01, 0x00 }, // Uppercase 'T'
	{ 0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00 }, // Uppercase 'U'
	{ 0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00 }, // Uppercase 'V'
	{ 0x3F, 0x40, 0x7E, 0x40, 0x3F, 0x00 }, // Uppercase 'W'
	{ 0x63, 0x14, 0x08, 0x14, 0x63, 0x00 }, // Uppercase 'X'
	{ 0x07, 0x08, 0x70, 0x08, 0x07, 0x00 }, // Uppercase 'Y'
	{ 0x61, 0x51, 0x49, 0x45, 0x43, 0x00 }, // Uppercase 'Z'
	{ 0x00, 0x7F, 0x41, 0x41, 0x00, 0x00 }, // Punctuation '['
	{ 0x02, 0x04, 0x08, 0x10, 0x20, 0x00 }, // Punctuation '\'
	{ 0x00, 0x41, 0x41, 0x7F, 0x00, 0x00 }, // Punctuation ']'
	{ 0x04, 0x02, 0x01, 0x02, 0x04, 0x00 }, // Punctuation '^'
	{ 0x20, 0x20, 0x20, 0x20, 0x20, 0x00 }, // Punctuation '_'
	{ 0x00, 0x01, 0x02, 0x04, 0x00, 0x00 }, // Punctuation '`'
	{ 0x20, 0x54, 0x54, 0x54, 0x78, 0x00 }, // Lowercase 'A'
	{ 0x7F, 0x48, 0x44, 0x44, 0x38, 0x00 }, // Lowercase 'B'
	{ 0x38, 0x44, 0x44, 0x44, 0x44, 0x00 }, // Lowercase 'C'
	{ 0x38, 0x44, 0x44, 0x48, 0x7F, 0x00 }, // Lowercase 'D'
	{ 0x38, 0x54, 0x54, 0x54, 0x18, 0x00 }, // Lowercase 'E'
	{ 0x08, 0x7E, 0x09, 0x01, 0x02, 0x00 }, // Lowercase 'F'
	{ 0x0C, 0x52, 0x52, 0x52, 0x3E, 0x00 }, // Lowercase 'G'
	{ 0x7F, 0x08, 0x04, 0x04, 0x78, 0x00 }, // Lowercase 'H'
	{ 0x00, 0x00, 0x7A, 0x00, 0x00, 0x00 }, // Lowercase 'I'
	{ 0x20, 0x40, 0x40, 0x3A, 0x00, 0x00 }, // Lowercase 'J'
	{ 0x7F, 0x10, 0x28, 0x44, 0x00, 0x00 }, // Lowercase 'K'
	{ 0x00, 0x41, 0x7F, 0x40, 0x00, 0x00 }, // Lowercase 'L'
	{ 0x7C, 0x04, 0x18, 0x04, 0x78, 0x00 }, // Lowercase 'M'
	{ 0x7C, 0x08, 0x04, 0x04, 0x78, 0x00 }, // Lowercase 'N'
	{ 0x38, 0x44, 0x44, 0x44, 0x38, 0x00 }, // Lowercase 'O'
	{ 0x7C, 0x14, 0x14, 0x14, 0x08, 0x00 }, // Lowercase 'P'
	{ 0x08, 0x14, 0x14, 0x18, 0x7C, 0x00 }, // Lowercase 'Q'
	{ 0x7C, 0x08, 0x04, 0x04, 0x08, 0x00 }, // Lowercase 'R'
	{ 0x08, 0x54, 0x54, 0x54, 0x20, 0x00 }, // Lowercase 'S'
	{ 0x04, 0x3F, 0x44, 0x40, 0x20, 0x00 }, // Lowercase 'T'
	{ 0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00 }, // Lowercase 'U'
	{ 0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00 }, // Lowercase 'V'
	{ 0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00 }, // Lowercase 'W'
	{ 0x44, 0x28, 0x10, 0x28, 0x44, 0x00 }, // Lowercase 'X'
	{ 0x0C, 0x50, 0x50, 0x50, 0x2C, 0x00 }, // Lowercase 'Y'
	{ 0x44, 0x64, 0x54, 0x4C, 0x44, 0x00 }, // Lowercase 'Z'
	{ 0x00, 0x08, 0x36, 0x41, 0x00, 0x00 }, // Punctuation '{'
	{ 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00 }, // Punctuation '|'
	{ 0x00, 0x41, 0x36, 0x08, 0x00, 0x00 }, // Punctuation '}'
	{ 0x08, 0x04, 0x08, 0x10, 0x08, 0x00 }, // Punctuation '~'
	{ 0x7F, 0x41, 0x41, 0x41, 0x7F, 0x00 }, // ASCII code 127 (DEL), use its sprite to represent an unknown character
};

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void DisplayInitialize(void)
{
	unsigned short i;

	SPI_SELECT_DISPLAY();

	// Configure the reset and A0 pins
	// Configure the pins as digital
	ANSELB &= 0xF9;
	// Configure the pins direction
	TRISB &= 0xF9;

	// Reset the display controller
	DISPLAY_PIN_RESET = 0;
	__delay_ms(10);
	DISPLAY_PIN_RESET = 1;
	__delay_ms(100);

	// Configure the horizontal addressing mode, so the row and column indexes are increased accordingly after each data write, returning to the beginning of the display when all pixels data have been received
	DISPLAY_PIN_DC = DISPLAY_DC_MODE_COMMAND;
	SPITransferByte(0x20);
	SPITransferByte(0x00);
	__delay_ms(10);

	// Clear the data RAM to avoid displaying random pixels
	DISPLAY_PIN_DC = DISPLAY_DC_MODE_DATA;
	for (i = 0; i < DISPLAY_COLUMNS_COUNT * DISPLAY_ROWS_COUNT; i++) SPITransferByte(0);

	// Turn the display on now that it is configured
	DISPLAY_PIN_DC = DISPLAY_DC_MODE_COMMAND;
	SPITransferByte(0xAF);
	__delay_ms(10);

	// Leave the display in data mode by default
	DISPLAY_PIN_DC = DISPLAY_DC_MODE_DATA;

	SPI_DESELECT_DISPLAY();
}

void DisplayDrawHalfSizeBuffer(void *Pointer_Buffer)
{
	unsigned char Row, Column, *Pointer_Buffer_Bytes = Pointer_Buffer, Display_Byte, i, j, Frame_Buffer_Chunk[4], Pixel_Mask;

	SPI_SELECT_DISPLAY();

	// Convert a chunk of 32 pixels (8x4 pixels) at a time, outputting 64 pixels (8x8 pixels)
	for (Row = 0; Row < DISPLAY_ROWS_COUNT / 2; Row += 4) // Load a chunk of 4 vertical bytes, so increment the row per 4
	{
		for (Column = 0; Column < (DISPLAY_COLUMNS_COUNT / 2) / 8; Column++) // There are 8 horizontal pixels per byte in the local frame buffer
		{
			// Load the 8 frame buffer horizontal bytes needed to create 16 vertical display buffer bytes (pixels are doubled)
			for (i = 0; i < 4; i++) Frame_Buffer_Chunk[i] = Pointer_Buffer_Bytes[(Row + i) * ((DISPLAY_COLUMNS_COUNT / 2) / 8) + Column];

			// Convert the frame buffer horizontal pixels to display controller expected vertical ones, also double the output pixels by a simple 2x scaling
			for (j = 0; j < 8; j++)
			{
				Display_Byte = 0;
				for (i = 0; i < 4; i++)
				{
					// The most significant bit of the display buffer is displayed starting from the display bottom
					if (Frame_Buffer_Chunk[i] & 0x80) Pixel_Mask = 0xC0; // Double the pixel vertically if the pixel is lit
					else Pixel_Mask = 0;
					Display_Byte |= Pixel_Mask;
					if (i < 3) Display_Byte >>= 2; // Append each pixel the to most significant bit of the byte, and do not shift the last time or the initial bit would be lost

					// Put the next buffer pixel bit to check to the most significant location
					Frame_Buffer_Chunk[i] <<= 1;
				}

				// The horizontal 8 pixels are ready to be displayed, write the byte twice to double the pixels horizontally
				SPITransferByte(Display_Byte);
				SPITransferByte(Display_Byte);
			}
		}
	}

	SPI_DESELECT_DISPLAY();
}

void DisplayDrawFullSizeBuffer(void *Pointer_Buffer)
{
	unsigned char Row, Column, *Pointer_Buffer_Bytes = Pointer_Buffer, Display_Byte, i, j, Frame_Buffer_Chunk[8], Pixel_Mask;

	SPI_SELECT_DISPLAY();

	// Convert and display a chunk of 64 pixels (8x8 pixels) at a time
	for (Row = 0; Row < DISPLAY_ROWS_COUNT; Row += 8) // Load a chunk of 8 vertical bytes, so increment the row per 8
	{
		for (Column = 0; Column < DISPLAY_COLUMNS_COUNT / 8; Column++) // There are 8 horizontal pixels per byte in the local frame buffer
		{
			// Load the 8 frame buffer horizontal bytes needed to create 8 vertical display buffer bytes
			for (i = 0; i < 8; i++) Frame_Buffer_Chunk[i] = Pointer_Buffer_Bytes[(Row + i) * (DISPLAY_COLUMNS_COUNT / 8) + Column];

			// Convert the frame buffer horizontal pixels to display controller expected vertical ones
			for (j = 0; j < 8; j++)
			{
				Display_Byte = 0;
				for (i = 0; i < 8; i++)
				{
					if (Frame_Buffer_Chunk[i] & 0x80) Pixel_Mask = 0x80; // The most significant bit of the display buffer is displayed starting from the display bottom
					else Pixel_Mask = 0;
					Display_Byte |= Pixel_Mask;
					if (i < 7) Display_Byte >>= 1; // Append each pixel the to most significant bit of the byte, and do not shift the last time or the initial bit would be lost

					// Put the next pixel bit to check to the most significant location
					Frame_Buffer_Chunk[i] <<= 1;
				}

				// The horizontal 8 pixels are ready to be displayed
				SPITransferByte(Display_Byte);
			}
		}
	}

	SPI_DESELECT_DISPLAY();
}
void DisplaySetTextCursor(unsigned char X, unsigned char Y)
{
	// Make sure the character will be entirely displayed on the screen
	if ((X >= DISPLAY_TEXT_MODE_WIDTH) || (Y >= DISPLAY_TEXT_MODE_HEIGHT)) return;

	Display_Text_Cursor_X = X;
	Display_Text_Cursor_Y = Y;
}

void DisplayWriteCharacter(void *Pointer_Buffer, unsigned char Character)
{
	unsigned short Index;
	unsigned char *Pointer_Buffer_Bytes = Pointer_Buffer;

	// Do nothing if the text cursor coordinates are out of the screen
	if ((Display_Text_Cursor_X >= DISPLAY_TEXT_MODE_WIDTH) || (Display_Text_Cursor_Y >= DISPLAY_TEXT_MODE_HEIGHT)) return;

	// Compute the pixel coordinates of the initial character byte in the frame buffer
	Index = (Display_Text_Cursor_Y * DISPLAY_COLUMNS_COUNT * DISPLAY_TEXT_CHARACTER_HEIGHT / 8) + (Display_Text_Cursor_X * DISPLAY_TEXT_CHARACTER_WIDTH);

	// Select a default sprite if this character has no sprite
	if ((Character < 32) || (Character > 127)) Character = 127;
	Character -= 32; // The characters sprites array does not contain the non-printable ASCII characters

	memcpy(Pointer_Buffer_Bytes + Index, Display_Font_Sprites[Character], DISPLAY_TEXT_CHARACTER_WIDTH);
}

void DisplayWriteString(void *Pointer_Buffer, const char *Pointer_String)
{
	unsigned char Character;

	while (*Pointer_String != 0)
	{
		Character = *Pointer_String;
		Pointer_String++;

		// Display only drawable characters
		if (Character >= 32)
		{
			DisplayWriteCharacter(Pointer_Buffer, Character);
			Display_Text_Cursor_X++;
		}

		// Should the cursor go to the next line ?
		if ((Display_Text_Cursor_X >= DISPLAY_TEXT_MODE_WIDTH) || (Character == '\n'))
		{
			Display_Text_Cursor_X = 0;
			Display_Text_Cursor_Y++;

			// Stop rendering the string if it crosses the display bounds
			if (Display_Text_Cursor_Y >= DISPLAY_TEXT_MODE_HEIGHT) return;
		}
	}
}

void DisplayDrawTextBuffer(void *Pointer_Buffer)
{
	unsigned char Row, Column, *Pointer_Buffer_Bytes = Pointer_Buffer;

	SPI_SELECT_DISPLAY();

	// Use the native display controller hardware order (1 byte represents 8 vertical pixels), so it is easy to adjust characters starting column
	for (Row = 0; Row < DISPLAY_ROWS_COUNT; Row += 8)
	{
		for (Column = 0; Column < DISPLAY_COLUMNS_COUNT; Column++)
		{
			SPITransferByte(*Pointer_Buffer_Bytes);
			Pointer_Buffer_Bytes++;
		}
	}

	SPI_DESELECT_DISPLAY();
}

void DisplayDrawTextMessage(void *Pointer_Buffer, const char *Pointer_String_Title, const char *Pointer_String_Message)
{
	unsigned char Title_X, Length;

	// Clear the frame buffer
	memset(Pointer_Buffer, 0, sizeof(Shared_Buffer_Display));

	// Center the title
	Length = (unsigned char) strlen(Pointer_String_Title);
	if (Length < DISPLAY_TEXT_MODE_WIDTH)
	{
		Title_X = (DISPLAY_TEXT_MODE_WIDTH - Length) / 2;
	}
	else
	{
		SERIAL_PORT_LOG(DISPLAY_IS_LOGGING_ENABLED, "The title string \"%s\" is too long to fit on a single display row.", Pointer_String_Title);
		Title_X = 0;
	}
	DisplaySetTextCursor(Title_X, 0);
	DisplayWriteString(Pointer_Buffer, Pointer_String_Title);

	// Render the message text
	DisplaySetTextCursor(0, 2);
	DisplayWriteString(Pointer_Buffer, Pointer_String_Message);

	DisplayDrawTextBuffer(Pointer_Buffer);
}

void DisplaySetBrightness(unsigned char Brightness)
{
	// The default brightness at display reset is 0x7F, so avoid higher values in case they could harm the OLED pixels
	if (Brightness > 0x7F)
	{
		SERIAL_PORT_LOG(DISPLAY_IS_LOGGING_ENABLED, "The specified brightness 0x%08X is too high, capping it to 0x7F.\n", Brightness);
		Brightness = 0x7F;
	}

	SPI_SELECT_DISPLAY();

	// Send the "Set Contrast Control" command
	DISPLAY_PIN_DC = DISPLAY_DC_MODE_COMMAND;
	SPITransferByte(0x81);
	SPITransferByte(Brightness);
	__delay_ms(4);
	DISPLAY_PIN_DC = DISPLAY_DC_MODE_DATA;

	SPI_DESELECT_DISPLAY();
}
