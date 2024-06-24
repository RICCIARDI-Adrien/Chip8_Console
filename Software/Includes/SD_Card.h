/** @file SD_Card.h
 * Read data from a SD card switched to SPI mode.
 * @author Adrien RICCIARDI
 */
#ifndef H_SD_CARD_H
#define H_SD_CARD_H

#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** A block size in bytes. */
#define SD_CARD_BLOCK_SIZE 512

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** All existing card detection statuses. */
typedef enum
{
	SD_CARD_DETECTION_STATUS_NO_CARD,
	SD_CARD_DETECTION_STATUS_DETECTED_NOT_REMOVED,
	SD_CARD_DETECTION_STATUS_DETECTED_REMOVED
} TSDCardDetectionStatus;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the SD card detection pin mechanism. */
void SDCardInitialize(void);

/** Switch the SD card to SPI mode and configure it for operation.
 * @return 1 if no SD card was found or if the SD card was not successfully initialized,
 * @return 0 if the SD card is successfully initialized and ready for operations.
 */
unsigned char SDCardProbe(void);

/** Read a 512-byte block from the SD card.
 * @param Block_Address The logical block address.
 * @param Pointer_Buffer On output, contain the read data. Make sure the buffer has room for SD_CARD_BLOCK_SIZE bytes.
 * @return 1 if an error occurred,
 * @return 0 on success.
 */
unsigned char SDCardReadBlock(unsigned long Block_Address, unsigned char *Pointer_Buffer);

/** Tell whether a SD card is currently detected and if it has been removed then reinserted since last check.
 * @return SD_CARD_DETECTION_STATUS_DETECTED_REMOVED if the SD card is detected and has been removed since the last call to this function,
 * @return SD_CARD_DETECTION_STATUS_DETECTED_NOT_REMOVED if the SD card is detected and has not been removed since the last call to this function,
 * @return SD_CARD_DETECTION_STATUS_NO_CARD if no SD card is inserted.
 */
TSDCardDetectionStatus SDCardGetDetectionStatus(void);

#endif
