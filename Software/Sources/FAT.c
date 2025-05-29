/** @file FAT.c
 * See FAT.h for description.
 * @author Adrien RICCIARDI
 */
#include <FAT.h>
#include <SD_Card.h>
#include <Serial_Port.h>
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define FAT_IS_LOGGING_ENABLED 0

/** How many FAT directories are stored in a sector. The FAT specification tells that a FAT directory can't cross a sector boundary. */
#define FAT_DIRECTORY_ENTRIES_PER_SECTOR (SD_CARD_BLOCK_SIZE / sizeof(TFATDirectory))

/** All FAT file attributes. */
#define FAT_FILE_ATTRIBUTE_READ_ONLY 0x01
#define FAT_FILE_ATTRIBUTE_HIDDEN 0x02
#define FAT_FILE_ATTRIBUTE_SYSTEM 0x04
#define FAT_FILE_ATTRIBUTE_VOLUME_ID 0x08
#define FAT_FILE_ATTRIBUTE_DIRECTORY 0x10
#define FAT_FILE_ATTRIBUTE_ARCHIVE 0x20
#define FAT_FILE_ATTRIBUTE_LONG_FILE_NAME (FAT_FILE_ATTRIBUTE_READ_ONLY | FAT_FILE_ATTRIBUTE_HIDDEN | FAT_FILE_ATTRIBUTE_SYSTEM | FAT_FILE_ATTRIBUTE_VOLUME_ID)

//-------------------------------------------------------------------------------------------------
// Private types
//-------------------------------------------------------------------------------------------------
/** The extended BPB for FAT12 and FAT16 volumes. */
typedef struct __attribute__((packed))
{
	unsigned char Drive_Number;
	unsigned char Reserved_1;
	unsigned char Extended_Boot_Signature;
	unsigned long Volume_Serial_Number;
	unsigned char String_Volume_Label[11];
	unsigned char String_File_System_Type[8];
	unsigned char Reserved_2[448];
} TFATExtendedBIOSParameterBlockFAT16;

/** The extended BPB for FAT32 volumes. */
typedef struct __attribute__((packed))
{
	unsigned long FAT_Sectors_Count;
	unsigned short Extended_Flags;
	unsigned short File_System_Version;
	unsigned long Root_Directory_First_Cluster;
	unsigned short File_System_Information;
	unsigned short Backup_Boot_Record_Sector;
	unsigned char Reserved_0[12];
	unsigned char Drive_Number;
	unsigned char Reserved_1;
	unsigned char Extended_Boot_Signature;
	unsigned long Volume_Serial_Number;
	unsigned char String_Volume_Label[11];
	unsigned char String_File_System_Type[8];
	unsigned char Reserved_2[420];
} TFATExtendedBIOSParameterBlockFAT32;

/** A FAT Boot Sector (also named BIOS Parameter Block). Handle only FAT32 for now. */
typedef struct __attribute__((packed))
{
	unsigned char Jump_Boot_Instructions[3];
	unsigned char String_OEM_Name[8];
	unsigned short Bytes_Per_Sector_Count;
	unsigned char Sectors_Per_Cluster_Count;
	unsigned short Reserved_Sectors_Count;
	unsigned char FATs_Count;
	unsigned short Root_Entries_Count; // Only for FAT12 and FAT16
	unsigned short Old_Total_Sectors_Count; // Only for FAT12 and FAT16
	unsigned char Media_Type;
	unsigned short Old_FAT_Sectors_Count; // Only for FAT12 and FAT16
	unsigned short Sectors_Per_Track_Count;
	unsigned short Heads_Count;
	unsigned long Hidden_Sectors_Count;
	unsigned long Total_Sectors_Count;
	union
	{
		TFATExtendedBIOSParameterBlockFAT16 FAT_16;
		TFATExtendedBIOSParameterBlockFAT32 FAT_32;
	} Extended_BPB;
	unsigned short Signature_Word;
} TFATBootSector;

/** Relevant FAT information. */
typedef struct
{
	unsigned char Cluster_Size_Sectors; // Maximum value told in the spec (BPB_SecPerClus) is 128, leading to 65536 for 512-byte sectors
	unsigned long Total_Sectors_Count;
	unsigned long First_FAT_Sector;
	unsigned long First_Cluster_Sector; //!< The LBA sector address of the first cluster of the volume.
	unsigned long First_Root_Directory_Sector;
	unsigned long First_Root_Directory_Cluster;
	unsigned long First_Data_Area_Sector;
} TFATInformation;

