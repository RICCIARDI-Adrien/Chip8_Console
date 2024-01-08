/** @file MBR.c
 * See MBR.h for description.
 * @author Adrien RICCIARDI
 */
#include <MBR.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** A partition table entry. */
typedef struct __attribute__((packed))
{
	unsigned char Boot_Flag;
	unsigned char First_Sector_CHS[3];
	unsigned char Type;
	unsigned char Last_Sector_CHS[3];
	unsigned long First_Sector_LBA;
	unsigned long Sectors_Count;
} TMBRPartitionEntry;

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void MBRParsePrimaryPartitions(void *Pointer_MBR_Sector, TMBRPartitionData Partitions_Data[MBR_PRIMARY_PARTITIONS_COUNT])
{
	TMBRPartitionEntry *Pointer_Entries;
	TMBRPartitionData *Pointer_Partitions_Data = Partitions_Data;
	unsigned char i;

	// Map a structure to the partition table to access relevant more easily
	Pointer_Entries = (TMBRPartitionEntry *) ((unsigned char *) Pointer_MBR_Sector + 446);

	// Parse all primary partitions
	for (i = 0; i < MBR_PRIMARY_PARTITIONS_COUNT; i++)
	{
		// Extract only the relevant information
		Pointer_Partitions_Data->Type = Pointer_Entries->Type;
		Pointer_Partitions_Data->Start_Sector = Pointer_Entries->First_Sector_LBA;
		Pointer_Partitions_Data->Sectors_Count = Pointer_Entries->Sectors_Count;

		Pointer_Entries++;
		Pointer_Partitions_Data++;
	}
}
