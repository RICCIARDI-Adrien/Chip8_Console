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

/** Allow to split a cluster access into its internal blocks (because a cluster size can easily excess the available RAM).
 * @note This structure is not meant for a direct usage.
 */
typedef struct
{
	unsigned long Sector_Address; //!< The sector LBA address into the FAT.
	unsigned char Remaining_Sectors_Count; //!< How many sectors into the cluster have not been processed yet.
} TFATClusterAccessInformation;

/** Keep the processing state of a file during a read operation. */
typedef struct
{
	unsigned long Current_Cluster_Number; //!< The number of the file cluster currently being processed.
	unsigned long Size_Clusters; //!< The file size converted in cluster units. This value is updated by the file reading operation.
	TFATClusterAccessInformation Current_Cluster_Information; //!< Track the processing state of the current cluster.
} TFATFileDescriptor;

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

/** Configure a file reading operation.
 * @param Pointer_File_Information Provide the details about the file to read.
 * @param Pointer_File_Descriptor On output, initialize the file descriptor to start reading the file from the beginning.
 */
void FATReadSectorsStart(TFATFileInformation *Pointer_File_Information, TFATFileDescriptor *Pointer_File_Descriptor);

/** Read the file content sequentially, with the granularity of one sector.
 * @param Pointer_File_Descriptor The file descriptor must have been initialized before the first call to this function with FATReadSectorsStart(). Then, the same file descriptor can be provided to this function with no change until the end of the file is reached.
 * @param Sectors_Count How many file sectors to read.
 * @param Pointer_Destination_Buffer On output, store the read data to this buffer.
 * @return 0 on success,
 * @return 1 if the file end has been reached,
 * @return 2 if an error occurred.
 */
unsigned char FATReadSectorsNext(TFATFileDescriptor *Pointer_File_Descriptor, unsigned char Sectors_Count, void *Pointer_Destination_Buffer);

#endif