/** A FAT Directory Structure. */
typedef struct __attribute__((packed))
{
	unsigned char Buffer_Name[FAT_SHORT_FILE_NAME_SIZE];
	unsigned char Attributes;
	unsigned char NT_Reserved;
	unsigned char Creation_Time_Tenth;
	unsigned short Creation_Time;
	unsigned short Creation_Date;
	unsigned short Last_Access_Date;
	unsigned short First_Cluster_High;
	unsigned short Last_Modification_Time;
	unsigned short Last_Modification_Date;
	unsigned short First_Cluster_Low;
	unsigned long Size;
} TFATDirectory;

/** The files listing function internal state machine states. */
typedef enum
{
	FAT_LIST_FILE_STATE_CONFIGURE_CLUSTER_NUMBER,
	FAT_LIST_FILE_STATE_READ_CLUSTER_SECTOR,
	FAT_LIST_FILE_STATE_PARSE_DIRECTORY_ENTRIES,
} TFATListFileState;

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Keep the mounted file system relevant information. */
static TFATInformation FAT_Information;

/** The cluster reading function internal sector LBA address. */
static unsigned long FAT_Read_Cluster_Current_Sector_Address;
/** The cluster reading function internal count of remaining sectors to read. */
static unsigned char FAT_Read_Cluster_Remaining_Sectors_Count;

/** The files listing function internal state machine state. */
static TFATListFileState FAT_List_File_State;
/** The files listing function internal cluster number. */
static unsigned long FAT_List_File_Current_Cluster_Number;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Prepare the cluster reading function to read a specific cluster content.
 * @param Cluster_Number The number of the cluster to read.
 */
static void FATConfigureClusterReading(unsigned long Cluster_Number)
{
	FAT_Read_Cluster_Current_Sector_Address = FAT_Information.First_Cluster_Sector + ((Cluster_Number - 2) * FAT_Information.Cluster_Size_Sectors); // TODO validity check
	FAT_Read_Cluster_Remaining_Sectors_Count = FAT_Information.Cluster_Size_Sectors;
}

/** Allow to read a full cluster sector per sector to minimize the amount of needed RAM. Call this function repeatedly until it returns a result different from 0 to read all the sectors of a given cluster.
 * @param Pointer_Buffer On output, will contain the content of the next sector of the cluster to read.
 * @return 0 if the current cluster sector has been read successfully and the cluster contains more sectors to read,
 * @return 1 if the current cluster sector has been read successfully and it was the last sector of the cluster,
 * @return 2 if an error occurred.
 */
static unsigned char FATReadClusterAsSectors(void *Pointer_Buffer)
{
	// Stop reading when the full cluster has been read
	if (FAT_Read_Cluster_Remaining_Sectors_Count == 0) return 1;

	// Read the next cluster's sector from the SD card
	if (SDCardReadBlock(FAT_Read_Cluster_Current_Sector_Address, Pointer_Buffer) != 0)
	{
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Failed to read the sector %u of the corresponding cluster (sector LBA address is 0x%08lX).", FAT_Information.Cluster_Size_Sectors - FAT_Read_Cluster_Remaining_Sectors_Count, FAT_Read_Cluster_Current_Sector_Address);
		return 2;
	}

	// Prepare for next call
	FAT_Read_Cluster_Current_Sector_Address++;
	FAT_Read_Cluster_Remaining_Sectors_Count--;

	// Cluster reading is not finished yet
	return 0;
}

/** Convert a DOS 8.3 file format to a normal NULL-terminated C string.
 * @param Pointer_Buffer_Name The DOS 8.3 format name buffer.
 * @param Pointer_String On output, contain the name formatted as a human-readable string.
 */
