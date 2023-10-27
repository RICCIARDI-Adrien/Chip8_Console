/** @file Main.c
 * Entry point and main loop for the Chip-8 Console.
 * @author Adrien RICCIARDI
 */
#include <NCO.h>
#include <Serial_Port.h>
#include <Sound.h>
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
// Entry point
//-------------------------------------------------------------------------------------------------
void main(void)
{
	// Wait for the internal oscillator to stabilize
	while (!OSCSTATbits.HFOR);

	// TEST
	ANSELCbits.ANSELC0 = 0;
	LATCbits.LATC0 = 0;
	TRISCbits.TRISC0 = 0;

	// Initialize all needed modules
	SerialPortInitialize();
	NCOInitialize(); // This module must be initialized before the sound module
	SoundInitialize();

	// TEST
	while (1)
	{
		LATCbits.LATC0 = !LATCbits.LATC0;
		SerialPortWriteString("Bonjour !\r\n");
		__delay_ms(1000);

		SoundPlay(255);
		__delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000);

		SoundPlay(127);
		__delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000);

		SoundPlay(64);
		__delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000);

		SoundPlay(1);
		__delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000); __delay_ms(1000);
	}
}
