/** @file Interpreter.c
 * See Interpreter.h for description.
 * @author Adrien RICCIARDI
 */
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

/** How many instructions can be stored in the interpreter memory. */
#define INTERPRETER_MEMORY_SIZE 4096
#define INTERPRETER_INSTRUCTIONS_COUNT (INTERPRETER_MEMORY_SIZE / 2) // Each instruction is 2-byte wide and the memory is 

//-------------------------------------------------------------------------------------------------
// Private types
//-------------------------------------------------------------------------------------------------
/** Allow to easily access to the interpreter memory. */
typedef union
{
	unsigned short Instructions[INTERPRETER_INSTRUCTIONS_COUNT];
	unsigned char Bytes[INTERPRETER_MEMORY_SIZE];
} TInterpreterMemory;

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
static TInterpreterMemory Interpreter_Memory;

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
unsigned char InterpreterLoadProgramFromFile(TFATFileInformation *Pointer_File_Information)
{
	// TODO
	static unsigned char a[] =
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
	memcpy(&Interpreter_Memory.Bytes[0x200], a, sizeof(a));
	
	/*Interpreter_Memory.Bytes[0x200] = 0x6E; Interpreter_Memory.Bytes[0x201] = 0x05;
	Interpreter_Memory.Bytes[0x202] = 0x65; Interpreter_Memory.Bytes[0x203] = 0x00;
	Interpreter_Memory.Bytes[0x204] = 0x6B; Interpreter_Memory.Bytes[0x205] = 0x06;
	Interpreter_Memory.Bytes[0x206] = 0x6A; Interpreter_Memory.Bytes[0x207] = 0x00;
	Interpreter_Memory.Bytes[0x208] = 0xA3; Interpreter_Memory.Bytes[0x209] = 0x0C;
	Interpreter_Memory.Bytes[0x20A] = 0xDA; Interpreter_Memory.Bytes[0x20B] = 0xB1;
	Interpreter_Memory.Bytes[0x20C] = 0x7A; Interpreter_Memory.Bytes[0x20D] = 0x04;
	Interpreter_Memory.Bytes[0x20E] = 0x3A; Interpreter_Memory.Bytes[0x20F] = 0x40;
	Interpreter_Memory.Bytes[0x210] = 0x12; Interpreter_Memory.Bytes[0x211] = 0x08;
	Interpreter_Memory.Bytes[0x212] = 0x7B; Interpreter_Memory.Bytes[0x213] = 0x01;
	Interpreter_Memory.Bytes[0x214] = 0x3B; Interpreter_Memory.Bytes[0x215] = 0x12;
	Interpreter_Memory.Bytes[0x216] = 0x12; Interpreter_Memory.Bytes[0x217] = 0x06;
	Interpreter_Memory.Bytes[0x218] = 0x6C; Interpreter_Memory.Bytes[0x219] = 0x20;
	Interpreter_Memory.Bytes[0x21A] = 0x6D; Interpreter_Memory.Bytes[0x21B] = 0x1F;
	Interpreter_Memory.Bytes[0x21C] = 0xA3; Interpreter_Memory.Bytes[0x21D] = 0x10;
	Interpreter_Memory.Bytes[0x21E] = 0xDC; Interpreter_Memory.Bytes[0x21F] = 0xD1;
	Interpreter_Memory.Bytes[0x220] = 0x22; Interpreter_Memory.Bytes[0x221] = 0xF6;*/

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
		Instruction_High_Byte = Interpreter_Memory.Bytes[Interpreter_Register_PC];
		Instruction_Low_Byte = Interpreter_Memory.Bytes[Interpreter_Register_PC + 1];
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Fetching instruction at address 0x%03X : 0x%02X 0x%02X.\r\n", Interpreter_Register_PC, Instruction_High_Byte, Instruction_Low_Byte);

		// 
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
						// TODO
						break;

					default:
						goto Invalid_Instruction;
				}
				break;
			}
			
			// JP addr
			case 0x10:
				Interpreter_Register_PC = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				Interpreter_Register_PC |= Instruction_Low_Byte;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "JP 0x%03X.\r\n", Interpreter_Register_PC);
				continue; // Bypass PC incrementation

			case 0x20:
				// TODO
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

					default:
						goto Invalid_Instruction;
				}
				break;
			}

			// LD I, addr
			case 0xA0:
				Interpreter_Register_I = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				Interpreter_Register_I |= Instruction_Low_Byte;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD I, 0x%03X.\r\n", Interpreter_Register_I);
				break;

			// DRW Vx, Vy, nibble
			case 0xD0:
				// TODO
				break;

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
