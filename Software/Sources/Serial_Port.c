/** @file Serial_Port.c
 * See Serial_Port.h for description.
 * @author Adrien RICCIARDI
 */
#include <Serial_Port.h>
#ifdef SERIAL_PORT_ENABLE_LOGGING
	#include <stdarg.h>
	#include <stdio.h>
#endif
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
	U1CON0 = 0xB0; // Select the high-speed baud rate generator, disable auto-baud detection, enable transmission, enable reception, select the asynchronous 8-bit UART mode without parity
	U1CON1 = 0x80; // Enable the serial port, disable the wake-up feature
	U1CON2 = 0; // Configure 1 stop bit, disable checksum, do not invert transmitted data, disable flow control

	// Configure the UART pins
	// Select the RC6 pin for the UART transmission
	RC6PPS = 0x13;
	// Select the RC7 pin for the UART reception
	U1RXPPS = 0x17;
	// Configure the pins as digital
	ANSELCbits.ANSELC6 = 0;
	ANSELCbits.ANSELC7 = 0;
	// Disable pin output driver (set them as input)
	TRISCbits.TRISC6 = 1;
	TRISCbits.TRISC7 = 1;

	// Display the following message only if the serial port logging feature is enabled
	SERIAL_PORT_LOG("Serial port logging is enabled.\r\n");
}

unsigned char SerialPortReadByte(void)
{
	// Wait for a byte to be received
	while (!PIR3bits.U1RXIF);

	// Retrieve the byte from the FIFO
	return U1RXB;
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

// Implement the XC8 C library putch() function to be able to directly use printf() in the code
void putch(char data)
{
	SerialPortWriteByte(data);
}
