/** @file Main.c
 * Entry point and main loop for the Chip-8 Console.
 * @author Adrien RICCIARDI
 */
#include <Battery.h>
#include <Display.h>
#include <EEPROM.h>
#include <FAT.h>
#include <INI_Parser.h>
#include <Interpreter.h>
#include <Keyboard.h>
#include <LED.h>
#include <MBR.h>
#include <NCO.h>
#include <SD_Card.h>
#include <Serial_Port.h>
#include <Shared_Buffer.h>
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
#pragma config XINST = OFF, DEBUG = OFF, STVREN = ON, PPS1WAY = OFF, ZCD = OFF, BORV = VBOR_2P7 // Disable Extended Instruction Set (it is not yet supported by the compiler), disable the Background debugger, reset on stack overflow or underflow, PPSLOCK bit can be set and cleared repeatedly, disable the unused Zero-cross Detection module, set the Brown-out voltage to the highest voltage at which the core can safely operate with a 64MHz clock
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
// Private constants
//-------------------------------------------------------------------------------------------------
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define MAIN_IS_LOGGING_ENABLED 1

/** The configuration file name on the SD card. */
#define MAIN_CONFIGURATION_FILE_NAME "CONFIG.INI"

/** How many measures to compute the average on. */
#define MAIN_BATTERY_SAMPLES_COUNT 8

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** The splash screen image displayed at console boot. */
const unsigned char Main_Splash_Screen[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFE, 0x00,
	0x01, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0x80,
	0x01, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0xFF, 0x80, 0x00, 0x01, 0xF0, 0x03, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0xFF, 0xC0, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0xFF, 0xE0, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0xF0, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x70, 0x00, 0x01, 0xF0, 0x03, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x70, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x70, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x7F, 0xFF, 0xC0, 0x78, 0x0F, 0x00, 0xF0, 0xFF, 0x80, 0xFF, 0xFF, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x7F, 0xFF, 0xC0, 0x78, 0x0F, 0x7F, 0xE0, 0xFF, 0x81, 0xF0, 0x03, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x7F, 0xFF, 0xC0, 0x78, 0x0F, 0x7F, 0xE0, 0xFF, 0x81, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x7F, 0x80, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x01, 0xF0, 0x03, 0xE0,
	0x01, 0xFF, 0xFF, 0xC0, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xFF, 0xFF, 0xE0, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xFF, 0xFF, 0xE0, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0x80,
	0x00, 0xFF, 0xFF, 0xC0, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFE, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xFF, 0xE1, 0x80, 0xC1, 0xFC, 0x0F, 0xFE, 0x18, 0x01, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xFF, 0xE1, 0xC0, 0xC3, 0xFE, 0x0F, 0xFE, 0x18, 0x01, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0xE0, 0xC3, 0x07, 0x0C, 0x06, 0x18, 0x01, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0xF0, 0xC3, 0x00, 0x0C, 0x06, 0x18, 0x01, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0xB8, 0xC3, 0xFE, 0x0C, 0x06, 0x18, 0x01, 0xBE, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0x9C, 0xC1, 0xFF, 0x0C, 0x06, 0x18, 0x01, 0xBE, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0x87, 0xC0, 0x03, 0x0C, 0x06, 0x18, 0x01, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0x83, 0xC3, 0x03, 0x0C, 0x06, 0x18, 0x01, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x7F, 0xF0, 0xFF, 0xE1, 0x81, 0xC3, 0xFF, 0x0F, 0xFE, 0x1F, 0xF1, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x7F, 0xF0, 0xFF, 0xE1, 0x80, 0xC1, 0xFE, 0x0F, 0xFE, 0x1F, 0xF1, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Program the EEPROM relevant locations with the default values. */
static void MainInitializeEEPROM()
{
	unsigned char i;

	// Do nothing if the EEPROM contains already data
	if (EEPROMReadByte(EEPROM_ADDRESS_IS_MEMORY_CONTENT_INITIALIZED) == 1)
	{
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "EEPROM content is already initialized, skipping initialization.");
		return;
	}
	SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "EEPROM is erased, starting the initialization process...");

	// Set the maximum sound level
	EEPROMWriteByte(EEPROM_ADDRESS_SOUND_LEVEL_PERCENTAGE, EEPROM_SOUND_LEVEL_PERCENTAGE_HIGH);

	// Set the maximum display brightness
	EEPROMWriteByte(EEPROM_ADDRESS_DISPLAY_BRIGHTNESS, EEPROM_DISPLAY_BRIGHTNESS_HIGH);

	// Clear all non-volatile storage registers of the Super-Chip 8 interpreter
	for (i = 0; i < INTERPRETER_FLAG_REGISTERS_COUNT; i++) EEPROMWriteByte(EEPROM_ADDRESS_INTERPRETER_FLAG_REGISTER_0 + i, 0);

	// Tell that the EEPROM is initialized
	EEPROMWriteByte(EEPROM_ADDRESS_IS_MEMORY_CONTENT_INITIALIZED, 1);
	SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "EEPROM content has been successfully initialized.");
}

