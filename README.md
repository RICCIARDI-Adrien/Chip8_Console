# Chip-8 Console

A Chip-8/SuperChip-8 console powered by a 8-bit PIC18 microcontroller.

![Console displaying the boot logo](Resources/Finished_Console_1.jpg)

## Specifications

* Microchip PIC18F27K42 8-bit microcontroller
* 64 MHz, giving a processing speed of up to 16 MIPS
* 8 KB of on-chip static RAM
* 128x64 pixels monochrome OLED display (Waveshare 2.42inch OLED Display Module)
* SD card interface to store games and files
* Powered by 2x AA batteries
* 8 keys usable with the Chip-8 interpreter, 1 menu key, 1 power-supply switch
* Monotone buzzer

## Dimensions

* Length : 175 mm
* Width : 95 mm
* Depth : 38 mm (including the adhesive feet)
* Weight : 1200 g (including the batteries and the SD card)

## Hardware

Display module :
![Display module front view](Resources/Display_Module_Top.jpg)
![Display module bottom view](Resources/Display_Module_Bottom.jpg)

Bare PCB :
![Bare PCB front view](Resources/Bare_PCB_Top.jpg)
![Bare PCB bottom view](Resources/Bare_PCB_Bottom.jpg)

Assembled PCB :
![Assembled PCB front view](Resources/Assembled_PCB_Top.jpg)
![Assembled PCB bottom view](Resources/Assembled_PCB_Bottom.jpg)

Assembled PCB with the display module screwed and connected :
![Assembled PCB with display front view](Resources/Assembled_PCB_With_Display_Top.jpg)
![Assembled PCB with display bottom view](Resources/Assembled_PCB_With_Display_Bottom.jpg)

## Casing

The casing is made of two laser-cut metal plates. They are both 4 mm thick and made of stainless 304 steel.  
The metal spacers are 20 mm high. The reference used here is `970200471` from Würth Elektronik.  

Casing plates :
![Casing top plate](Resources/Casing_Top_Plate.jpg)
![Casing bottom plate](Resources/Casing_Bottom_Plate.jpg)

Bottom plate with screws :
![Casing bottom plate with screws](Resources/Casing_Bottom_Plate_With_Screws.jpg)

Bottom plate with screwed PCB :
![Casing bottom plate with PCB](Resources/Casing_Bottom_Plate_With_PCB.jpg)

Fully assembled console :
![Assembled console front](Resources/Assembled_Console_Front.jpg)
![Assembled console batteries side](Resources/Assembled_Console_Side_Batteries.jpg)

## Battery pack hatch

The hatch is made of PCB FR4. The hatch does not require to be as strong as the console casing and the FR4 is a good material to cut complex shapes into.

The hatch is cut as a PCB panel containing all the required parts :
![Battery hatch panel](Resources/Battery_Hatch_Panel.jpg)

The first assembly step is to glue the hinges :
![Battery hatch hinges 1](Resources/Battery_Hatch_Hinges_1.jpg)
![Battery hatch hinges 2](Resources/Battery_Hatch_Hinges_2.jpg)
![Battery hatch hinges 3](Resources/Battery_Hatch_Hinges_3.jpg)
![Battery hatch hinges 4](Resources/Battery_Hatch_Hinges_4.jpg)

The second step is to screw the sliding part, and also the bolt used to manipulate the hatch slider :
![Battery hatch assembled 1](Resources/Battery_Hatch_Assembled_1.jpg)
![Battery hatch assembled 2](Resources/Battery_Hatch_Assembled_2.jpg)
![Battery hatch assembled 3](Resources/Battery_Hatch_Assembled_3.jpg)

There is not enough room to use Nyloc nuts, so normal nuts are used and held in place with threadlocker.  
The tightening of these bolts allows the slider to slide with a little force, so the hatch can't open alone.
![Battery hatch assembled 4](Resources/Battery_Hatch_Assembled_4.jpg)

The strike is made of a piece of PCB coming from the panel edge that is inserted by force between the two metal plates :
![Battery hatch strike 1](Resources/Battery_Hatch_Strike_1.jpg)
![Battery hatch strike 2](Resources/Battery_Hatch_Strike_2.jpg)
![Battery hatch strike 3](Resources/Battery_Hatch_Strike_3.jpg)

Now the hatch can be assembled onto the console.  
Locked hatch :
![Battery hatch locked](Resources/Battery_Hatch_Locked.jpg)

Unlocked hatch :
![Battery hatch unlocked](Resources/Battery_Hatch_Unlocked.jpg)

Opened hatch :
![Battery hatch opened](Resources/Battery_Hatch_Opened.jpg)

## Photo gallery

The console boot logo :
![Console boot logo](Resources/Finished_Console_2.jpg)

Improved the PCB supports that were made of several nuts. The nuts did not had the same width, resulting in the PCB not being perfectly flat. The nuts have then been replaced by precise 10mm spacers :
![PCB supports with nuts](Resources/PCB_Supports_Nuts.jpg)
![PCB supports with spacers](Resources/PCB_Supports_Spacers.jpg)

Improved the top plate button holes (using holes of 9.6mm diameter) :
![Improved top plate](Resources/Improved_Top_Plate.jpg)

The various console menus (as of firmware `V1.1`) :
![Console main menu](Resources/Console_Menu_Main.jpg)
![Console settings menu](Resources/Console_Menu_Settings.jpg)
![Console information menu](Resources/Console_Menu_Information.jpg)

Playing some video games :  
*3D VIP'r Maze* :
![Playing 3D VIP'r Maze](Resources/Playing_3D_VIP'r_Maze.jpg)
*Ant* :
![Playing Ant](Resources/Playing_Ant.jpg)
*Glitch Ghost* :
![Playing Glitch Ghost](Resources/Playing_Glitch_Ghost.jpg)
