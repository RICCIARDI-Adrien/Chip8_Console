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
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define SD_CARD_IS_LOGGING_ENABLED 0

/** A SD command size in bits. */
#define SD_CARD_COMMAND_SIZE 6

/** The command 0 bit pattern. */
#define SD_CARD_CMD0_GO_IDLE_STATE 0
/** The command 8 bit pattern. */
#define SD_CARD_CMD8_SEND_IF_COND 8
/** The command 17 bit pattern. */
#define SD_CARD_CMD17_READ_SINGLE_BLOCK 17
/** The command 58 bit pattern. */
#define SD_CARD_CMD58_READ_OCR 58
/** The command 55 bit pattern. */
#define SD_CARD_CMD55_APP_CMD 55
/** The application specific command 41 bit pattern. */
#define SD_CARD_ACMD41_SD_SEND_OP_COND 41

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Tell whether the SD card supports protocol version 1.x or protocol V2.00 and later. */
static unsigned char SD_Card_Is_Version_2_Protocol;

/** Tell whether the card has been removed since last check. */
static unsigned char SD_Card_Is_Card_Removed = 1; // On boot, tell that the card has been removed to force probing it

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Interrupt handler for the external interrupt 1. */
static void __interrupt(irq(INT1), high_priority) SDCardInterrupt(void)
{
	// Keep the record of the card removal state
	SD_Card_Is_Card_Removed = 1;

	// Clear the interrupt flag
	PIR5bits.INT1IF = 0;
}

/** Prepare the command at the SD format and transmit it on the SPI bus.
 * @param Command_Bit_Pattern The command number.
 * @param Argument The command argument, depending on the command.
 * @param Checksum The command checksum (should be needed only while the card is still in SD mode).
 */
void SDCardSendCommand(unsigned char Command_Bit_Pattern, unsigned long Argument, unsigned char Checksum)
{
	unsigned char Buffer[SD_CARD_COMMAND_SIZE + 1], i;

	// Create the command token
	Buffer[0] = 0xFF; // Add a dummy transfer at the beginning of the command to make sure the card is ready
	// Append the Start Bit and the Transmission Bit to the command code
	Command_Bit_Pattern &= 0x3F; // Only the 6 lower bits are meaningful
	Buffer[1] = 0x40 | Command_Bit_Pattern; // Configure the Start Bit and the Transmission Bit at the same time
	// Fill the argument
	Buffer[2] = (unsigned char) (Argument >> 24);
	Buffer[3] = (unsigned char) (Argument >> 16);
	Buffer[4] = (unsigned char) (Argument >> 8);
	Buffer[5] = (unsigned char) Argument;
	// Append the CRC and the End Bit
	Buffer[6] = Checksum | 0x01;

	// Send the command
	for (i = 0; i < sizeof(Buffer); i++) SPITransferByte(Buffer[i]);
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
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Received byte : 0x%02X.", Byte);
		if (!(Byte & 0x80)) return Byte; // The R1 response bit 7 is always 0 when the response is valid
	}

	return 0x80; // Tell that a timeout occurred
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void SDCardInitialize(void)
{
	// Configure the pin as digital
	ANSELBbits.ANSELB4 = 0;

	// Configure the pin as input
	TRISBbits.TRISB4 = 1;

	// Assign the external interrupt 1 feature to the RB4 pin
	INT1PPS = 0x0C; // Port B, pin 4

	// Configure the external interrupt 1 interrupt
	INTCON0bits.INT1EDG = 1; // Trigger the interrupt on signal rising edge (to detect card removal)
	PIR5bits.INT1IF = 0; // Make sure the interrupt flag is cleared to avoid triggering a false interrupt
	PIE5bits.INT1IE = 1; // Enable the interrupt
}

