/** @file Battery.c
 * See Battery.h for description.
 * @author Adrien RICCIARDI
 */
#include <Battery.h>
#include <Serial_Port.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define BATTERY_IS_LOGGING_ENABLED 1

/** ADC acquisition time registers value.
 * With a 3.3V microcontroller power supply, the sampling switch resistance Rss should be 6K, use 7K for a little safety margin.
 * The source impedance is the voltage divider one, which is equal to the resistors in parallel : Rs = (7360 * 10000) / (7360 + 10000) = 4240ohms.
 * Ric max is 1K, Chold = 28pF.
 * Tc = -Chold * (Ric + Rss + Rs) * ln(1 / 8191) = -28pF * (1K + 7K + 4240) * ln(1 / 8191) = 3.09us.
 * With a 50°C ambient temperature (which is a quite high margin) and the default settling time of 2us, Tacq = 2us + 3.09us + ((50°C - 25°C) * 0.05us/°C) = 6.34us.
 * With a 64MHz clock (Tclk = 1/64MHz = 16ns), the corresponding amount of clock cycles to wait is 6.34us / 0.016us = 396.25. Use 400 cycles to have a tiny margin.
 */
#define BATTERY_ADC_ACQUISITION_TIME 400

/** The ADC conversion value corresponding to a fully charged battery.
 * The minimum battery voltage is set to 2V (each AA cell would be at 1V. As the ADC value 4095 corresponds to 3.555V, the low voltage ADC value is 4095 * 2V / 3.555V = 2303.
 */
#define BATTERY_LOW_CHARGE_RAW_ADC_VALUE 2303

/** The ADC conversion value corresponding to a fully charged battery.
 * The ADC maximum value 4095 corresponds to 3.555V.
 * We consider that a 100% charged battery voltage made of 2 AA cells in serial is 3V, which corresponds to the ADC value 4095 * 3V / 3.555V = 3456.
 */
#define BATTERY_FULL_CHARGE_RAW_ADC_VALUE 3456

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void BatteryInitialize(void)
{
	// Configure the fixed voltage reference to generate a 2.048V voltage
	FVRCON = 0x02; // Do not enable the temperature indicator, enable only the buffer 1 that is connected to the ADC reference voltage multiplexer, select a 2x gain
	FVRCONbits.EN = 1; // Enable the module
	while (!FVRCONbits.RDY); // Wait for the reference voltage output to stabilize

	// Configure the pin connected to the battery voltage divider as an analog input
	ANSELBbits.ANSELB3 = 1;
	TRISBbits.TRISB3 = 1;

	// Configure the selected channel
	ADPCH = 0x0B; // Select ANB3 as the sampled channel

	// Configure the reference voltage
	ADREF = 0x03; // Connect VREF- to GND, connect VREF+ to the Fixed Reference Voltage buffer 1

	// Configure the ADC clock, a 1us per bit is fast enough and is a valid Tad
	ADCLK = 0x1F;

	// Configure the acquisition time using the equation 36-6 of the datasheet
	ADACQL = (unsigned char) BATTERY_ADC_ACQUISITION_TIME;
	ADACQH = BATTERY_ADC_ACQUISITION_TIME >> 8;

	// Disable the precharge time, as we are sampling a DC voltage
	ADPREH = 0;
	ADPREL = 0;

	// Do not use the ADC charge pump, as the results should be good enough with the Vdd supply voltage and this saves a little amount of power
	ADCP = 0;

	// Gather 8 samples before running the computations step
	ADRPT = 8;

	// Configure the module operations
	ADCON0 = 0x04; // Do not enable the ADC module yet, disable continuous operation, derive the ADC clock from the system clock, results are right-justified
	ADCON1 = 0; // Disable double sampling, do not care about the precharge settings
	ADCON2 = 0x33; // Divide the accumulator by 8 when performing the final computations, select Burst Average mode

	// Enable the ADC module
	ADCON0bits.ON = 1;
}

unsigned char BatteryGetCurrentChargePercentage(void)
{
	unsigned short Raw_Value;
	unsigned char Percentage;

	// Reset the result and temporary registers before the new sampling
	ADCON2bits.ACLR = 1;
	while (ADCON2bits.ACLR);

	// Start the conversion
	ADCON0bits.GO = 1;
	while (ADCON0bits.GO); // Wait for the conversion to terminate

	// Retrieve the conversion result
	Raw_Value = ((unsigned short) (ADFLTRH << 8)) | ADFLTRL;
	SERIAL_PORT_LOG(BATTERY_IS_LOGGING_ENABLED, "Raw ADC conversion value : %u.", Raw_Value);

	// Convert to a charge percentage : 0% if the voltage is lesser or equal to BATTERY_LOW_CHARGE_RAW_ADC_VALUE, 100% if the voltage is higher or equal to BATTERY_FULL_CHARGE_RAW_ADC_VALUE
	if (Raw_Value <= BATTERY_LOW_CHARGE_RAW_ADC_VALUE) Percentage = 0;
	else if (Raw_Value >= BATTERY_FULL_CHARGE_RAW_ADC_VALUE) Percentage = 100;
	else
	{
		Raw_Value -= BATTERY_LOW_CHARGE_RAW_ADC_VALUE; // Subtract the lower value to simplify the percentage computation
		Percentage = 100UL * Raw_Value / ((unsigned long) (BATTERY_FULL_CHARGE_RAW_ADC_VALUE - BATTERY_LOW_CHARGE_RAW_ADC_VALUE)); // The percentage value is in range ]BATTERY_LOW_CHARGE_RAW_ADC_VALUE; BATTERY_FULL_CHARGE_RAW_ADC_VALUE[
	}

	SERIAL_PORT_LOG(BATTERY_IS_LOGGING_ENABLED, "Percentage : %u%%.", Percentage);
	return Percentage;
}
