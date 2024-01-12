/** @file FAT.h
 * A simple Microsoft FAT file system driver. It is based on the Microsoft FAT specification from August 30 2005.
 * @author Adrien RICCIARDI
 */
#ifndef H_FAT_H
#define H_FAT_H

#include <MBR.h>

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Parse the provided partition to find and mount a FAT file system (if any).
 * @param Pointer_Partition The partition to mount.
 * @return 0 if the file system was successfully mounted,
 * @return 1 if an error occurred.
 */
unsigned char FATMount(TMBRPartitionData *Pointer_Partition);

#endif
