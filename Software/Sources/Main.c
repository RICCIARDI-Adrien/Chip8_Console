/** @file Main.c
 * Entry point and main loop for the Chip-8 Console.
 * @author Adrien RICCIARDI
 */
#include <FAT.h>
#include <MBR.h>
#include <NCO.h>
#include <SD_Card.h>
#include <Serial_Port.h>
#include <Sound.h>
#include <SPI.h>
#include <stdio.h>
#include <string.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Microcontroller configuration
//-------------------------------------------------------------------------------------------------
// CONFIG1L register
#pragma config RSTOSC = HFINTOSC_64MHZ, FEXTOSC = OFF  // Use the internal oscillator at 64MHz on reset, disable external oscillator
// CONFIG1H register
#pragma config FCMEN = OFF, CSWEN = ON, PR1WAY = OFF, CLKOUTEN = OFF // Disable Fail-Safe Clock Monitor, allow writing to the OSCCON register to change the oscillator settings, allow the DMA priority bit PRLOCK to be written several times, disable the CLKOUT feature
// CONFIG2L register
#pragma config BOREN = SBORDIS, LPBOREN = OFF, IVT1WAY = OFF, MVECEN = ON, PWRTS = PWRT_64, MCLRE = EXTMCLR // Always enable the Brown-out Reset, disable Low Power Brown-out Reset, IVTLOCK bit can be cleared and set repeatedly, enable interrupts vector table, configure the Power-up timer to 64ms, enable /MCLR
// CONFIG2H register
#pragma config XINST = OFF, DEBUG = OFF, STVREN = ON, PPS1WAY = OFF, ZCD = OFF, BORV = VBOR_2P85 // Disable Extended Instruction Set (it is not yet supported by the compiler), disable the Background debugger, reset on stack overflow or underflow, PPSLOCK bit can be set and cleared repeatedly, disable the unused Zero-cross Detection module, set the Brown-out voltage to the highest available (2.85V)
// CONFIG3L register
#pragma config WDTE = OFF // Disable the Watchdog timer
// CONFIG3H register
#pragma config WDTCCS = SC, WDTCWS = WDTCWS_7 // Set default values (all '1') as the watchdog timer is not used
// CONFIG4L register
#pragma config WRTAPP = OFF, SAFEN = OFF, BBEN = OFF // Allow writing to the Application Block, disable the Storage Area Flash, disable the Boot Block
// CONFIG4H register
#pragma config LVP = ON, WRTSAF = OFF, WRTD = OFF, WRTC = ON, WRTB = OFF // Enable Low-Voltage Programming to avoid needing to apply a high voltage on the /MCLR pin during programming, Storage Area Flash is not write-protected, Data EEPROM is not write-protected, Configuration Registers are write-protected, Boot Block is not write-protected
// CONFIG5L
#pragma config CP = OFF // Disable program and data code protection

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** TODO */
unsigned char MainMountSDCard(void)
{
	static unsigned char Buffer[SD_CARD_BLOCK_SIZE];
	TMBRPartitionData Partitions_Data[MBR_PRIMARY_PARTITIONS_COUNT], *Pointer_Partitions_Data;
	unsigned char i;

	// The first SD card block contains the MBR, get it
	if (SDCardReadBlock(0, Buffer) != 0)
	{
		SERIAL_PORT_LOG("Failed to read SD card block.\r\n");
		return 1;
	}

	// Find the first valid primary partitions (do not care about the partition type, as it does not reflect the real file system the partition is formatted with)
	MBRParsePrimaryPartitions(Buffer, Partitions_Data);
	for (i = 0; i < MBR_PRIMARY_PARTITIONS_COUNT; i++)
	{
		// Cache the partition data access
		Pointer_Partitions_Data = &Partitions_Data[i];
		SERIAL_PORT_LOG("Partition %d : type=0x%02X, start sector=%lu, sectors count=%lu.\r\n", i + 1, Pointer_Partitions_Data->Type, Pointer_Partitions_Data->Start_Sector, Pointer_Partitions_Data->Sectors_Count);

		// Bypass any empty partition
		if (Pointer_Partitions_Data->Type == 0)
		{
			SERIAL_PORT_LOG("Partition is empty, trying next one.\r\n");
			continue;
		}

		// Try to mount the file system as the partition is not empty
		if (FATMount(Pointer_Partitions_Data) != 0)
		{
			SERIAL_PORT_LOG("Failed to mount the partition %d.\r\n", i + 1);
			continue;
		}
		SERIAL_PORT_LOG("Partition %d was successfully mounted.\r\n", i + 1);
		break;
	}
	// Were all partitions invalid ?
	if (i == MBR_PRIMARY_PARTITIONS_COUNT)
	{
		SERIAL_PORT_LOG("No valid partition could be found.\r\n");
		return 2;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
void main(void)
{
	unsigned char a = '0';

	// Wait for the internal oscillator to stabilize
	while (!OSCSTATbits.HFOR);

	// TEST
	ANSELBbits.ANSELB0 = 0;
	LATBbits.LATB0 = 1;
	TRISBbits.TRISB0 = 0;

	// Initialize all needed modules
	SerialPortInitialize();
	NCOInitialize(); // This module must be initialized before the sound module
	SoundInitialize();
	SPIInitialize(); // The SPI module must be initialized before the SD card module
	if (SDCardInitialize() != 0)
	{
		SERIAL_PORT_LOG("\033[31mFailed to initialize the SD card.\033[0m\r\n");
		while (1); // TODO
	}

	// TODO
	if (MainMountSDCard() != 0)
	{
		SERIAL_PORT_LOG("\033[31mFailed to mount the SD card file system.\033[0m\r\n");
		while (1); // TODO
	}

	// TEST
	SerialPortWriteString("\033[33m#######################################\033[0m\r\n");

	while (1)
	{
		LATBbits.LATB0 = !LATBbits.LATB0;

		__delay_ms(1000);
	}
}
