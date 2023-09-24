/** @file Serial_Port.h
 * Provide a full-duplex 115200 8N1 serial port using the EUSART module 1.
 * @author Adrien RICCIARDI
 */
#ifndef H_SERIAL_PORT_H
#define H_SERIAL_PORT_H

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Initialize the EUSART module for the required operations. */
void SerialPortInitialize(void);

/** Send a single byte of data through the serial port.
 * @param Data The byte to send.
 */
void SerialPortWriteByte(unsigned char Data);

/** Send an ASCIIZ string through the serial port.
 * @param String The string to send.
 */
void SerialPortWriteString(const char *Pointer_String);

#endif