/** Display the main menu with the battery charge that is automatically updated.
 * @return A keys mask with the allowed key pressed by the user.
 */
static TKeyboardKey MainDisplayMainMenu(void)
{
	unsigned char Battery_Charge_Samples[MAIN_BATTERY_SAMPLES_COUNT], Battery_Charge, Keys_Mask, i, Sample_Index = 0, Ticks_Counter_Samples = 0, Ticks_Counter_Show_Menu;
	unsigned short Mean;

	// Initialize all the samples with the current battery value
	for (i = 0; i < MAIN_BATTERY_SAMPLES_COUNT; i++) Battery_Charge_Samples[i] = BatteryGetCurrentChargePercentage();

	// Display the main menu until an allowed key is pressed
	Ticks_Counter_Show_Menu = 250; // Set the counter to its expiring value, so the menu is immediately displayed when the function is called
	do
	{
		// Increment the ticks counters (no need for a real time accuracy here)
		if (NCO_60HZ_TICK())
		{
			Ticks_Counter_Samples++;
			Ticks_Counter_Show_Menu++;
			NCO_CLEAR_TICK_INTERRUPT_FLAG();
		}

		// Each ~1s, sample a new battery value
		if (Ticks_Counter_Samples >= (1000 / 16))
		{
			Battery_Charge_Samples[Sample_Index] = BatteryGetCurrentChargePercentage();
			Sample_Index++;
			if (Sample_Index >= MAIN_BATTERY_SAMPLES_COUNT) Sample_Index = 0;
			Ticks_Counter_Samples = 0;
		}

		// Each ~4s, display the menu with an updated battery charge value
		if (Ticks_Counter_Show_Menu >= (4000 / 16))
		{
			Mean = 0;
			for (i = 0; i < MAIN_BATTERY_SAMPLES_COUNT; i++) Mean += Battery_Charge_Samples[i];
			Mean /= MAIN_BATTERY_SAMPLES_COUNT;

			snprintf(Shared_Buffers.String_Temporary, sizeof(Shared_Buffers.String_Temporary), "A. Games\nB. Settings\nC. Information\n\n\nBattery charge : %u%%", Mean);
			DisplayDrawTextMessage(Shared_Buffer_Display, "- Main menu -", Shared_Buffers.String_Temporary);
			Ticks_Counter_Show_Menu = 0;
		}

		Keys_Mask = KeyboardReadKeysMask();
	} while ((Keys_Mask & (KEYBOARD_KEY_A | KEYBOARD_KEY_B | KEYBOARD_KEY_C)) == 0);

	// Wait for all the keys to be released
	while (KeyboardReadKeysMask() != 0);

	return Keys_Mask;
}

/** Probe the SD card and mount the first FAT partition.
 * @return 0 if the SD card was not removed since the last time it was probed,
 * @return 1 if the SD card was removed since the last time it was probed.
 */
