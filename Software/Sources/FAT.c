/** @file FAT.c
 * See FAT.h for description.
 * @author Adrien RICCIARDI
 */
#include <FAT.h>
#include <SD_Card.h>
#include <Serial_Port.h>
#include <string.h>

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
	//unsigned short Cluster_Size_Bytes; // Maximum value told in the spec (BPB_SecPerClus) is 128, leading to 65536 for 512-byte sectors
	unsigned short Cluster_Size_Sectors; // Maximum value told in the spec (BPB_SecPerClus) is 128, leading to 65536 for 512-byte sectors
	unsigned long Total_Sectors_Count;
	unsigned long First_FAT_Sector;
	unsigned long First_Cluster_Sector; //!< The LBA sector address of the first cluster of the volume.
	unsigned long First_Root_Directory_Sector;
	unsigned long First_Data_Area_Sector;
} TFATInformation;

/** A FAT Directory Structure. */
typedef struct __attribute__((packed))
{
	unsigned char Buffer_Name[11];
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

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Keep the mounted file system relevant information. */
static TFATInformation FAT_Information;

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
unsigned char FATMount(TMBRPartitionData *Pointer_Partition)
{
	static unsigned char Buffer[SD_CARD_BLOCK_SIZE];
	TFATBootSector *Pointer_Boot_Sector;

	// Retrieve the boot sector
	if (SDCardReadBlock(Pointer_Partition->Start_Sector, Buffer) != 0)
	{
		SERIAL_PORT_LOG("Failed to read the boot sector (sector LBA address is 0x%08X).\r\n", Pointer_Partition->Start_Sector);
		return 1;
	}

	// Make sure this is a valid FAT partition (assume for now this is a FAT32 partition)
	Pointer_Boot_Sector = (TFATBootSector *) Buffer;
	if (Pointer_Boot_Sector->Signature_Word != 0xAA55)
	{
		SERIAL_PORT_LOG("Bad signature word, this is not a FAT partition.\r\n");
		return 1;
	}

	// Cache some relevant values
	FAT_Information.Cluster_Size_Sectors = Pointer_Boot_Sector->Sectors_Per_Cluster_Count;
	FAT_Information.First_FAT_Sector = Pointer_Partition->Start_Sector + Pointer_Boot_Sector->Reserved_Sectors_Count; // Start from the beginning of the partition
	FAT_Information.First_Cluster_Sector = FAT_Information.First_FAT_Sector + (Pointer_Boot_Sector->Extended_BPB.FAT_32.FAT_Sectors_Count * Pointer_Boot_Sector->FATs_Count); // The first cluster is located after the various copies of the FAT
	FAT_Information.First_Root_Directory_Sector = FAT_Information.First_Cluster_Sector + ((Pointer_Boot_Sector->Extended_BPB.FAT_32.Root_Directory_First_Cluster - 2) * FAT_Information.Cluster_Size_Sectors); // Subtract 2 because the clusters count start at 2

	// Display some file system information
	#ifdef SERIAL_PORT_ENABLE_LOGGING
	{
		char String_Temporary[12];

		// OEM name identifier
		memcpy(String_Temporary, Pointer_Boot_Sector->String_OEM_Name, sizeof(Pointer_Boot_Sector->String_OEM_Name));
		String_Temporary[sizeof(Pointer_Boot_Sector->String_OEM_Name)] = 0; // Make sure the string is terminated
		SERIAL_PORT_LOG("OEM name identifier : \"%s\".\r\n", String_Temporary);
		// Bytes per sector count
		SERIAL_PORT_LOG("Bytes per sector count : %u.\r\n", Pointer_Boot_Sector->Bytes_Per_Sector_Count);
		// Sectors per cluster count
		SERIAL_PORT_LOG("Sectors per cluster count : %u.\r\n", Pointer_Boot_Sector->Sectors_Per_Cluster_Count);
		// FATs count
		SERIAL_PORT_LOG("FATs count : %u.\r\n", Pointer_Boot_Sector->FATs_Count);
		// Old total sectors count
		SERIAL_PORT_LOG("Total sectors count (only for FAT12 and FAT16) : %u.\r\n", Pointer_Boot_Sector->Old_Total_Sectors_Count);
		// Media type
		SERIAL_PORT_LOG("Media type : 0x%02X.\r\n", Pointer_Boot_Sector->Media_Type);
		// Old FAT sectors count
		SERIAL_PORT_LOG("FAT sectors count (only for FAT12 and FAT16) : %u.\r\n", Pointer_Boot_Sector->Old_FAT_Sectors_Count);
		// Sectors per track count
		SERIAL_PORT_LOG("Sectors per track count : %u.\r\n", Pointer_Boot_Sector->Sectors_Per_Track_Count);
		// Heads count
		SERIAL_PORT_LOG("Heads count : %u.\r\n", Pointer_Boot_Sector->Heads_Count);
		// Hidden sectors count
		SERIAL_PORT_LOG("Hidden sectors count : %lu.\r\n", Pointer_Boot_Sector->Hidden_Sectors_Count);
		// FAT sectors count
		SERIAL_PORT_LOG("Total sectors count (only for FAT32) : %lu.\r\n", Pointer_Boot_Sector->Total_Sectors_Count);
		// FAT32 specific fields
		SERIAL_PORT_LOG("Root directory first cluster (only for FAT32) : %lu.\r\n", Pointer_Boot_Sector->Extended_BPB.FAT_32.Root_Directory_First_Cluster);
		if (Pointer_Boot_Sector->Extended_BPB.FAT_32.Extended_Boot_Signature == 0x29) // When set to this value, this indicates that the following 3 fields are present
		{
			// Volume serial number
			SERIAL_PORT_LOG("Volume serial number : 0x%08X.\r\n", Pointer_Boot_Sector->Extended_BPB.FAT_32.Volume_Serial_Number);
			// Volume label
			memcpy(String_Temporary, Pointer_Boot_Sector->Extended_BPB.FAT_32.String_Volume_Label, sizeof(Pointer_Boot_Sector->Extended_BPB.FAT_32.String_Volume_Label));
			String_Temporary[sizeof(Pointer_Boot_Sector->Extended_BPB.FAT_32.String_Volume_Label)] = 0; // Make sure the string is terminated
			SERIAL_PORT_LOG("Volume label : \"%s\".\r\n", String_Temporary);
			// File system type
			memcpy(String_Temporary, Pointer_Boot_Sector->Extended_BPB.FAT_32.String_File_System_Type, sizeof(Pointer_Boot_Sector->Extended_BPB.FAT_32.String_File_System_Type));
			String_Temporary[sizeof(Pointer_Boot_Sector->Extended_BPB.FAT_32.String_File_System_Type)] = 0; // Make sure the string is terminated
			SERIAL_PORT_LOG("File system type : \"%s\".\r\n", String_Temporary);
		}

		// Cached values
		SERIAL_PORT_LOG("Cached cluster size in sectors : %u.\r\n", FAT_Information.Cluster_Size_Sectors);
		SERIAL_PORT_LOG("Cached first FAT sector : %lu.\r\n", FAT_Information.First_FAT_Sector);
		SERIAL_PORT_LOG("Cached first cluster sector : %lu.\r\n", FAT_Information.First_Cluster_Sector);
		SERIAL_PORT_LOG("Cached first root directory sector : %lu.\r\n", FAT_Information.First_Root_Directory_Sector);
	}
	#endif

	return 0;
}
