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
	U1BRGH = 0;
	U1BRGL = 138; // With this value we get 115107.9 bit/s, which is a 0.08% error

	// Configure the UART for asynchronous operation
	U1CON0 = 0xA0; // Select the high-speed baud rate generator, disable auto-baud detection, enable transmission, select the asynchronous 8-bit UART mode without parity
	U1CON1 = 0x80; // Enable the serial port, disable the wake-up feature
	U1CON2 = 0; // Configure 1 stop bit, disable checksum, do not invert transmitted data, disable flow control

	// Configure the UART pins
	// Select the RC6 pin for the UART transmission
	RC6PPS = 0x13;
	// Configure the pins as digital
	ANSELCbits.ANSELC6 = 0;
	ANSELCbits.ANSELC7 = 0;
	// Disable pin output driver (set them as input)
	TRISCbits.TRISC6 = 1;
	TRISCbits.TRISC7 = 1;
}

void SerialPortWriteByte(unsigned char Data)
{
	// Wait for the previous transmission to finish
	while (!PIR3bits.U1TXIF);
	
	// Send the byte
	U1TXB = Data;
}

void SerialPortWriteString(const char *Pointer_String)
{
	while (*Pointer_String != 0)
	{
		// Send the next character (do not call SerialPortWriteByte() to save some cycle and stack space)
		while (!PIR3bits.U1TXIF); // Wait for the previous transmission to finish
		U1TXB = *Pointer_String; // Send the character

		Pointer_String++;
	}
}
