/** @file Display.h
 * Control an OLED display using a SS1309 controller through a 4-wire SPI bus.
 * @author Adrien RICCIARDI
 */
#ifndef H_DISPLAY_H
#define H_DISPLAY_H

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------
/** Configure the display controller, clear its data RAM and turn the display on. */
void DisplayInitialize(void);

#endif
