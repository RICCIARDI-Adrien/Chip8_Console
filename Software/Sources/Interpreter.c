/** @file Interpreter.c
 * See Interpreter.h for description.
 * @author Adrien RICCIARDI
 */
#include <Display.h>
#include <Interpreter.h>
#include <Serial_Port.h>

// TEST
#include <string.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define INTERPRETER_IS_LOGGING_ENABLED 1

/** The amount of general purpose registers (V0 to VF). */
#define INTERPRETER_REGISTERS_V_COUNT 16

/** The amount of levels in the virtual stack. */
#define INTERPRETER_STACK_SIZE 16

/** The interpreter memory size in bytes. */
#define INTERPRETER_MEMORY_SIZE 4096

#define INTERPRETER_DISPLAY_COLUMNS_COUNT_CHIP_8 64
#define INTERPRETER_DISPLAY_ROWS_COUNT_CHIP_8 32

//-------------------------------------------------------------------------------------------------
// Private variables
//-------------------------------------------------------------------------------------------------
/** Store the general purpose registers. */
static unsigned char Interpreter_Registers_V[INTERPRETER_REGISTERS_V_COUNT];
/** The index register I. */
static unsigned short Interpreter_Register_I;
/** The program counter register PC. */
static unsigned short Interpreter_Register_PC;
/** The stack pointer register SP. */
static unsigned char Interpreter_Register_SP;

/** The virtual stack. */
static unsigned short Interpreter_Stack[INTERPRETER_STACK_SIZE];

/** The whole interpreter memory containing the program instructions and data. */
static unsigned char Interpreter_Memory[INTERPRETER_MEMORY_SIZE] =
{
	0xF0, 0x10, 0x20, 0x40, 0x40
};