unsigned char SDCardProbe(void)
{
	unsigned char i, Buffer[4], Result;
	unsigned long Command_Argument;

	// Switch the SD card to SPI mode
	SPI_DESELECT_SD_CARD(); // Do not select the SD card (/SS must be kept high)
	__delay_ms(100); // Give some time to the SD card to run some internal firmware
	for (i = 0; i < 10; i++) SPITransferByte(0xFF); // The MOSI line must also be high while at least 74 clock cycles must be sent by the master

	// Send the CMD0 (GO_IDLE_STATE) command to execute a software reset of the card
	SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Sending CMD0 to the SD card...");
	SPI_SELECT_SD_CARD();
	Command_Argument = 0;
	SDCardSendCommand(SD_CARD_CMD0_GO_IDLE_STATE, Command_Argument, 0x94);
	Result = SDCardWaitForR1Response();
	SPI_DESELECT_SD_CARD();
	if (Result != 0x01)
	{
		if (Result == 0x80) SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : timeout during execution of CMD0.");
		else SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : initialization failed (R1 response : 0x%02X).", Result);
		return 1;
	}
	SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "CMD0 was successful.");
	__delay_ms(10);

	// Send the CMD8 (SEND_IF_COND) to determine whether the card is first generation or V2.00
	SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Sending CMD8 to the SD card...");
	SPI_SELECT_SD_CARD();
	Command_Argument = 0x01UL << 8; // Tell that the supplied voltage (VHS) is in range 2.7V-3.6V
	Command_Argument |= 0xAAUL; // Use the recommended check pattern
	SDCardSendCommand(SD_CARD_CMD8_SEND_IF_COND, Command_Argument, 0x86);
	Result = SDCardWaitForR1Response();
	if (Result == 0x01)
	{
		SD_Card_Is_Version_2_Protocol = 1;
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "The card supports version 2.00 or later of the specifications (R1 response : 0x%02X).", Result);
	}
	else if (Result == 0x05) // The CMD8 does not exist in protocol version 1.x, so the "illegal command" error is returned by the card
	{
		SD_Card_Is_Version_2_Protocol = 0;
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "The card supports version 1.x of the specifications (R1 response : 0x%02X).", Result);
	}
	else
	{
		SPI_DESELECT_SD_CARD();
		if (Result == 0x80) SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : timeout during execution of CMD8.");
		else SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : the card does not support version 1.x nor version 2.00 of the specifications (R1 response : 0x%02X).", Result);
		return 1;
	}
	// Retrieve the R7 reponse remaining bytes (only if the command was valid because the supported protocol version is recent enough)
	if (SD_Card_Is_Version_2_Protocol)
	{
		for (i = 0; i < 4; i++) Buffer[i] = SPITransferByte(0xFF);
		SPI_DESELECT_SD_CARD();
		// The command execution is successful if the voltage range is confirmed by the card and the check pattern is returned
		if (!(Buffer[2] & 0x01))
		{
			SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : the card did not confirm the voltage range.");
			return 1;
		}
		if (Buffer[3] != 0xAA)
		{
			SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : the check pattern returned by the card is wrong.");
			return 1;
		}
	}
	SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "CMD8 was successful.");
	__delay_ms(10);

	// Run the card initialization process until the card is ready
	SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Looping through the initialization process...");
	for (i = 0; i < 100; i++) // The minimum timeout specified by the specifications is 1s
	{
		// Tell the card that an application specific command will be sent
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Sending CMD55 to the SD card...");
		SPI_SELECT_SD_CARD();
		Command_Argument = 0;
		SDCardSendCommand(SD_CARD_CMD55_APP_CMD, Command_Argument, 0);
		Result = SDCardWaitForR1Response();
		SPI_DESELECT_SD_CARD();
		if (Result != 0x01)
		{
			if (Result == 0x80) SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : timeout during execution of CMD8.");
			else SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : R1 response : 0x%02X.", Result);
			return 1;
		}
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "CMD55 was successful.");
		__delay_ms(10);

		// Run the initialization process
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Sending ACMD41 to the SD card...");
		SPI_SELECT_SD_CARD();
		Command_Argument = 0xFFFFFFFF; // Set all reserved bits to 1 as asked by specification, set the bit 30 (HCS) to tell that high capacity cards are supported
		SDCardSendCommand(SD_CARD_ACMD41_SD_SEND_OP_COND, Command_Argument, 0);
		Result = SDCardWaitForR1Response();
		SPI_DESELECT_SD_CARD();
		if (Result & 0xFE) // Check for any error bit but the "idle" one
		{
			SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : R1 response : 0x%02X.", Result);
			return 1;
		}
		if (Result == 0) // The "idle" bit is cleared when the initialization process is completed
		{
			SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "ACMD41 was successful.");
			break;
		}
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Card is not ready yet (attempt %d).", i + 1);

		// Wait 10 milliseconds (taking into account the 1 millisecond of delay that has already been spent during the CMD55 execution)
		__delay_ms(9);
	}
	__delay_ms(10);

	// Reading the OCR register is part of the initialization process of the protocol V2.00 and later
	if (SD_Card_Is_Version_2_Protocol)
	{
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Sending CMD58 to the SD card...");
		SPI_SELECT_SD_CARD();
		Command_Argument = 0;
		SDCardSendCommand(SD_CARD_CMD58_READ_OCR, Command_Argument, 0);
		Result = SDCardWaitForR1Response();
		if (Result & 0xFE) // Check for any error (not taking into account the "idle" bit)
		{
			SPI_DESELECT_SD_CARD();
			if (Result == 0x80) SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : timeout during execution of CMD58.");
			else SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : R1 response : 0x%02X.", Result);
			return 1;
		}
		// The OCR register is provided on additional 4 bytes
		for (i = 0; i < 4; i++) Buffer[i] = SPITransferByte(0xFF);
		SPI_DESELECT_SD_CARD();
		// Display some card features for debugging purposes
		// Card power up status bit (bit 31)
		if (Buffer[0] & 0x80) SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "OCR card power up status bit is set, card is ready.");
		else
		{
			SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : OCR card power up status bit is cleared, card is busy.");
			return 1;
		}
		// Card Capacity Status (bit 30), this bit is valid only if the Card power up status bit is set
		if (Buffer[0] & 0x40) SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Card is High Capacity or Extended Capacity (CCS bit is set).");
		else SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Card is Standard Capacity.");
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "CMD58 was successful.");
		__delay_ms(10);
	}

	return 0;
}

