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
}