static unsigned char Interpreter_Frame_Buffer_Chip_8[128*64/8];//INTERPRETER_DISPLAY_COLUMNS_COUNT_CHIP_8 * INTERPRETER_DISPLAY_ROWS_COUNT_CHIP_8];

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
unsigned char InterpreterLoadProgramFromFile(TFATFileInformation *Pointer_File_Information)
{
	// TODO
	/*static const unsigned char a[] =
	{
		0x6e, 0x05, 0x65, 0x00, 0x6b, 0x06, 0x6a, 0x00, 0xa3, 0x0c, 0xda, 0xb1, 0x7a, 0x04, 0x3a, 0x40,
		0x12, 0x08, 0x7b, 0x01, 0x3b, 0x12, 0x12, 0x06, 0x6c, 0x20, 0x6d, 0x1f, 0xa3, 0x10, 0xdc, 0xd1,
		0x22, 0xf6, 0x60, 0x00, 0x61, 0x00, 0xa3, 0x12, 0xd0, 0x11, 0x70, 0x08, 0xa3, 0x0e, 0xd0, 0x11,
		0x60, 0x40, 0xf0, 0x15, 0xf0, 0x07, 0x30, 0x00, 0x12, 0x34, 0xc6, 0x0f, 0x67, 0x1e, 0x68, 0x01,
		0x69, 0xff, 0xa3, 0x0e, 0xd6, 0x71, 0xa3, 0x10, 0xdc, 0xd1, 0x60, 0x04, 0xe0, 0xa1, 0x7c, 0xfe,
		0x60, 0x06, 0xe0, 0xa1, 0x7c, 0x02, 0x60, 0x3f, 0x8c, 0x02, 0xdc, 0xd1, 0xa3, 0x0e, 0xd6, 0x71,
		0x86, 0x84, 0x87, 0x94, 0x60, 0x3f, 0x86, 0x02, 0x61, 0x1f, 0x87, 0x12, 0x47, 0x1f, 0x12, 0xac,
		0x46, 0x00, 0x68, 0x01, 0x46, 0x3f, 0x68, 0xff, 0x47, 0x00, 0x69, 0x01, 0xd6, 0x71, 0x3f, 0x01,
		0x12, 0xaa, 0x47, 0x1f, 0x12, 0xaa, 0x60, 0x05, 0x80, 0x75, 0x3f, 0x00, 0x12, 0xaa, 0x60, 0x01,
		0xf0, 0x18, 0x80, 0x60, 0x61, 0xfc, 0x80, 0x12, 0xa3, 0x0c, 0xd0, 0x71, 0x60, 0xfe, 0x89, 0x03,
		0x22, 0xf6, 0x75, 0x01, 0x22, 0xf6, 0x45, 0xc0, 0x13, 0x18, 0x12, 0x46, 0x69, 0xff, 0x80, 0x60,
		0x80, 0xc5, 0x3f, 0x01, 0x12, 0xca, 0x61, 0x02, 0x80, 0x15, 0x3f, 0x01, 0x12, 0xe0, 0x80, 0x15,
		0x3f, 0x01, 0x12, 0xee, 0x80, 0x15, 0x3f, 0x01, 0x12, 0xe8, 0x60, 0x20, 0xf0, 0x18, 0xa3, 0x0e,
		0x7e, 0xff, 0x80, 0xe0, 0x80, 0x04, 0x61, 0x00, 0xd0, 0x11, 0x3e, 0x00, 0x12, 0x30, 0x12, 0xde,
		0x78, 0xff, 0x48, 0xfe, 0x68, 0xff, 0x12, 0xee, 0x78, 0x01, 0x48, 0x02, 0x68, 0x01, 0x60, 0x04,
		0xf0, 0x18, 0x69, 0xff, 0x12, 0x70, 0xa3, 0x14, 0xf5, 0x33, 0xf2, 0x65, 0xf1, 0x29, 0x63, 0x37,
		0x64, 0x00, 0xd3, 0x45, 0x73, 0x05, 0xf2, 0x29, 0xd3, 0x45, 0x00, 0xee, 0xf0, 0x00, 0x80, 0x00,
		0xfc, 0x00, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x05, 0x00, 0xe0, 0x12, 0x04
	};
	memcpy(&Interpreter_Memory[0x200], a, sizeof(a));*/

	Interpreter_Memory[0x200] = 0x60; Interpreter_Memory[0x201] = 127; // LD V0, 0
	Interpreter_Memory[0x202] = 0x61; Interpreter_Memory[0x203] = 0; // LD V1, 0
	Interpreter_Memory[0x204] = 0xA0; Interpreter_Memory[0x205] = 0; // LD I, 0
	Interpreter_Memory[0x206] = 0xD0; Interpreter_Memory[0x207] = 0x15; // LD I, 0
	Interpreter_Memory[0x208] = 0x12; Interpreter_Memory[0x209] = 0x08; // JP 0x208

	// Configure the registers for the program execution
	Interpreter_Register_PC = 0x200; // The default entry point
	Interpreter_Register_SP = 0; // Clear the stack

	return 0;
}

