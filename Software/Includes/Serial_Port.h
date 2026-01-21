/** @file Serial_Port.h
 * Provide a full-duplex serial port using the EUSART module 1.
 * @author Adrien RICCIARDI
 */
#ifndef H_SERIAL_PORT_H
#define H_SERIAL_PORT_H

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Initialize the EUSART module for the required operations. */
void SerialPortInitialize(void);

/** Wait for a byte of data to be received from the serial port.
 * @return The received byte.
 */
unsigned char SerialPortReadByte(void);

/** Send a single byte of data through the serial port.
 * @param Data The byte to send.
 */
void SerialPortWriteByte(unsigned char Data);

/** Send an ASCIIZ string through the serial port.
 * @param Pointer_String The string to send.
 */
void SerialPortWriteString(const char *Pointer_String);

#endif
