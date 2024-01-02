/** @file SPI.h
 * Base driver for the SPI module.
 * @author Adrien RICCIARDI
 */
#ifndef H_SPI_H
#define H_SPI_H

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** All available SPI devices.
 * As there is only one hardware "chip select" pin, the target device must be selected before accessing it to multiplex the hardware /SS pin to the correct GPIO.
 */
typedef enum
{
	SPI_DEVICE_ID_SD_CARD,
	SPI_DEVICE_ID_DISPLAY
} TSPIDeviceID;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the SPI module to manage two slave devices. */
void SPIInitialize(void);

/** Select the target device to communicate with. This will connect the hardware /SS pin to the selected device /ENABLE pin.
 * @param Device_ID The ID of the device to communicate with.
 */
void SPISetTargetDevice(TSPIDeviceID Device_ID);

/** Send a byte to the selected target device and and simultaneously receive a byte from it.
 * @param Byte The data byte to send to the target device.
 * @return The data byte received from the target device.
 */
unsigned char SPITransferByte(unsigned char Byte);

/** TODO */
void SPITransferBuffer(unsigned char Device_ID, unsigned char *Pointer_Transmission_Buffer, unsigned char *Pointer_Reception_Buffer, unsigned char Bytes_Count);

#endif
