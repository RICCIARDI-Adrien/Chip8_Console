/** @file Serial_Port.h
 * Provide a full-duplex 115200 8N1 serial port using the EUSART module 1.
 * @author Adrien RICCIARDI
 */
#ifndef H_SERIAL_PORT_H
#define H_SERIAL_PORT_H

//-------------------------------------------------------------------------------------------------
// Constants and macros
//-------------------------------------------------------------------------------------------------
/** Print a printf() like message on the serial port preceded by the function name and the line.
 * @param Format A printf() like format string.
 * @note The internal buffer is limited to 256 bytes, do not write too long strings.
 */
#ifdef SERIAL_PORT_ENABLE_LOGGING
	#define SERIAL_PORT_LOG(Format, ...) SerialPortWriteLog("[%s:%d] " Format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
	#define SERIAL_PORT_LOG(Format, ...) do {} while (0)
#endif

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

/** Print a printf() like message on the serial port.
 * @param Format A printf() like format string.
 * @note The internal buffer is limited to 256 bytes, do not write too long strings.
 */
void SerialPortWriteLog(const char *Pointer_String_Format, ...);

#endif