unsigned char InterpreterRunProgram(void)
{
	unsigned char Instruction_High_Byte, Instruction_Low_Byte, Operand_1, Operand_2;
	unsigned short Temporary_Word;

	while (1)
	{
		// Make sure only the instruction address can't go out the array bounds
		Interpreter_Register_PC &= 0x0FFF;

		// Fetch the next instruction
		Instruction_High_Byte = Interpreter_Memory[Interpreter_Register_PC];
		Instruction_Low_Byte = Interpreter_Memory[Interpreter_Register_PC + 1];
		//SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Fetching instruction at address 0x%03X : 0x%02X 0x%02X.\r\n", Interpreter_Register_PC, Instruction_High_Byte, Instruction_Low_Byte);

		// Decode and execute the instruction
		switch (Instruction_High_Byte & 0xF0)
		{
			case 0x00:
			{
				switch (Instruction_Low_Byte)
				{
					// CLS
					case 0xE0:
						// TODO
						break;

					// RET
					case 0xEE:
						// Make sure there is a valid address on the stack
						if (Interpreter_Register_SP == 0)
						{
							SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Virtual program error : stack underflow. Stopping interpreter.\r\n");
							while (1);
						}
						Interpreter_Register_PC = Interpreter_Stack[Interpreter_Register_SP];
						Interpreter_Register_SP--;
						continue; // Bypass PC incrementation

					default:
						goto Invalid_Instruction;
				}
				break;
			}
			
			// JP addr
			case 0x10:
				Interpreter_Register_PC = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				Interpreter_Register_PC |= Instruction_Low_Byte;
				//SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "JP 0x%03X.\r\n", Interpreter_Register_PC);
				continue; // Bypass PC incrementation

			// CALL addr
			case 0x20:
				// Make sure there is still room on the stack
				if (Interpreter_Register_SP >= INTERPRETER_STACK_SIZE)
				{
					SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Virtual program error : stack overflow. Stopping interpreter.\r\n");
					while (1);
				}
				Interpreter_Stack[Interpreter_Register_SP] = Interpreter_Register_PC;
				Interpreter_Register_SP++;
				break;

			// SE Vx, byte
			case 0x30:
				// Skip the next instruction if Vx is equal to the immediate value
				Operand_1 = Instruction_High_Byte & 0x0F;
				if (Interpreter_Registers_V[Operand_1] == Instruction_Low_Byte) Interpreter_Register_PC += 2;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SE V%01X (= 0x%02X), 0x%02X.\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Instruction_Low_Byte);
				break;

			// SNE Vx, byte
			case 0x40:
				// Skip the next instruction if Vx is not equal to the immediate value
				Operand_1 = Instruction_High_Byte & 0x0F;
				if (Interpreter_Registers_V[Operand_1] != Instruction_Low_Byte) Interpreter_Register_PC += 2;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SNE V%01X (= 0x%02X), 0x%02X.\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Instruction_Low_Byte);
				break;

			// SE Vx, Vy
			case 0x50:
				// Skip the next instruction if Vx is not equal to the immediate value
				Operand_1 = Instruction_High_Byte & 0x0F;
				Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
				if (Interpreter_Registers_V[Operand_1] == Interpreter_Registers_V[Operand_2]) Interpreter_Register_PC += 2;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SE V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Operand_2, Interpreter_Registers_V[Operand_2]);
				break;

			// LD Vx, byte
			case 0x60:
				Operand_1 = Instruction_High_Byte & 0x0F;
				Interpreter_Registers_V[Operand_1] = Instruction_Low_Byte;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, 0x%02X.\r\n", Operand_1, Instruction_Low_Byte);
				break;

			// ADD Vx, byte
			case 0x70:
				Operand_1 = Instruction_High_Byte & 0x0F;
				Interpreter_Registers_V[Operand_1] += Instruction_Low_Byte;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "ADD V%01X, 0x%02X.\r\n", Operand_1, Instruction_Low_Byte);
				break;

			case 0x80:
			{
				switch (Instruction_Low_Byte & 0x0F)
				{
					// LD Vx, Vy
					case 0x00:
						Operand_1 = Instruction_High_Byte & 0x0F;
						Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
						Interpreter_Registers_V[Operand_1] = Interpreter_Registers_V[Operand_2];
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, V%01X (= 0x%02X).\r\n", Operand_1, Operand_2, Interpreter_Registers_V[Operand_2]);
						break;

					// OR Vx, Vy
					case 0x01:
						Operand_1 = Instruction_High_Byte & 0x0F;
						Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "OR V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Operand_2, Interpreter_Registers_V[Operand_2]);
						Interpreter_Registers_V[Operand_1] |= Interpreter_Registers_V[Operand_2];
						break;

					// AND Vx, Vy
					case 0x02:
						Operand_1 = Instruction_High_Byte & 0x0F;
						Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "AND V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Operand_2, Interpreter_Registers_V[Operand_2]);
						Interpreter_Registers_V[Operand_1] &= Interpreter_Registers_V[Operand_2];
						break;

					// XOR Vx, Vy
					case 0x03:
						Operand_1 = Instruction_High_Byte & 0x0F;
						Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "XOR V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Operand_2, Interpreter_Registers_V[Operand_2]);
						Interpreter_Registers_V[Operand_1] ^= Interpreter_Registers_V[Operand_2];
						break;

					// ADD Vx, Vy
					case 0x04:
						Operand_1 = Instruction_High_Byte & 0x0F;
						Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
						Temporary_Word = Interpreter_Registers_V[Operand_1] + Interpreter_Registers_V[Operand_2];
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "ADD V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Operand_2, Interpreter_Registers_V[Operand_2]);
						Interpreter_Registers_V[Operand_1] = (unsigned char) Temporary_Word;
						// Set VF register if carry is set
						if (Temporary_Word & 0x0100) Operand_1 = 1; // Recycle Operand_1 variable
						else Operand_1 = 0;
						Interpreter_Registers_V[15] = Operand_1;
						break;

					// SUB Vx, Vy
					case 0x05:
						Operand_1 = Instruction_High_Byte & 0x0F;
						Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
						Instruction_High_Byte = Interpreter_Registers_V[Operand_1]; // Recycle Instruction_XXX_Byte variables
						Instruction_Low_Byte = Interpreter_Registers_V[Operand_2];
						Interpreter_Registers_V[Operand_1] -= Instruction_Low_Byte;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SUB V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Operand_2, Instruction_Low_Byte);
						// Set VF register if borrow is clear
						if (Instruction_High_Byte > Instruction_Low_Byte) Operand_1 = 1; // Recycle Operand_1 variable
						else Operand_1 = 0;
						Interpreter_Registers_V[15] = Operand_1;
						break;

					// SHR Vx
					case 0x06:
						Operand_1 = Instruction_High_Byte & 0x0F;
						Instruction_High_Byte = Interpreter_Registers_V[Operand_1]; // Recycle Instruction_High_Byte variable
						if (Instruction_High_Byte & 0x01) Interpreter_Registers_V[15] = 1; // Set VF if the Vx least significant bit is set
						else Interpreter_Registers_V[Operand_1] = 0;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SHR V%01X (= 0x%02X).\r\n", Operand_1, Instruction_High_Byte);
						Instruction_High_Byte >>= 1;
						Interpreter_Registers_V[Operand_1] = Instruction_High_Byte;
						break;

					// SUBN Vx, Vy
					case 0x07:
						Operand_1 = Instruction_High_Byte & 0x0F;
						Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
						Instruction_High_Byte = Interpreter_Registers_V[Operand_1]; // Recycle Instruction_XXX_Byte variables
						Instruction_Low_Byte = Interpreter_Registers_V[Operand_2];
						Interpreter_Registers_V[Operand_1] = Instruction_Low_Byte - Instruction_High_Byte;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SUBN V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Operand_2, Instruction_Low_Byte);
						// Set VF register if borrow is clear
						if (Instruction_Low_Byte > Instruction_High_Byte) Operand_1 = 1; // Recycle Operand_1 variable
						else Operand_1 = 0;
						Interpreter_Registers_V[15] = Operand_1;
						break;

					// SHL Vx
					case 0x0E:
						Operand_1 = Instruction_High_Byte & 0x0F;
						Instruction_High_Byte = Interpreter_Registers_V[Operand_1]; // Recycle Instruction_High_Byte variable
						if (Instruction_High_Byte & 0x80) Interpreter_Registers_V[15] = 1; // Set VF if the Vx most significant bit is set
						else Interpreter_Registers_V[Operand_1] = 0;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SHL V%01X (= 0x%02X).\r\n", Operand_1, Instruction_High_Byte);
						Instruction_High_Byte <<= 1;
						Interpreter_Registers_V[Operand_1] = Instruction_High_Byte;
						break;

					default:
						goto Invalid_Instruction;
				}
				break;
			}

			// SNE Vx, Vy
			case 0x90:
				Operand_1 = Instruction_High_Byte & 0x0F;
				Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
				if (Interpreter_Registers_V[Operand_1] != Interpreter_Registers_V[Operand_2]) Interpreter_Register_PC += 2;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SNE V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Operand_2, Interpreter_Registers_V[Operand_2]);
				break;

			// LD I, addr
			case 0xA0:
				Interpreter_Register_I = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				Interpreter_Register_I |= Instruction_Low_Byte;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD I, 0x%03X.\r\n", Interpreter_Register_I);
				break;

			// JP V0, nnn
			case 0xB0:
				Temporary_Word = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "JP V0 (= 0x%02X), 0x%03X with PC = 0x%03X.\r\n", Interpreter_Registers_V[0], Temporary_Word, Interpreter_Register_PC);
				Interpreter_Register_PC = Interpreter_Registers_V[0] + Temporary_Word;
				Interpreter_Register_PC &= 0x0FFF; // Make sure the address does not cross the 4KB boundary
				continue; // Bypass PC incrementation

			// RND Vx, byte
			case 0xC0:
				// TODO
				break;

			// DRW Vx, Vy, nibble
			case 0xD0:
			{
				unsigned char *Pointer_Sprite, *Pointer_Display, Sprite_Size, Shift_Offset, Column, Row, Sprite_Row, Sprite_Column, Byte;
				//unsigned short

				// Extract the instruction parameters
				Operand_1 = Instruction_High_Byte & 0x0F;
				Operand_2 = (Instruction_Low_Byte >> 4) & 0x0F;
				Sprite_Size = Instruction_Low_Byte & 0x0F;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "DRW V%01X (= 0x%02X), V%01X (= 0x%02X), %d with I = 0x%03X.\r\n", Operand_1, Interpreter_Registers_V[Operand_1], Operand_2, Interpreter_Registers_V[Operand_2], Sprite_Size, Interpreter_Register_I);

				// Retrieve the sprite displaying coordinates and make sure they do not cross the display boundaries (screen wraping is not handled for now)
				Sprite_Column = Interpreter_Registers_V[Operand_1] /*& 0x3F*/; // Limit to 64 horizontal values in Chip-8 mode
				Sprite_Row = Interpreter_Registers_V[Operand_2] & 0x1F; // Limit to 32 vertical values in Chip-8 mode
				//Pointer_Display = &Interpreter_Frame_Buffer_Chip_8[(Operand_1 * INTERPRETER_DISPLAY_COLUMNS_COUNT_CHIP_8) / 8 + Operand_2];
				Pointer_Sprite = &Interpreter_Memory[Interpreter_Register_I];

				// Determine the amount of bits the sprite must be shifted (in case it would not fit entirely in a frame buffer byte)
				Shift_Offset = Sprite_Column & 0x07;
				printf("Shift_Offset = %d\r\n", Shift_Offset);
				for (Row = Sprite_Row; Row < INTERPRETER_DISPLAY_ROWS_COUNT_CHIP_8; Row++)
				{
					// Stop when the requested amount of sprite bytes has been displayed
					if (Sprite_Size == 0) break; // Start with this check in case the specified sprite size is 0, so the loop immediately exits

					Pointer_Display = &Interpreter_Frame_Buffer_Chip_8[Row * (/*INTERPRETER_DISPLAY_COLUMNS_COUNT_CHIP_8*/ 128 / 8) + (Sprite_Column / 8)];
					Byte = *Pointer_Sprite;

					//
					if (Shift_Offset == 0) *Pointer_Display = Byte;
					else
					{
						*Pointer_Display = Byte >> Shift_Offset;
						if (Sprite_Column < (128/8) - 1) *(Pointer_Display + 1) = (unsigned char) (Byte << (8 - Shift_Offset));
					}

					printf("Row = %d, Sprite_Size = %d, Sprite_Column = %d\r\n", Row, Sprite_Size, Sprite_Column);

					Pointer_Sprite++;
					Sprite_Size--;
				}

				DisplayShowBuffer(Interpreter_Frame_Buffer_Chip_8);
				break;
			}

			case 0xF0:
			{
				switch (Instruction_Low_Byte)
				{
					// LD Vx, DT
					case 0x07:
						// TODO
						break;

					// LD DT, Vx
					case 0x15:
						// TODO
						break;

					default:
						goto Invalid_Instruction;
				}
				break;
			}

			default:
				goto Invalid_Instruction;
		}

		// Increment the instruction address
		Interpreter_Register_PC += 2;
	}

Invalid_Instruction:
	// Always display this error, even if logging is not enabled
	SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Invalid instruction 0x%02X%02X at address 0x%03X. Stopping interpreter.\r\n", Instruction_High_Byte, Instruction_Low_Byte, Interpreter_Register_PC);
	while (1);

	return 0;
}