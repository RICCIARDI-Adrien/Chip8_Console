/** @file EEPROM.c
 * See EEPROM.h for description.
 * @author Adrien RICCIARDI
 */
#include <EEPROM.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
unsigned char EEPROMReadByte(unsigned short Address)
{
	// Do nothing if the provided address is invalid
	if (Address >= EEPROM_SIZE) return 0;

	// Set the target address
	NVMADRL = (unsigned char) Address;
	NVMADRH = Address >> 8;

	// Select the EEPROM memory
	NVMCON1 = 0;

	// Start the reading operation
	NVMCON1bits.RD = 1;
	while (NVMCON1bits.RD); // Wait for the read cycle to terminate

	return NVMDAT;
}

void EEPROMWriteByte(unsigned short Address, unsigned char Value)
{
	// Do nothing if the provided address is invalid
	if (Address >= EEPROM_SIZE) return;

	// Set the target address
	NVMADRL = (unsigned char) Address;
	NVMADRH = Address >> 8;

	// Set the target data
	NVMDAT = Value;

	// Select the EEPROM memory
	NVMCON1 = 0;

	// Allow write operations
	NVMCON1bits.WREN = 1;
	
	// Execute the unlocking sequence
	INTCON0bits.GIE = 0; // Disable all interrupts to avoid the unlocking sequence being perturbed
	NVMCON2 = 0x55;
	NVMCON2 = 0xAA;
	NVMCON1bits.WR = 1; // Start the writing operation immediately after the unlocking sequence
	INTCON0bits.GIE = 1; // Re-enable interrupts

	// Wait for the write cycle to terminate
	while (NVMCON1bits.WR);

	// Disable write operations
	NVMCON1bits.WREN = 0;
}