static unsigned char MainMountSDCard(void)
{
	static unsigned char Buffer[SD_CARD_BLOCK_SIZE];
	TMBRPartitionData Partitions_Data[MBR_PRIMARY_PARTITIONS_COUNT], *Pointer_Partitions_Data;
	unsigned char i, Is_Message_Displayed;
	TSDCardDetectionStatus Card_Detection_Status;

	// Wait for an SD card to be inserted
Detect_SD_Card:
	Is_Message_Displayed = 0;
	while (1)
	{
		Card_Detection_Status = SDCardGetDetectionStatus();
		if (Card_Detection_Status != SD_CARD_DETECTION_STATUS_NO_CARD) break;

		// Avoid redrawing the message at each loop
		if (!Is_Message_Displayed)
		{
			DisplayDrawTextMessage(Shared_Buffer_Display, "SD card", "Please insert a SD\ncard.");
			Is_Message_Displayed = 1;
		}
	}

	// The card has not changed and was already probed, nothing more to do
	if (Card_Detection_Status == SD_CARD_DETECTION_STATUS_DETECTED_NOT_REMOVED)
	{
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "The card has not changed and was already probed.");
		return 0;
	}

	// Probe the SD card
	if (SDCardProbe() != 0)
	{
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "\033[31mFailed to probe the SD card.\033[0m");
		DisplayDrawTextMessage(Shared_Buffer_Display, "SD card", "Failed to probe the\nSD card.\nInsert another SD\ncard and press Menu.");
		while (!KeyboardIsMenuKeyPressed());
		__delay_ms(1000); // Give some time to the SD card to wake up
		goto Detect_SD_Card;
	}

	// The first SD card block contains the MBR, get it
	if (SDCardReadBlock(0, Buffer) != 0)
	{
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Failed to read the SD card MBR block.");
		DisplayDrawTextMessage(Shared_Buffer_Display, "SD card", "Failed to read the SDcard MBR block.\nInsert another SD\ncard and press Menu.");
		while (!KeyboardIsMenuKeyPressed());
		__delay_ms(1000); // Give some time to the SD card to wake up
		goto Detect_SD_Card;
	}

	// Find the first valid primary partitions (do not care about the partition type, as it does not reflect the real file system the partition is formatted with)
	MBRParsePrimaryPartitions(Buffer, Partitions_Data);
	for (i = 0; i < MBR_PRIMARY_PARTITIONS_COUNT; i++)
	{
		// Cache the partition data access
		Pointer_Partitions_Data = &Partitions_Data[i];
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Partition %d : type=0x%02X, start sector=%lu, sectors count=%lu.", i + 1, Pointer_Partitions_Data->Type, Pointer_Partitions_Data->Start_Sector, Pointer_Partitions_Data->Sectors_Count);

		// Bypass any empty partition
		if (Pointer_Partitions_Data->Type == 0)
		{
			SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Partition is empty, trying next one.");
			continue;
		}

		// Try to mount the file system as the partition is not empty
		if (FATMount(Pointer_Partitions_Data) != 0)
		{
			SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Failed to mount the partition %d.", i + 1);
			continue;
		}
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Partition %d was successfully mounted.", i + 1);
		break;
	}
	// Were all partitions invalid ?
	if (i == MBR_PRIMARY_PARTITIONS_COUNT)
	{
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "No valid partition could be found.");
		DisplayDrawTextMessage(Shared_Buffer_Display, "SD card", "No valid partition\ncould be found.\nInsert another SD\ncard and press Menu.");
		while (!KeyboardIsMenuKeyPressed());
		goto Detect_SD_Card;
	}

	// The SD card has been removed since last time it was probed
	return 1;
}

/** Retrieve the games configuration file from the SD card.
 * @param Pointer_Size On output, contain the size of the configuration file in bytes.
 * @return 1 if an error occurred,
 * @return 0 when a valid configuration file has been found and loaded.
 */
static unsigned char MainLoadConfigurationFile(unsigned short *Pointer_Size)
{
	TFATFileInformation File_Information;
	unsigned long Size;

	SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Finding the configuration file on the SD card...");

	// Begin listing the files
	if (FATListStart("/") != 0)
	{
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "FATListStart() failed.");
		return 1;
	}

	// Search for the configuration file
	while (FATListNext(&File_Information) == 0)
	{
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "File found : name=\"%s\", is directory=%u, size=%lu, first cluster=%lu.",
			File_Information.String_Short_Name,
			File_Information.Is_Directory,
			File_Information.Size,
			File_Information.First_Cluster_Number);

		// We are searching for a file, discard any directory
		if (File_Information.Is_Directory)
		{
			SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "This is a directory, skipping it.");
			continue;
		}

		// Is this the searched file ?
		if (strcmp((char *) File_Information.String_Short_Name, MAIN_CONFIGURATION_FILE_NAME) == 0)
		{
			SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Found the configuration file, loading it.");

			// Load the file
			if (FATReadFile(&File_Information, Shared_Buffers.Configuration_File, sizeof(Shared_Buffers.Configuration_File)) != 0) // Keep room for the terminating bytes
			{
				SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Error : failed to load the configuration file.");
				DisplayDrawTextMessage(Shared_Buffer_Display, "SD card", "Failed to load the\nconfiguration file.\nReplace SD card and\npress Menu.");
				while (!KeyboardIsMenuKeyPressed());
				return 1;
			}

			// Terminate the INI buffer (see the INI parser documentation for information about the two terminating bytes)
			Size = File_Information.Size;
			if (Size > (sizeof(Shared_Buffers.Configuration_File) - 2)) Size = sizeof(Shared_Buffers.Configuration_File) - 2;
			Shared_Buffers.Configuration_File[Size] = INI_PARSER_END_CHARACTER;
			Shared_Buffers.Configuration_File[Size + 1] = INI_PARSER_END_CHARACTER;
			*Pointer_Size = (unsigned short) Size; // A 16-bit value is enough to store this size, as there are only 8KB of RAM in total

			SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "The configuration file was successfully loaded.");
			return 0;
		}
	}

	// No configuration file found, tell the user to provide an updated SD card
	DisplayDrawTextMessage(Shared_Buffer_Display, "SD card", "No configuration filefound. Replace the SDcard and press Menu.");
	while (!KeyboardIsMenuKeyPressed());
	return 1;
}