unsigned char SDCardReadBlock(unsigned long Block_Address, unsigned char *Pointer_Buffer)
{
	const unsigned short START_TOKEN_TRANSFER_RETRIES_COUNT = 2000;  // Host is required to wait at least 100ms before declaring a timeout, as the SPI module is clocked at 125KHz, each 8-bit transfer takes (1 / 125KHz) * 8 = 64us, so we need at least 100000us (100ms) / 64us = 1563 transfers (then take some margin)
	unsigned char Result;
	unsigned short i;

	// Send the single block read command
	SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Sending CMD17 to the SD card (block address : %lu)...", Block_Address);
	SPI_SELECT_SD_CARD();
	SDCardSendCommand(SD_CARD_CMD17_READ_SINGLE_BLOCK, Block_Address, 0);
	Result = SDCardWaitForR1Response();
	if (Result & 0xFE) // Check any error without taking the "idle" bit into account
	{
		SPI_DESELECT_SD_CARD();
		if (Result == 0x80) SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : timeout during execution of CMD17.");
		else SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : R1 response : 0x%02X.", Result);
		return 1;
	}

	// Wait for the start token
	for (i = 0; i < START_TOKEN_TRANSFER_RETRIES_COUNT; i++)
	{
		Result = SPITransferByte(0xFF);
		if (Result == 0xFE) break;
	}
	if (i == START_TOKEN_TRANSFER_RETRIES_COUNT)
	{
		SPI_DESELECT_SD_CARD();
		SERIAL_PORT_LOG(SD_CARD_IS_LOGGING_ENABLED, "Error : timeout while waiting for the start token.");
		return 1;
	}

	// Retrieve the data block
	for (i = 0; i < SD_CARD_BLOCK_SIZE;  i++)
	{
		*Pointer_Buffer = SPITransferByte(0xFF);
		Pointer_Buffer++;
	}

	// Discard the 16-bit CRC
	SPITransferByte(0xFF);
	SPITransferByte(0xFF);

	SPI_DESELECT_SD_CARD();

	return 0;
}

TSDCardDetectionStatus SDCardGetDetectionStatus(void)
{
	unsigned char Is_Card_Detected;

	// Cache the detection state
	Is_Card_Detected = !PORTBbits.RB4;

	if (!Is_Card_Detected) return SD_CARD_DETECTION_STATUS_NO_CARD;
	if (SD_Card_Is_Card_Removed)
	{
		SD_Card_Is_Card_Removed = 0; // Reset the removal state at each call, but only if a card is inserted
		return SD_CARD_DETECTION_STATUS_DETECTED_REMOVED;
	}
	return SD_CARD_DETECTION_STATUS_DETECTED_NOT_REMOVED;
}
