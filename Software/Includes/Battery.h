/** @file Battery.h
 * Sample the battery voltage and determine the battery charge.
 * @author Adrien RICCIARDI
 */
#ifndef H_BATTERY_H
#define H_BATTERY_H

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Initialize the ADC module to sample the battery voltage. */
void BatteryInitialize(void);

/** Sample the battery voltage and determine the current charge.
 * @return The battery charge as a percentage.
 */
unsigned char BatteryGetCurrentChargePercentage(void);

#endif