static void FATConvertFileNameToString(unsigned char *Pointer_Buffer_Name, unsigned char *Pointer_String)
{
	unsigned char i, Character;

	// Copy the file name until the end of the name section is reached
	for (i = 0; i < 8; i++)
	{
		// Copy all characters but the padding spaces
		Character = *Pointer_Buffer_Name;
		if (Character != 0x20)
		{
			*Pointer_String = Character;
			Pointer_String++;
		}

		// Check next character
		Pointer_Buffer_Name++;
	}

	// Is there a valid file extension ?
	if (*Pointer_Buffer_Name != 0x20)
	{
		// Add a dot character
		*Pointer_String = '.';
		Pointer_String++;

		// Copy the file extension
		for (i = 0; i < 3; i++)
		{
			// Is this the first padding space ?
			Character = *Pointer_Buffer_Name;
			if (Character == 0x20) break;
			*Pointer_String = Character;

			// Check next character
			Pointer_Buffer_Name++;
			Pointer_String++;
		}
	}

	// Terminate the string
	*Pointer_String = 0;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
unsigned char FATMount(TMBRPartitionData *Pointer_Partition, void *Pointer_Temporary_Buffer)
{
	TFATBootSector *Pointer_Boot_Sector;

	// Retrieve the boot sector
	if (SDCardReadBlock(Pointer_Partition->Start_Sector, Pointer_Temporary_Buffer) != 0)
	{
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Failed to read the boot sector (sector LBA address is 0x%08lX).", Pointer_Partition->Start_Sector);
		return 1;
	}

	// Make sure this is a valid FAT partition (assume for now this is a FAT32 partition)
	Pointer_Boot_Sector = (TFATBootSector *) Pointer_Temporary_Buffer;
	if (Pointer_Boot_Sector->Signature_Word != 0xAA55)
	{
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Bad signature word, this is not a FAT partition.");
		return 1;
	}

	// Cache some relevant values
	FAT_Information.Cluster_Size_Sectors = Pointer_Boot_Sector->Sectors_Per_Cluster_Count;
	FAT_Information.First_FAT_Sector = Pointer_Partition->Start_Sector + Pointer_Boot_Sector->Reserved_Sectors_Count; // Start from the beginning of the partition
	FAT_Information.First_Cluster_Sector = FAT_Information.First_FAT_Sector + (Pointer_Boot_Sector->Extended_BPB.FAT_32.FAT_Sectors_Count * Pointer_Boot_Sector->FATs_Count); // The first cluster is located after the various copies of the FAT
	FAT_Information.First_Root_Directory_Sector = FAT_Information.First_Cluster_Sector + ((Pointer_Boot_Sector->Extended_BPB.FAT_32.Root_Directory_First_Cluster - 2) * FAT_Information.Cluster_Size_Sectors); // Subtract 2 because the clusters count start at 2
	FAT_Information.First_Root_Directory_Cluster = Pointer_Boot_Sector->Extended_BPB.FAT_32.Root_Directory_First_Cluster;

	// Display some file system information
	#ifdef SERIAL_PORT_ENABLE_LOGGING
	{
		char String_Temporary[12];

		// OEM name identifier
		memcpy(String_Temporary, Pointer_Boot_Sector->String_OEM_Name, sizeof(Pointer_Boot_Sector->String_OEM_Name));
		String_Temporary[sizeof(Pointer_Boot_Sector->String_OEM_Name)] = 0; // Make sure the string is terminated
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "OEM name identifier : \"%s\".", String_Temporary);
		// Bytes per sector count
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Bytes per sector count : %u.", Pointer_Boot_Sector->Bytes_Per_Sector_Count);
		// Sectors per cluster count
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Sectors per cluster count : %u.", Pointer_Boot_Sector->Sectors_Per_Cluster_Count);
		// FATs count
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "FATs count : %u.", Pointer_Boot_Sector->FATs_Count);
		// Old total sectors count
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Total sectors count (only for FAT12 and FAT16) : %u.", Pointer_Boot_Sector->Old_Total_Sectors_Count);
		// Media type
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Media type : 0x%02X.", Pointer_Boot_Sector->Media_Type);
		// Old FAT sectors count
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "FAT sectors count (only for FAT12 and FAT16) : %u.", Pointer_Boot_Sector->Old_FAT_Sectors_Count);
		// Sectors per track count
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Sectors per track count : %u.", Pointer_Boot_Sector->Sectors_Per_Track_Count);
		// Heads count
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Heads count : %u.", Pointer_Boot_Sector->Heads_Count);
		// Hidden sectors count
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Hidden sectors count : %lu.", Pointer_Boot_Sector->Hidden_Sectors_Count);
		// FAT sectors count
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Total sectors count (only for FAT32) : %lu.", Pointer_Boot_Sector->Total_Sectors_Count);
		// FAT32 specific fields
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Root directory first cluster (only for FAT32) : %lu.", Pointer_Boot_Sector->Extended_BPB.FAT_32.Root_Directory_First_Cluster);
		if (Pointer_Boot_Sector->Extended_BPB.FAT_32.Extended_Boot_Signature == 0x29) // When set to this value, this indicates that the following 3 fields are present
		{
			// Volume serial number
			SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Volume serial number : 0x%08lX.", Pointer_Boot_Sector->Extended_BPB.FAT_32.Volume_Serial_Number);
			// Volume label
			memcpy(String_Temporary, Pointer_Boot_Sector->Extended_BPB.FAT_32.String_Volume_Label, sizeof(Pointer_Boot_Sector->Extended_BPB.FAT_32.String_Volume_Label));
			String_Temporary[sizeof(Pointer_Boot_Sector->Extended_BPB.FAT_32.String_Volume_Label)] = 0; // Make sure the string is terminated
			SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Volume label : \"%s\".", String_Temporary);
			// File system type
			memcpy(String_Temporary, Pointer_Boot_Sector->Extended_BPB.FAT_32.String_File_System_Type, sizeof(Pointer_Boot_Sector->Extended_BPB.FAT_32.String_File_System_Type));
			String_Temporary[sizeof(Pointer_Boot_Sector->Extended_BPB.FAT_32.String_File_System_Type)] = 0; // Make sure the string is terminated
			SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "File system type : \"%s\".", String_Temporary);
		}

		// Cached values
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Cached cluster size in sectors : %u.", FAT_Information.Cluster_Size_Sectors);
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Cached first FAT sector : %lu.", FAT_Information.First_FAT_Sector);
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Cached first cluster sector : %lu.", FAT_Information.First_Cluster_Sector);
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Cached first root directory sector : %lu.", FAT_Information.First_Root_Directory_Sector);
		SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Cached first root directory cluster : %lu.", FAT_Information.First_Root_Directory_Cluster);
	}
	#endif

	return 0;
}

