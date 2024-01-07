/** @file SD_Card.h
 * Read data from a SD card switched to SPI mode.
 * @author Adrien RICCIARDI
 */
#ifndef H_SD_CARD_H
#define H_SD_CARD_H

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Switch the SD card to SPI mode and configure it for operation.
 * @return 1 if no SD card was found or if the SD card was not successfully initialized,
 * @return 0 if the SD card is successfully initialized and ready for operations.
 */
unsigned char SDCardInitialize(void);

#endif
