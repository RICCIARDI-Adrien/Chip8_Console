/** @file SPI.h
 * Base driver for the SPI module.
 * @author Adrien RICCIARDI
 */
#ifndef H_SPI_H
#define H_SPI_H

#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Constants and macros
//-------------------------------------------------------------------------------------------------
#define SPI_SELECT_DISPLAY() LATCbits.LATC1 = 0
#define SPI_DESELECT_DISPLAY() LATCbits.LATC1 = 1

#define SPI_SELECT_SD_CARD() LATCbits.LATC0 = 0
#define SPI_DESELECT_SD_CARD() LATCbits.LATC0 = 1

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the SPI module to manage two slave devices. */
void SPIInitialize(void);

/** Send a byte to the selected target device and and simultaneously receive a byte from it.
 * @param Byte The data byte to send to the target device.
 * @return The data byte received from the target device.
 */
unsigned char SPITransferByte(unsigned char Byte);

#endif
