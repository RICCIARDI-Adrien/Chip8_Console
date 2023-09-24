/** @file Serial_Port.c
 * See Serial_Port.h for description.
 * @author Adrien RICCIARDI
 */
#include <Serial_Port.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void SerialPortInitialize(void)
{
	// Configure baud rate generation, computed with following formula : SPBRGH:SPBRG = (Fosc / (4 * Baud_Rate)) - 1
	SPBRGH1 = 0;
	SPBRG1 = 138; // With this value we get 115107.9 bit/s, which is a 0.08% error

	// Configure UART for asynchronous operation
	TXSTA1 = 0x24; // Select 8-bit transmission, enable transmitter, select asynchronous mode, enable high speed baud rate generation (to allow the above formula to be used)
	RCSTA1 = 0x90; // Enable serial port, select 8-bit reception, enable receiver
	BAUDCON1 = 0x08; // Use default signal polarities, enable 16-bit baud rate generator, disable auto-baud detection

	// Configure the UART pins
	// Configure the pins as digital
	ANSELCbits.ANSC6 = 0;
	ANSELCbits.ANSC7 = 0;
	// Disable pin output driver (set them as input)
	TRISCbits.TRISC6 = 1;
	TRISCbits.TRISC7 = 1;
}

void SerialPortWriteByte(unsigned char Data)
{
	// Wait for the bus to be idle
	while (!PIR1bits.TX1IF);
	
	// Send the byte
	TXREG1 = Data;
}

void SerialPortWriteString(const char *Pointer_String)
{
	while (*Pointer_String != 0)
	{
		// Send the next character (do not call SerialPortWriteByte() to save some cycle and stack space)
		while (!PIR1bits.TX1IF); // Wait for the bus to be idle
		TXREG1 = *Pointer_String; // Send the character

		Pointer_String++;
	}
}
