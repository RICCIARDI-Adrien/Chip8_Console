/** @file SPI.c
 * See SPI.h for description.
 * @author Adrien RICCIARDI
 */
#include <SPI.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void SPIInitialize(void)
{
	// Configure the baud rate
	SPI1CLK = 0; // Clock the SPI module from Fosc (64MHz)
	SPI1BAUD = 31; // SPIxBAUD = Fcsel / (2 * Fbaud) - 1, with Fcsel = HFINTOSC = 64MHz and Fbaud = 1MHz

	// Configure the module
	SPI1TWIDTH = 0; // Select 8-bit communications
	SPI1CON0 = 0x03; // Do not enable the SPI module yet as it is not fully configured, send data with the most significant bit first, configure master mode, set BMODE to 1 to be able to ignore the TX and RX counters feature (like previous generation PIC SPI modules)
	SPI1CON1 = 0x44; // Sample SDI input in the middle of the data output time to let the time to the signal to stabilize, select clock edge and phase 0 mode, slave select pin is active low, output data changes on transition from active to idle clock state, SDI and SDO pins are active high
	SPI1CON2 = 0x03; // The slave select pin drive is active when the transmit counter is greater than zero (the always active mode really activate the /SS pin continuously), select full duplex mode

	// Assign the pins to the SPI functions
	RC3PPS = 0x1E; // Select the RC3 pin as SPI SCLK
	SPI1SDIPPS = 0x14; // Select the RC4 pin as SPI SDI (MISO here)
	RC5PPS = 0x1F; // Select the RC5 pin as SPI SDO (MOSI here)*/
	SPISetTargetDevice(SPI_DEVICE_ID_NONE); // Configure the /SS pins, RC0 and RC1
	// Configure the pins as digital
	ANSELC &= 0xC4;
	// Configure the pins direction
	TRISC &= 0xD4; // The two /SS pins plus SCLK and SDO are outputs, SDI is input
	TRISCbits.TRISC4 = 1; // The SDI pin is input

	// Enable the SPI module
	SPI1CON0bits.EN = 1;
}

void SPISetTargetDevice(TSPIDeviceID Device_ID)
{
	if (Device_ID == SPI_DEVICE_ID_DISPLAY)
	{
		// Deselect the SD card
		LATCbits.LATC0 = 1;
		RC0PPS = 0; // Connect the latch function to the pin

		// Connect the SPI /SS function to the pin
		SPI1SSPPS = 0x11;
		RC1PPS = 0x20;
	}
	// The SD card /SS pin is manually controlled
	else
	{
		// Deselect the SD card and the display
		LATCbits.LATC0 = 1;
		LATCbits.LATC1 = 1;

		// Disconnect the /SS function from all pins
		RC0PPS = 0;
		RC1PPS = 0;
	}
}

unsigned char SPITransferByte(unsigned char Byte)
{
	// The /SS pin is active while SPI1TCNT is greater than 0
	SPI1TCNTH = 0;
	SPI1TCNTL = 1;

	// Transfer a byte
	SPI1TXB = Byte;
	while (!SPI1CON2bits.BUSY); // Wait for the busy bit to be set as this takes some time due to hardware synchronizers
	while (SPI1CON2bits.BUSY); // Wait for the transfer to terminate

	return SPI1RXB;
}