/** Propose each available game to the user and allow him to select one.
 * @param Configuration_File_Size The size of the configuration file in bytes.
 * @param Pointer_Last_Played_Game_Index On input, set the displayed game index (use the value 0 if no previous game has been played). On output, contain the index in the current games list of the last game selected by the player.
 * @return NULL if an error occurred or if the player wants to return to the main menu,
 * @return A string pointer on the INI section corresponding to the selected game.
 */
static char *MainSelectGame(unsigned short Configuration_File_Size, unsigned char *Pointer_Last_Played_Game_Index)
{
	char *Pointer_String_Section, *Pointer_String_Content, String_Line[DISPLAY_TEXT_MODE_WIDTH + 1];
	unsigned char Games_Count = 0, Current_Game_Index = 0, Keys_Mask, Is_Games_List_Incrementing = 1, Last_Played_Game_Index = *Pointer_Last_Played_Game_Index;

	// Count the available games
	SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Counting the available games...");
	Pointer_String_Section = Shared_Buffers.Configuration_File;
	while (1)
	{
		// Locate the next game section
		Pointer_String_Section = INIParserFindNextSection(Pointer_String_Section);
		if (Pointer_String_Section == NULL) break;
		Games_Count++;
	}
	SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Found games count : %u.", Games_Count);

	// Do not continue if no game is available
	if (Games_Count == 0)
	{
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "No game found, stopping.");
		DisplayDrawTextMessage(Shared_Buffer_Display, "SD card", "No game found in the\nconfiguration file.\nReplace the SD card\nand press Menu.");
		while (!KeyboardIsMenuKeyPressed());
		return NULL;
	}

	// Display the last played game
	SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Searching for the last played game index (%u).", Last_Played_Game_Index);
	if (Last_Played_Game_Index > Games_Count) // Make sure the provided value is not bad
	{
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Error : the last played game index (%u) is greater than the games count (%u).", Last_Played_Game_Index, Games_Count);
		return NULL;
	}
	// Go through the games list until the index before the last played game is found
	else
	{
		if (Last_Played_Game_Index > 0) Last_Played_Game_Index--; // Stop just before the game, as the displaying game loop will parse it
		Pointer_String_Section = Shared_Buffers.Configuration_File;
		while (Current_Game_Index < Last_Played_Game_Index)
		{
			Pointer_String_Section = INIParserFindNextSection(Pointer_String_Section);
			if (Pointer_String_Section == NULL)
			{
				SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Error : the last game has been reached but the last played game index was not found, this issue should not occur (current game index = %u, last played game index = %u).", Current_Game_Index, Last_Played_Game_Index);
				return NULL;
			}
			else Current_Game_Index++;
		}
	}

	// Display the games selection menu
	while (1)
	{
		// Go to the next game
		if (Is_Games_List_Incrementing)
		{
			Pointer_String_Section = INIParserFindNextSection(Pointer_String_Section);
			if (Pointer_String_Section == NULL)
			{
				SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Last game reached, looping to the first one.");
				Current_Game_Index = 1;
				Pointer_String_Section = Shared_Buffers.Configuration_File;
				Pointer_String_Section = INIParserFindNextSection(Pointer_String_Section); // Make sure to point to the second section next time
			}
			else Current_Game_Index++;
		}
		// Go to the previous game
		else
		{
			Pointer_String_Section = INIParserFindPreviousSection(Shared_Buffers.Configuration_File, Pointer_String_Section);
			if (Pointer_String_Section == NULL)
			{
				SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "First game reached, looping to the last one.");
				Current_Game_Index = Games_Count;
				Pointer_String_Section = (char *) (Shared_Buffers.Configuration_File + Configuration_File_Size);
				Pointer_String_Section = INIParserFindPreviousSection(Shared_Buffers.Configuration_File, Pointer_String_Section); // Make sure to point to the penultimate section next time
			}
			else Current_Game_Index--;
		}
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Current game index : %u.", Current_Game_Index);

		// Show the game into the display frame buffer
		memset(Shared_Buffer_Display, 0, sizeof(Shared_Buffer_Display));
		sprintf(String_Line, "- Game %u/%u -", Current_Game_Index, Games_Count);
		DisplaySetTextCursor((DISPLAY_TEXT_MODE_WIDTH - (unsigned char) strlen(String_Line)) / 2, 0); // Center the text
		DisplayWriteString(Shared_Buffer_Display, String_Line);

		// Retrieve the game information
		// Title
		Pointer_String_Content = INIParserReadString(Pointer_String_Section, "Title");
		if (Pointer_String_Content == NULL)
		{
			SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Warning : no game title found.");
			Pointer_String_Content = "! NO TITLE PROVIDED !";
		}
		SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Game title : \"%s\".", Pointer_String_Content);
		DisplaySetTextCursor(0, 2);
		DisplayWriteString(Shared_Buffer_Display, Pointer_String_Content);
		// Description
		Pointer_String_Content = INIParserReadString(Pointer_String_Section, "Description");
		if (Pointer_String_Content == NULL) SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Warning : no game description found.");
		// Do not display an error message if no description is provided, just display nothing
		else
		{
			DisplaySetTextCursor(0, 3);
			DisplayWriteString(Shared_Buffer_Display, Pointer_String_Content);
		}

		// Display instructions
		DisplaySetTextCursor(0, DISPLAY_TEXT_MODE_HEIGHT - 1);
		DisplayWriteString(Shared_Buffer_Display, "C : start, D : back.");
		DisplayDrawTextBuffer(Shared_Buffer_Display);

		// Wait for a key press
		while (1)
		{
			Keys_Mask = KeyboardReadKeysMask();

			// Show the previous game
			if (Keys_Mask & KEYBOARD_KEY_LEFT)
			{
				Is_Games_List_Incrementing = 0;
				while (KeyboardReadKeysMask() & KEYBOARD_KEY_LEFT); // Wait for key release
				break;
			}
			// Show the next game
			if (Keys_Mask & KEYBOARD_KEY_RIGHT)
			{
				Is_Games_List_Incrementing = 1;
				while (KeyboardReadKeysMask() & KEYBOARD_KEY_RIGHT); // Wait for key release
				break;
			}
			// Select the current game
			if (Keys_Mask & KEYBOARD_KEY_C)
			{
				SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Selected game %u.", Current_Game_Index);
				*Pointer_Last_Played_Game_Index = Current_Game_Index;
				while (KeyboardReadKeysMask() & KEYBOARD_KEY_C); // Wait for key release
				return Pointer_String_Section; // The actual data is stored in the shared buffer, so a pointer to such data can be returned safely
			}
			// Return to main menu
			if (Keys_Mask & KEYBOARD_KEY_D)
			{
				while (KeyboardReadKeysMask() & KEYBOARD_KEY_D); // Wait for key release
				return NULL;
			}
		}
	}
}

