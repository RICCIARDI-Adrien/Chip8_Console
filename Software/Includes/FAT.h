/** @file FAT.h
 * A simple Microsoft FAT file system driver. It is based on the Microsoft FAT specification from August 30 2005.
 * @author Adrien RICCIARDI
 */
#ifndef H_FAT_H
#define H_FAT_H

#include <MBR.h>

//-------------------------------------------------------------------------------------------------
// Constants
//-------------------------------------------------------------------------------------------------
/** A short file name size in characters. */
#define FAT_SHORT_FILE_NAME_SIZE 11

//-------------------------------------------------------------------------------------------------
// Types
//-------------------------------------------------------------------------------------------------
/** The relevant information about a file. */
typedef struct
{
	unsigned char String_Short_Name[FAT_SHORT_FILE_NAME_SIZE + 2]; // Add one byte for the '.' character and one more for the terminating zero
	unsigned char Is_Directory; //!< Set to 1 if this entry represents a directory, set to 0 for a file (do not care about other flags).
	unsigned long Size; //!< The file size in bytes.
	unsigned long First_Cluster_Number;
} TFATFileInformation;

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Parse the provided partition to find and mount a FAT file system (if any).
 * @param Pointer_Partition The partition to mount.
 * @param Pointer_Temporary_Buffer The buffer used internally to load sectors. It must have room for SD_CARD_BLOCK_SIZE bytes.
 * @return 0 if the file system was successfully mounted,
 * @return 1 if an error occurred.
 */
unsigned char FATMount(TMBRPartitionData *Pointer_Partition, void *Pointer_Temporary_Buffer);

/** Configure a file and directories listing operation.
 * @param Pointer_String_Absolute_Path The absolute path of the directory to list. Directories separator character is '/'.
 * @note You must call this function once before a directory listing to initialize the FATListNext() internal state machine.
 * @return 0 on success,
 * @return 1 if an error occurred.
 */
unsigned char FATListStart(char *Pointer_String_Absolute_Path);
/** Find the next file or directory present in the root directory provided to FATListStart(). Call this function repeatedly until it returns a result different from 0 to list all files and directories present in the root directory.
 * @param Pointer_File_Information On output, contain the found file information.
 * @return 0 when a valid file or directory has been found and more are present in the root directory,
 * @return 1 when a valid file or directory has been found and this the last entry in the root directory,
 * @return 2 if an error occurred.
 */
unsigned char FATListNext(TFATFileInformation *Pointer_File_Information);

/** Load a file from the file system and store it in memory.
 * @param Pointer_File_Information The file to load.
 * @param Pointer_Destination_Buffer On output, will contain the file content.
 * @param Bytes_Count How may bytes of the file to store in memory. TODO
 * @param Destination_Buffer_Size The size in bytes of the buffer in which to store the read data. It must be a multiple of the size of a sector (512 bytes).
 * @return 0 on success,
 * @return 1 if an error occurred,
 * @return 2 if the destination buffer size is not a multiple of a sector size.
 */
unsigned char FATReadFile(TFATFileInformation *Pointer_File_Information, void *Pointer_Destination_Buffer, unsigned long Destination_Buffer_Size);

#endif
