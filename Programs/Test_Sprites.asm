; @file Test_Sprites.asm
; Display two lines made of the built-in alphanumerical sprites.
; @author Adrien RICCIARDI

; Private variables
define Row V0
define Column V1
define Sprite_Index V2

    ; Entry point
    LD Row, 0
    LD Column, 0
    LD Sprite_Index, 0

    ; Display the next sprite to the next displaying location
Display_Sprite:
    LD F, Sprite_Index
    DRW Column, Row, 5

    ; Go to next displaying column
    ADD Column, 8
    SE Column, 64 ; Was the last displaying column reached ?
    JP Increment_Sprite_Index
    ; Yes, go to second displaying row
    ADD Row, 8
    LD Column, 0

    ; Select the next sprite to display and check if all sprites have been displayed
Increment_Sprite_Index:
    ADD Sprite_Index, 1
    SNE Sprite_Index, 16 ; Is the last index reached ?
Infinite_Loop:
    JP Infinite_Loop ; Yes
    JP Display_Sprite ; No