/** Display the settings menu and interact with the user. */
static void MainDisplaySettingsMenu(void)
{
	const char *Pointer_String_Sound, *Pointer_String_Brightness;
	unsigned char Sound_Level_Percentage, Brightness;
	TKeyboardKey Keys_Mask;

	while (1)
	{
		// Retrieve the settings value
		// Sound level
		Sound_Level_Percentage = EEPROMReadByte(EEPROM_ADDRESS_SOUND_LEVEL_PERCENTAGE);
		if (Sound_Level_Percentage == EEPROM_SOUND_LEVEL_PERCENTAGE_OFF) Pointer_String_Sound = "disabled";
		else if (Sound_Level_Percentage == EEPROM_SOUND_LEVEL_PERCENTAGE_LOW) Pointer_String_Sound = "low";
		else if (Sound_Level_Percentage == EEPROM_SOUND_LEVEL_PERCENTAGE_MEDIUM) Pointer_String_Sound = "medium";
		else Pointer_String_Sound = "high";
		// Display brightness
		Brightness = EEPROMReadByte(EEPROM_ADDRESS_DISPLAY_BRIGHTNESS);
		if (Brightness == EEPROM_DISPLAY_BRIGHTNESS_LOW) Pointer_String_Brightness = "25%\n";
		else if (Brightness == EEPROM_DISPLAY_BRIGHTNESS_MEDIUM) Pointer_String_Brightness = "50%\n";
		else Pointer_String_Brightness = "100%";

		// Display the menu content
		snprintf(Shared_Buffers.String_Temporary, sizeof(Shared_Buffers.String_Temporary), "Sound (A) : %s\nBrightness (B) : %s\n\n\nMenu : back.", Pointer_String_Sound, Pointer_String_Brightness);
		DisplayDrawTextMessage(Shared_Buffer_Display, "- Settings -", Shared_Buffers.String_Temporary);

		// Wait for a key to be pressed
		Keys_Mask = KeyboardWaitForKeys(KEYBOARD_KEY_A | KEYBOARD_KEY_B | KEYBOARD_KEY_MENU);
		if (Keys_Mask & KEYBOARD_KEY_A)
		{
			if (Sound_Level_Percentage == EEPROM_SOUND_LEVEL_PERCENTAGE_OFF) Sound_Level_Percentage = EEPROM_SOUND_LEVEL_PERCENTAGE_LOW;
			else if (Sound_Level_Percentage == EEPROM_SOUND_LEVEL_PERCENTAGE_LOW) Sound_Level_Percentage = EEPROM_SOUND_LEVEL_PERCENTAGE_MEDIUM;
			else if (Sound_Level_Percentage == EEPROM_SOUND_LEVEL_PERCENTAGE_MEDIUM) Sound_Level_Percentage = EEPROM_SOUND_LEVEL_PERCENTAGE_HIGH;
			else Sound_Level_Percentage = EEPROM_SOUND_LEVEL_PERCENTAGE_OFF;
			SoundSetLevel(Sound_Level_Percentage);
			EEPROMWriteByte(EEPROM_ADDRESS_SOUND_LEVEL_PERCENTAGE, Sound_Level_Percentage);
		}
		else if (Keys_Mask & KEYBOARD_KEY_B)
		{
			if (Brightness == EEPROM_DISPLAY_BRIGHTNESS_LOW) Brightness = EEPROM_DISPLAY_BRIGHTNESS_MEDIUM;
			else if (Brightness == EEPROM_DISPLAY_BRIGHTNESS_MEDIUM) Brightness = EEPROM_DISPLAY_BRIGHTNESS_HIGH;
			else Brightness = EEPROM_DISPLAY_BRIGHTNESS_LOW;
			DisplaySetBrightness(Brightness);
			EEPROMWriteByte(EEPROM_ADDRESS_DISPLAY_BRIGHTNESS, Brightness);
		}
		else if (Keys_Mask & KEYBOARD_KEY_MENU) break;
	}
}

