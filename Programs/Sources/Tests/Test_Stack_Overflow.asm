; @file Test_Stack_Overflow.asm
; Check the interpreter behavior when encountering a Chip-8 stack overflow.
; @author Adrien RICCIARDI

	; Entry point
Recursive_Function:
	CALL Recursive_Function
