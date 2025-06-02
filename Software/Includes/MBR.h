/** @file MBR.h
 * Parse the PC Master Boot Record.
 * @author Adrien RICCIARDI
 */
#ifndef H_MBR_H
#define H_MBR_H

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** The MBR size in bytes. */
#define MBR_SIZE 512

/** The number of primary partitions in the partition table. */
#define MBR_PRIMARY_PARTITIONS_COUNT 4

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** A parsed partition data. */
typedef struct
{
	unsigned char Type;
	unsigned long Start_Sector;
	unsigned long Sectors_Count;
} TMBRPartitionData;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Retrieve the valid primary partitions from the MBR partition table.
 * @param Pointer_MBR_Sector The raw MBR sector, as read from the medium. It must be of MBR_SIZE bytes.
 * @param Partitions_Data On output, fill an array of MBR_PRIMARY_PARTITIONS_COUNT partitions with the extracted partitions data.
 */
void MBRParsePrimaryPartitions(void *Pointer_MBR_Sector, TMBRPartitionData Partitions_Data[MBR_PRIMARY_PARTITIONS_COUNT]);

#endif