#if MAIN_IS_LOGGING_ENABLED
	/** Display the various reasons that could lead to a system reset. */
	static void MainCheckResetReason(void)
	{
		if (PCON0bits.STKOVF) SERIAL_PORT_LOG(1, "Detected reset reason : stack overflow.");
		if (PCON0bits.STKUNF) SERIAL_PORT_LOG(1, "Detected reset reason : stack underflow.");
		if (!PCON0bits.WDTWV) SERIAL_PORT_LOG(1, "Detected reset reason : watchdog window violation.");
		if (!PCON0bits.RWDT) SERIAL_PORT_LOG(1, "Detected reset reason : watchdog timer triggered.");
		if (!PCON0bits.RMCLR) SERIAL_PORT_LOG(1, "Detected reset reason : /MCLR external reset.");
		if (!PCON0bits.RI) SERIAL_PORT_LOG(1, "Detected reset reason : RESET instruction executed.");
		if (!PCON0bits.POR) SERIAL_PORT_LOG(1, "Detected reset reason : power-on reset.");
		if (!PCON0bits.BOR) SERIAL_PORT_LOG(1, "Detected reset reason : brown-out reset.");
		if (!PCON1bits.MEMV) SERIAL_PORT_LOG(1, "Detected reset reason : memory violation.");

		// Clear the registers to have a fresh status on next reset
		PCON0 = 0x3F;
		PCON1 = 0x02;
	}
