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
	// Enable the peripheral module
	PMD5bits.SPI1MD = 0;

	// Configure the baud rate
	SPI1CLK = 0; // Clock the SPI module from Fosc (64MHz)
	SPI1BAUD = 15; // SPIxBAUD = Fcsel / (2 * Fbaud) - 1, with Fcsel = HFINTOSC = 64MHz and Fbaud = 2MHz

	// Configure the module
	SPI1TWIDTH = 0; // Select 8-bit communications
	SPI1CON0 = 0x03; // Do not enable the SPI module yet as it is not fully configured, send data with the most significant bit first, configure master mode, set BMODE to 1 to be able to ignore the TX and RX counters feature (like previous generation PIC SPI modules)
	SPI1CON1 = 0x44; // Sample SDI input in the middle of the data output time to let the time to the signal to stabilize, select clock edge and phase 0 mode, slave select pin is active low, output data changes on transition from active to idle clock state, SDI and SDO pins are active high
	SPI1CON2 = 0x03; // The slave select pin drive is active when the transmit counter is greater than zero (the always active mode really activate the /SS pin continuously), select full duplex mode

	// Assign the pins to the SPI functions
	RC3PPS = 0x1E; // Select the RC3 pin as SPI SCLK
	SPI1SDIPPS = 0x14; // Select the RC4 pin as SPI SDI (MISO here)
	RC5PPS = 0x1F; // Select the RC5 pin as SPI SDO (MOSI here)
	// Make sure no device is selected
	SPI_DESELECT_DISPLAY();
	SPI_DESELECT_SD_CARD();
	// Configure the pins as digital
	ANSELC &= 0xC4;
	// Configure the pins direction
	TRISC &= 0xD4; // The two /SS pins plus SCLK and SDO are outputs, SDI is input
	TRISCbits.TRISC4 = 1; // The SDI pin is input

	// Enable the SPI module
	SPI1CON0bits.EN = 1;
}

unsigned char SPITransferByte(unsigned char Byte)
{
	// Wait for the previous transfer to terminate (if any), in case SPIWriteByte() was used just before
	while (SPI1CON2bits.BUSY); // Wait for the transfer to terminate

	// The /SS pin is active while SPI1TCNT is greater than 0
	SPI1TCNTH = 0;
	SPI1TCNTL = 1;

	// Transfer a byte
	SPI1TXB = Byte;
	while (!SPI1CON2bits.BUSY); // Wait for the busy bit to be set as this takes some time due to hardware synchronizers
	while (SPI1CON2bits.BUSY); // Wait for the transfer to terminate

	return SPI1RXB;
}

void SPIWriteByte(unsigned char Byte)
{
	volatile unsigned char Received_Byte;

	// Wait for the previous transfer to terminate (if any)
	while (SPI1CON2bits.BUSY); // Wait for the transfer to terminate

	// Drain the reception FIFO even if it is empty (this would just set the RXRE bit), in order to not block the future transmissions
	Received_Byte = SPI1RXB;

	// Transfer a byte (no need to set the SPIxTCNLy registers, they are impacting only the reception that we are not using here)
	SPI1TXB = Byte;
}
