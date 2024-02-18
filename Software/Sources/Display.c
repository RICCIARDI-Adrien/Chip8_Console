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

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void DisplayInitialize(void)
{
	SPISetTargetDevice(SPI_DEVICE_ID_DISPLAY);

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
	DISPLAY_PIN_DC = 0;
	SPITransferByte(0x20);
	SPITransferByte(0x00);
	__delay_ms(10);

	// TEST
	{
		unsigned char Row, Column, Mask = 0x80;
		
		DISPLAY_PIN_DC = 1;
		for (Row = 0; Row < 8; Row++)
		{
			for (Column = 0; Column < 128; Column++)
			{
				SPITransferByte(Mask);
			}
			Mask >>= 1;
		}
	}

	// Turn the display on now that it is configured
	DISPLAY_PIN_DC = 0;
	SPITransferByte(0xAF);
	__delay_ms(10);
}