#endif

//-------------------------------------------------------------------------------------------------
// Entry point
//-------------------------------------------------------------------------------------------------
void main(void)
{
	char *Pointer_String_Game_INI_Section;
	unsigned char Is_SD_Card_Removed, Last_Played_Game_Index = 0;
	unsigned short Configuration_File_Size;
	TKeyboardKey Keys_Mask;

	// Wait for the internal oscillator to stabilize
	while (!OSCSTATbits.HFOR);

	// Initialize all needed modules
	LEDInitialize();
	LED_SET_ENABLED(1); // Turn the LED on during the microcontroller boot
	SerialPortInitialize();
	#if MAIN_IS_LOGGING_ENABLED
		MainCheckResetReason(); // Right after the serial port is working to display the results, determine if there was an abnormal reset
	#endif
	MainInitializeEEPROM(); // Initialize the EEPROM content if the microcontroller EEPROM area is not yet programmed
	NCOInitialize(); // This module must be initialized before the sound module
	SoundInitialize();
	KeyboardInitialize();
	InterpreterInitialize();
	SDCardInitialize();
	SPIInitialize(); // The SPI module must be initialized before the display
	DisplayInitialize();
	SDCardInitialize();
	BatteryInitialize();

	// Show the splash screen
	memcpy(Shared_Buffer_Display, Main_Splash_Screen, sizeof(Shared_Buffer_Display));
	DisplayDrawFullSizeBuffer(Shared_Buffer_Display);
	__delay_ms(2000);

	// Initialize the interrupts
	// Set the vector table base address to the default value
	IVTBASEU = 0;
	IVTBASEH = 0;
	IVTBASEL = 0x08;
	// Enable interrupts
	INTCON0bits.IPEN = 0; // Disable priority, all interrupts are high-priority and use the hardware order
	INTCON0bits.GIE = 1; // Enable all interrupts

	// The boot was completed, turn the LED off to same some power
	LED_SET_ENABLED(0);

	while (1)
	{
		// Main menu
		Keys_Mask = MainDisplayMainMenu();

		// Games
		if (Keys_Mask & KEYBOARD_KEY_A)
		{
			// Load the games configuration from the SD card
			while (1)
			{
				DisplayDrawTextMessage(Shared_Buffer_Display, "- Games -", "Loading console\nconfiguration...");

				// Block until an SD card with a valid FAT file system is inserted
				Is_SD_Card_Removed = MainMountSDCard();

				// Block until a valid configuration file is found
				if (MainLoadConfigurationFile(&Configuration_File_Size) != 0) continue;

				// Block until a game is found
				if (Is_SD_Card_Removed) Last_Played_Game_Index = 0; // Restart displaying the game selection menu from the first one if the card has been changed, as we do not know its content
				Pointer_String_Game_INI_Section = MainSelectGame(Configuration_File_Size, &Last_Played_Game_Index);
				if (Pointer_String_Game_INI_Section == NULL) break; // Return to main menu if the player wanted chose to, or if an error occurred

				// Try to load the game from the SD card
				if (InterpreterLoadProgramFromFile(Pointer_String_Game_INI_Section) != 0)
				{
					SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "Could not load the selected game.");
					DisplayDrawTextMessage(Shared_Buffer_Display, "SD card", "Failed to load the\ngame. Replace the SD\ncard and press Menu.");
					while (!KeyboardIsMenuKeyPressed());
					continue;
				}
				SERIAL_PORT_LOG(MAIN_IS_LOGGING_ENABLED, "The game was successfully loaded.");

				// TEST
				InterpreterRunProgram();
			}
		}
		// Settings
		else if (Keys_Mask & KEYBOARD_KEY_B) MainDisplaySettingsMenu();
		// Information
		else if (Keys_Mask & KEYBOARD_KEY_C)
		{
			DisplayDrawTextMessage(Shared_Buffer_Display, "- Information -", "Firmware : V" MAKEFILE_FIRMWARE_VERSION "\nDate : " __DATE__ "\nTime : " __TIME__ "\n\n\nD : back.");

			// Wait for the 'back' key to be pressed
			KeyboardWaitForKeys(KEYBOARD_KEY_D);
		}
	}
}
