/** @file SD_Card.c
 * See SD_Card.h for description.
 * @author Adrien RICCIARDI
 */
#include <SD_Card.h>
#include <Serial_Port.h>
#include <SPI.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private constants and macros
//-------------------------------------------------------------------------------------------------
/** A SD command size in bits. */
#define SD_CARD_COMMAND_SIZE 6

/** The command 0 bit pattern. */
#define SD_CARD_CMD0_GO_IDLE_STATE 0
/** The command 8 bit pattern. */
#define SD_CARD_CMD8_SEND_IF_COND 8
/** The command 17 bit pattern. */
#define SD_CARD_CMD17_READ_SINGLE_BLOCK 17

/** Select the SD card (/SS pin is logic low). */
#define SD_CARD_SPI_SLAVE_SELECT_ENABLE() LATCbits.LATC0 = 0
/** Unselect the SD card (/SS pin is logic low). */
#define SD_CARD_SPI_SLAVE_SELECT_DISABLE() LATCbits.LATC0 = 1

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Prepare the command at the SD format and transmit it on the SPI bus.
 * @param Command_Bit_Pattern The command number.
 * @param Argument The command argument, depending on the command.
 * @param Checksum The command checksum (should be needed only while the card is still in SD mode).
 */
void SDCardSendCommand(unsigned char Command_Bit_Pattern, unsigned long Argument, unsigned char Checksum)
{
	unsigned char Buffer[SD_CARD_COMMAND_SIZE], i;

	// Create the command token
	Command_Bit_Pattern &= 0x3F; // Only the 6 lower bits are meaningful
	Buffer[0] = 0x40 | Command_Bit_Pattern; // Configure the Start Bit and the Transmission Bit at the same time
	// Fill the argument
	Buffer[1] = (unsigned char) (Argument >> 24);
	Buffer[2] = (unsigned char) (Argument >> 16);
	Buffer[3] = (unsigned char) (Argument >> 8);
	Buffer[4] = (unsigned char) Argument;
	// Append the CRC and the End Bit
	Buffer[5] = Checksum | 0x01;

	// Send the command
	for (i = 0; i < SD_CARD_COMMAND_SIZE; i++) SPITransferByte(Buffer[i]);
}

/** Clock the SD card until a valid response is received. The response is using the R1 format, meaning that bit 7 must be zero.
 * @return The R1 response if the bit 7 is cleared,
 * @return 0x80 (i.e. the bit 7 set) if a timeout occurred.
 */
unsigned char SDCardWaitForR1Response(void)
{
	unsigned char i, Byte;

	// Wait for up to 16 cycles for a valid answer
	for (i = 0; i < 16; i++)
	{
		Byte = SPITransferByte(0xFF); // Clock the card while keeping the MOSI line high
		SERIAL_PORT_LOG("Received byte : 0x%02X.\r\n", Byte);
		if (!(Byte & 0x80)) return Byte; // The R1 response bit 7 is always 0 when the response is valid
	}

	return 0x80; // Tell that a timeout occurred
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void SDCardInitialize(void)
{
	unsigned char i, Buffer[16];
	unsigned long Command_Argument;

	// Switch the SD card to SPI mode
	SPISetTargetDevice(SPI_DEVICE_ID_SD_CARD);
	SD_CARD_SPI_SLAVE_SELECT_DISABLE(); // Do not select the SD card (/SS must be kept high)
	__delay_ms(1); // Give some time to the SD card to run some internal firmware
	for (i = 0; i < 10; i++) SPITransferByte(0xFF); // The MOSI line must also be high while at least 74 clock cycles must be sent by the master

	// Send the CMD0 (GO_IDLE_STATE) command to execute a software reset of the card
	SERIAL_PORT_LOG("Sending CMD0 to the SD card...\r\n");
	SD_CARD_SPI_SLAVE_SELECT_ENABLE();
	Command_Argument = 0;
	SDCardSendCommand(SD_CARD_CMD0_GO_IDLE_STATE, Command_Argument, 0x94);
	i = SDCardWaitForR1Response();
	SD_CARD_SPI_SLAVE_SELECT_DISABLE();
	if (i != 0x01)
	{
		if (i == 0x80) SERIAL_PORT_LOG("Error : timeout during execution of CMD0.\r\n");
		else SERIAL_PORT_LOG("Error : initialization failed (R1 response : 0x%02X).\r\n", i);
		return;
	}
	else SERIAL_PORT_LOG("CMD0 was successful.\r\n");
	__delay_ms(1);

	// Send the CMD8 (SEND_IF_COND) to determine whether the card is first generation or V2.00
	SERIAL_PORT_LOG("Sending CMD8 to the SD card...\r\n");
	SD_CARD_SPI_SLAVE_SELECT_ENABLE();
	Command_Argument = 0x01UL << 8; // Tell that the supplied voltage (VHS) is in range 2.7V-3.6V
	Command_Argument |= 0xAAUL; // Use the recommended check pattern
	SDCardSendCommand(SD_CARD_CMD8_SEND_IF_COND, Command_Argument, 0x86);
	i = SDCardWaitForR1Response();
	if (i != 0x01)
	{
		SD_CARD_SPI_SLAVE_SELECT_DISABLE();
		if (i == 0x80) SERIAL_PORT_LOG("Error : timeout during execution of CMD8.\r\n");
		else SERIAL_PORT_LOG("Error : the card does not support version 2.00 of the specifications (R1 response : 0x%02X).\r\n", i);
		return;
	}
	// Retrieve the R7 reponse remaining bytes
	for (i = 0; i < 4; i++) Buffer[i] = SPITransferByte(0xFF);
	SD_CARD_SPI_SLAVE_SELECT_DISABLE();
	// The command execution is successful if the voltage range is confirmed by the card and the check pattern is returned
	if (!(Buffer[2] & 0x01))
	{
		SERIAL_PORT_LOG("Error : the card did not confirm the voltage range.\r\n");
		return;
	}
	if (Buffer[3] != 0xAA)
	{
		SERIAL_PORT_LOG("Error : the check pattern return by the card is wrong.\r\n");
		return;
	}
	__delay_ms(1);
}
