/** @file SD_Card.h
 * Read data from a SD card switched to SPI mode.
 * @author Adrien RICCIARDI
 */
#ifndef H_SD_CARD_H
#define H_SD_CARD_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** A block size in bytes. */
#define SD_CARD_BLOCK_SIZE 512

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Switch the SD card to SPI mode and configure it for operation.
 * @return 1 if no SD card was found or if the SD card was not successfully initialized,
 * @return 0 if the SD card is successfully initialized and ready for operations.
 */
unsigned char SDCardInitialize(void);

/** Read a 512-byte block from the SD card.
 * @param Block_Address The logical block address.
 * @param Pointer_Buffer On output, contain the read data. Make sure the buffer has room for SD_CARD_BLOCK_SIZE bytes.
 * @return 1 if an error occurred,
 * @return 0 on success.
 */
unsigned char SDCardReadBlock(unsigned long Block_Address, unsigned char *Pointer_Buffer);

#endif
