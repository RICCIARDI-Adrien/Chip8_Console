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
/** Assert the chip select line of the display. */
#define SPI_SELECT_DISPLAY() LATCbits.LATC1 = 0
/** De-assert the chip select line of the display. */
#define SPI_DESELECT_DISPLAY() \
	{ \
		/* End any ongoing transfer before deselecting the slave (note that only the display is using SPIWriteByte()) */ \
		while (SPI1CON2bits.BUSY); \
		LATCbits.LATC1 = 1; \
	}

/** Assert the chip select line of the SD card. */
#define SPI_SELECT_SD_CARD() LATCbits.LATC0 = 0
/** De-assert the chip select line of the SD card. */
#define SPI_DESELECT_SD_CARD() LATCbits.LATC0 = 1

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the SPI module to manage two slave devices. */
void SPIInitialize(void);

/** Send a byte to the selected target device and simultaneously receive a byte from it.
 * @param Byte The data byte to send to the target device.
 * @return The data byte received from the target device.
 */
unsigned char SPITransferByte(unsigned char Byte);

/** Send a byte to the selected target device, without loosing time while receiving the target device data.
 * @param Byte The data byte to send to the target device.
 * @note The function immediately returns if there is room into the SPI peripheral transmission FIFO.
 */
void SPIWriteByte(unsigned char Byte);

#endif
