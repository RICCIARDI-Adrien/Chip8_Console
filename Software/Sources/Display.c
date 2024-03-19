/** @file Display.c
 * See Display.h for description.
 * @author Adrien RICCIARDI
 */
#include <Display.h>
#include <SPI.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** The display controller reset pin. */
#define DISPLAY_PIN_RESET LATBbits.LATB1
/** The display controller D/C pin. */
#define DISPLAY_PIN_DC LATBbits.LATB2

/** The D/C pin value to send a command to the display controller. */
#define DISPLAY_DC_MODE_COMMAND 0
/** The D/C pin value to send a data byte to the display controller. */
#define DISPLAY_DC_MODE_DATA 1

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

	// Convert a chunk of 32 pixels (8x4 pixels) at a time, outputing 64 pixels (8x8 pixels)
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
