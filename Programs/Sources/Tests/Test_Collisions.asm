; @file Test_Collisions.asm
; Draw various colliding sprites, checking if each collision is successfully detected.
; @author Adrien RICCIARDI

; Private variables
define Row V0
define Column V1
define Sprite_Height 8

	; Entry point
	CLS

	; Draw a sprite at the middle of the screen
	LD Row, 28
	LD Column, 12
	LD I, Sprite
	DRW Row, Column, Sprite_Height
	; Check collision result
	SE VF, 0
	JP Collision_Error

	; Draw a sprite on the top, colliding with the last row
	LD Row, 28
	LD Column, 5
	LD I, Sprite
	DRW Row, Column, Sprite_Height
	; Check collision result
	SE VF, 1
	JP Collision_Error

	; Draw a sprite on the left, colliding with the last column
	LD Row, 21
	LD Column, 12
	LD I, Sprite
	DRW Row, Column, Sprite_Height
	; Check collision result
	SE VF, 1
	JP Collision_Error

	; Draw a sprite on the bottom, colliding with the last column
	LD Row, 28
	LD Column, 19
	LD I, Sprite
	DRW Row, Column, Sprite_Height
	; Check collision result
	SE VF, 1
	JP Collision_Error

	; Draw a sprite on the right, colliding with the last column
	LD Row, 35
	LD Column, 12
	LD I, Sprite
	DRW Row, Column, Sprite_Height
	; Check collision result
	SE VF, 1
	JP Collision_Error

	JP Infinite_Loop

	; Display a 'E' on the screen bottom-left corner
Collision_Error:
	LD Row, 0
	LD Column, 26
	LD V2, #0E
	HEX V2
	DRW Row, Column, 5

Infinite_Loop:
	JP Infinite_Loop

Sprite:
db #FF, #FF, #FF, #FF, #FF, #FF, #FF, #FF