unsigned char FATListStart(char *Pointer_String_Absolute_Path)
{
	unsigned char i;

	// TODO split the path

	// TODO follow all root directory clusters up to the target directory

	// Reset the listing state machine
	FAT_List_File_Current_Cluster_Number = FAT_Information.First_Root_Directory_Cluster; // Always start from the root directory for now as directories support is not present
	FAT_List_File_State = FAT_LIST_FILE_STATE_CONFIGURE_CLUSTER_NUMBER;

	return 0;
}

unsigned char FATListNext(TFATFileInformation *Pointer_File_Information)
{
	static TFATDirectory FAT_Directories[FAT_DIRECTORY_ENTRIES_PER_SECTOR]; // This buffer has the size of a sector
	static unsigned char Current_Directory_Entry_Index_In_Sector;
	TFATDirectory *Pointer_FAT_Directory;
	unsigned char Result;

	// Parse all directory entries until a valid one is found or the last one is reached
	while (1)
	{
		switch (FAT_List_File_State)
		{
			case FAT_LIST_FILE_STATE_CONFIGURE_CLUSTER_NUMBER:
				SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Entering FAT_LIST_FILE_STATE_CONFIGURE_CLUSTER_NUMBER state.");
				FATConfigureClusterReading(FAT_List_File_Current_Cluster_Number);
				// When a cluster has been configured, its first sector must be read

			case FAT_LIST_FILE_STATE_READ_CLUSTER_SECTOR:
				SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Entering FAT_LIST_FILE_STATE_READ_CLUSTER_SECTOR state.");
				Result = FATReadClusterAsSectors(FAT_Directories);
				if (Result == 2) // Did an error occur ?
				{
					SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Error : could not read the SD card.");
					return 2;
				}
				if (Result == 1) // Is the cluster fully read ?
				{
					// TEST
					return 1; // Was this the last cluster in the chain ?

					// TODO search for the next directory cluster in the FAT
					FAT_List_File_State = FAT_LIST_FILE_STATE_CONFIGURE_CLUSTER_NUMBER;
				}
				// When a sector has been read, its directory entries must be parsed
				Current_Directory_Entry_Index_In_Sector = 0;
				FAT_List_File_State = FAT_LIST_FILE_STATE_PARSE_DIRECTORY_ENTRIES;

			case FAT_LIST_FILE_STATE_PARSE_DIRECTORY_ENTRIES:
				// Cache the directory entry access
				Pointer_FAT_Directory = &FAT_Directories[Current_Directory_Entry_Index_In_Sector];
				SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "Entering FAT_LIST_FILE_STATE_PARSE_DIRECTORY_ENTRIES state, directory entry %u : name=\"%s\".", Current_Directory_Entry_Index_In_Sector, Pointer_FAT_Directory->Buffer_Name);

				// Have all directory entries of this sector been read ?
				Current_Directory_Entry_Index_In_Sector++;
				if (Current_Directory_Entry_Index_In_Sector == FAT_DIRECTORY_ENTRIES_PER_SECTOR) FAT_List_File_State = FAT_LIST_FILE_STATE_READ_CLUSTER_SECTOR;

				// Is this a valid file or directory entry ?
				Result = Pointer_FAT_Directory->Attributes; // Recycle the Result variable to cache the value
				// Ignore the long file name entries
				if ((Result & FAT_FILE_ATTRIBUTE_LONG_FILE_NAME) == FAT_FILE_ATTRIBUTE_LONG_FILE_NAME)
				{
					SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "This is a long file name entry, trying the next directory entry.");
					continue;
				}
				// Ignore the volume name
				if (Result & FAT_FILE_ATTRIBUTE_VOLUME_ID)
				{
					SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "This is a volume ID entry, trying the next directory entry.");
					continue;
				}
				// Is the entry containing a deleted file ?
				Result = Pointer_FAT_Directory->Buffer_Name[0];
				if ((Result == 0) || (Result == 0xE5))
				{
					SERIAL_PORT_LOG(FAT_IS_LOGGING_ENABLED, "This is an empty file name entry, trying the next directory entry.");
					continue;
				}

				// Keep the useful entry pieces
				// Convert the file name to a normal string
				FATConvertFileNameToString(Pointer_FAT_Directory->Buffer_Name, Pointer_File_Information->String_Short_Name);
				// Is this a directory ?
				if (Pointer_FAT_Directory->Attributes & FAT_FILE_ATTRIBUTE_DIRECTORY) Pointer_File_Information->Is_Directory = 1;
				else Pointer_File_Information->Is_Directory = 0;
				// Retrieve file size
				Pointer_File_Information->Size = Pointer_FAT_Directory->Size;
				// Retrieve file first cluster
				Pointer_File_Information->First_Cluster_Number = ((unsigned long) Pointer_FAT_Directory->First_Cluster_High << 16) | Pointer_FAT_Directory->First_Cluster_Low;

				return 0;
		}
	}

	return 0;
}

unsigned char FATReadFile(TFATFileInformation *Pointer_File_Information, void *Pointer_Destination_Buffer, unsigned long Destination_Buffer_Size)
{
	unsigned long Cluster_Number;
	unsigned char *Pointer_Destination_Buffer_Bytes, Result;

	Cluster_Number = Pointer_File_Information->First_Cluster_Number;
	Pointer_Destination_Buffer_Bytes = Pointer_Destination_Buffer;

	// Read bytes until the destination buffer is filled
	while (Destination_Buffer_Size > 0)
	{
		// Read the next cluster content
		FATConfigureClusterReading(Cluster_Number);
		do
		{
			Result = FATReadClusterAsSectors(Pointer_Destination_Buffer_Bytes);
			if (Result == 2) return 1; // Did something bad happened ?

			// Prepare for next cluster sector read
			Pointer_Destination_Buffer_Bytes += SD_CARD_BLOCK_SIZE;
			Destination_Buffer_Size -= SD_CARD_BLOCK_SIZE;
		} while ((Result == 0) && (Destination_Buffer_Size > 0));

		// TODO call function to find the next cluster
		break; // TEST
	}

	return 0;
}
