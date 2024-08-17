/** @file Interpreter.c
 * See Interpreter.h for description.
 * @author Adrien RICCIARDI
 */
#include <Display.h>
#include <FAT.h>
#include <INI_Parser.h>
#include <Interpreter.h>
#include <Keyboard.h>
#include <NCO.h>
#include <Serial_Port.h>
#include <Shared_Buffer.h>
#include <Sound.h>
#include <string.h>
#include <xc.h>

//-------------------------------------------------------------------------------------------------
// Private constants
//-------------------------------------------------------------------------------------------------
/** Set to 1 to enable the log messages, set to 0 to disable them. */
#define INTERPRETER_IS_LOGGING_ENABLED 1
/** Set to 1 to enable the interpreter debugger facility. */
#define INTERPRETER_IS_DEBUGGER_ENABLED 0

/** The amount of general purpose registers (V0 to VF). */
#define INTERPRETER_REGISTERS_V_COUNT 16

/** The amount of levels in the virtual stack. */
#define INTERPRETER_STACK_SIZE 16

/** The Chip-8 default program entry point. */
#define INTERPRETER_PROGRAM_ENTRY_POINT 0x200

#define INTERPRETER_DISPLAY_COLUMNS_COUNT_CHIP_8 64
#define INTERPRETER_DISPLAY_ROWS_COUNT_CHIP_8 32

#define INTERPRETER_DISPLAY_COLUMNS_COUNT_SUPER_CHIP_8 128
#define INTERPRETER_DISPLAY_ROWS_COUNT_SUPER_CHIP_8 64

/** The total amount of keys supported by Chip-8. */
#define INTERPRETER_KEYS_COUNT_CHIP_8 16
/** The total amount of hardware keys supported by this console. */
#define INTERPRETER_KEYS_COUNT_CONSOLE 8

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
static const unsigned char Interpreter_Fonts[] =
{
	// 0
	0xF0, 0x90, 0x90, 0x90, 0xF0,
	// 1
	0x20, 0x60, 0x20, 0x20, 0x70,
	// 2
	0xF0, 0x10, 0xF0, 0x80, 0xF0,
	// 3
	0xF0, 0x10, 0xF0, 0x10, 0xF0,
	// 4
	0x90, 0x90, 0xF0, 0x10, 0x10,
	// 5
	0xF0, 0x80, 0xF0, 0x10, 0xF0,
	// 6
	0xF0, 0x80, 0xF0, 0x90, 0xF0,
	// 7
	0xF0, 0x10, 0x20, 0x40, 0x40,
	// 8
	0xF0, 0x90, 0xF0, 0x90, 0xF0,
	// 9
	0xF0, 0x90, 0xF0, 0x10, 0xF0,
	// A
	0xF0, 0x90, 0xF0, 0x90, 0x90,
	// B
	0xE0, 0x90, 0xE0, 0x90, 0xE0,
	// C
	0xF0, 0x80, 0x80, 0x80, 0xF0,
	// D
	0xE0, 0x90, 0x90, 0x90, 0xE0,
	// E
	0xF0, 0x80, 0xF0, 0x80, 0xF0,
	// F
	0xF0, 0x80, 0xF0, 0x80, 0x80
};

/** Hold a look-up table returning the physical switch code corresponding to a logical Chip-8 key code. This allows to remap Chip-8 program keys to the physical mapping of the console. */
static unsigned char Interpreter_Keys_Table_From_Interpreter[INTERPRETER_KEYS_COUNT_CHIP_8];
/** This reverse table allows to determine which Chip-8 key code to return when a console switch is pressed. This table is automatically created from the Interpreter_Keys_Table_From_Interpreter table content. */
static unsigned char Interpreter_Keys_Table_From_Console[INTERPRETER_KEYS_COUNT_CONSOLE];

/** The random seed. */
static unsigned char Interpreter_Random_Seed;

/** Tell whether the game enabled the fast display rendering feature. */
static unsigned char Interpreter_Is_Fast_Rendering_Enabled;

//-------------------------------------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------------------------------------
/** Retrieve the keys configuration from the game INI configuration.
 * @param Pointer_String_Game_INI_Section The INI section corresponding to the game.
 * @return 0 on success,
 * @return 1 if an error occurred.
 */
static unsigned char InterpreterConfigureKeyBindings(char *Pointer_String_Game_INI_Section)
{
	typedef struct
	{
		const char *Pointer_String_INI_Key_Name;
		unsigned char Keyboard_Key_Code;
	} TKeyBinding;
	static TKeyBinding Key_Bindings[INTERPRETER_KEYS_COUNT_CONSOLE] =
	{
		{ "KeyValueUp", KEYBOARD_KEY_UP },
		{ "KeyValueDown", KEYBOARD_KEY_DOWN },
		{ "KeyValueLeft", KEYBOARD_KEY_LEFT },
		{ "KeyValueRight", KEYBOARD_KEY_RIGHT },
		{ "KeyValueA", KEYBOARD_KEY_A },
		{ "KeyValueB", KEYBOARD_KEY_B },
		{ "KeyValueC", KEYBOARD_KEY_C },
		{ "KeyValueD", KEYBOARD_KEY_D }
	};
	TKeyBinding *Pointer_Key_Binding;
	unsigned char i, Console_Key_Mask, Console_Key_Index, Key_Index;

	// Map each console key code to the corresponding Chip-8 code
	for (i = 0; i < INTERPRETER_KEYS_COUNT_CONSOLE; i++)
	{
		// Cache the current binding access
		Pointer_Key_Binding = &Key_Bindings[i];
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Retrieving key binding for \"%s\".", Pointer_Key_Binding->Pointer_String_INI_Key_Name);

		// Try to retrieve the corresponding Chip-8 code
		Key_Index = INIParserRead8BitInteger(Pointer_String_Game_INI_Section, Pointer_Key_Binding->Pointer_String_INI_Key_Name);
		if (Key_Index >= INTERPRETER_KEYS_COUNT_CHIP_8)
		{
			SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Invalid Chip-8 key code, it must be in range 0 to 15 (read value is %u).", Key_Index);
			DisplayDrawTextMessage(Shared_Buffer_Display, "Chip-8", "Invalid key code for\n%s.\nPress Menu to exit.");
			while (!KeyboardIsMenuKeyPressed());
			return 1;
		}
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Chip-8 key binding value is %u.", Key_Index);
		Interpreter_Keys_Table_From_Interpreter[Key_Index] = Pointer_Key_Binding->Keyboard_Key_Code;
	}

	// Generate a reverse look-up table to match a Chip-8 key code with a console key switch
	Console_Key_Mask = 0x01;
	for (Console_Key_Index = 0; Console_Key_Index < INTERPRETER_KEYS_COUNT_CONSOLE; Console_Key_Index++)
	{
		for (Key_Index = 0; Key_Index < INTERPRETER_KEYS_COUNT_CHIP_8; Key_Index++)
		{
			if (Interpreter_Keys_Table_From_Interpreter[Key_Index] == Console_Key_Mask)
			{
				Interpreter_Keys_Table_From_Console[Console_Key_Index] = Key_Index;
				break;
			}
		}
		Console_Key_Mask <<= 1;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void InterpreterInitialize(void)
{
	// Use timer 6 as the Chip-8 delay timer
	T6CLK = 0x09; // Clock the timer by the NCO module
	T6HLT = 0xA8; // Prescaler output is synchronized with Fosc/4, also synchronize the ON bit with the timer clock input, select the one-shot mode with one-shot operation and software start
	T6CON = 0; // Do not enable the timer yet, do not enable any prescaler or postscaler (the timer is already clocked at the desired frequency)
}

unsigned char InterpreterLoadProgramFromFile(char *Pointer_String_Game_INI_Section)
{
	unsigned char Result;
	char *Pointer_String;
	TFATFileInformation File_Information;

	// Assign the console keys to the Chip-8 values expected by the game (do that before loading the ROM file because the INI data is stored in the same buffer that the one in which the ROM file will be loaded)
	if (InterpreterConfigureKeyBindings(Pointer_String_Game_INI_Section) != 0)
	{
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Error : failed to configure the key bindings.");
		return 1;
	}

	// Retrieve the game ROM file name
	Pointer_String = INIParserReadString(Pointer_String_Game_INI_Section, "ROMFile");
	if (Pointer_String == NULL)
	{
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Error : failed to retrieve the game ROM file from the INI configuration.");
		return 1;
	}
	SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "ROM file name : \"%s\".", Pointer_String);

	// If the key is not found, the fast rendering is disabled, in order to disable it by default
	Interpreter_Is_Fast_Rendering_Enabled = INIParserRead8BitInteger(Pointer_String_Game_INI_Section, "FastRendering");

	// Begin listing the files
	if (FATListStart("/") != 0)
	{
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "FATListStart() failed.");
		return 1;
	}

	// Search for the game ROM file
	while (1)
	{
		Result = FATListNext(&File_Information);
		if (Result != 0) break;
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "File found : name=\"%s\", is directory=%u, size=%lu, first cluster=%lu.",
			File_Information.String_Short_Name,
			File_Information.Is_Directory,
			File_Information.Size,
			File_Information.First_Cluster_Number);

		// Do not take directories into account
		if (File_Information.Is_Directory)
		{
			SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "This is a directory, skipping it.");
			continue;
		}

		// Is this the searched file ?
		if (strcmp((char *) File_Information.String_Short_Name, Pointer_String) == 0)
		{
			SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Found the game file, loading it.");
			break;
		}
	}
	// Was the file found ?
	if (Result != 0)
	{
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Error : could not find the game file, stopping.");
		return 1;
	}

	// Load the file
	if (FATReadFile(&File_Information, &Shared_Buffers.Interpreter_Memory[INTERPRETER_PROGRAM_ENTRY_POINT], INTERPRETER_MEMORY_SIZE - INTERPRETER_PROGRAM_ENTRY_POINT) != 0)
	{
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Error : failed to load the program from the file named \"%s\".", File_Information.String_Short_Name);
		return 1;
	}

	// Place the built-in fonts at the beginning of the interpreter memory
	memcpy(Shared_Buffers.Interpreter_Memory, Interpreter_Fonts, sizeof(Interpreter_Fonts));

	// Configure the registers for the program execution
	Interpreter_Register_PC = INTERPRETER_PROGRAM_ENTRY_POINT; // The default entry point
	Interpreter_Register_SP = 0; // Clear the stack
	// Reset all other registers
	Interpreter_Register_I = 0;
	memset(Interpreter_Registers_V, 0, sizeof(Interpreter_Registers_V));

	// Clear the frame buffer
	memset(Shared_Buffer_Display, 0, sizeof(Shared_Buffer_Display));

	// Purge any spurious press of the menu key
	KeyboardIsMenuKeyPressed();

	// Use the timer feeding the sound PWM generation (which is always running) to initialize the random seed for the entire Chip-8 program execution
	Interpreter_Random_Seed = T2TMR;

	return 0;
}

unsigned char InterpreterRunProgram(void)
{
	unsigned char Display_Rows_Count, Display_Columns_Count, Instruction_High_Byte, Instruction_Low_Byte, Is_Rendering_Needed = 0, Is_Super_Chip_8_Mode = 0; // TODO Initialize Is_Super_Chip_8_Mode according to the program to run

	// Configure the display settings according to the selected emulation mode
	if (Is_Super_Chip_8_Mode)
	{
		Display_Columns_Count = INTERPRETER_DISPLAY_COLUMNS_COUNT_SUPER_CHIP_8;
		Display_Rows_Count = INTERPRETER_DISPLAY_ROWS_COUNT_SUPER_CHIP_8;
	}
	else
	{
		Display_Columns_Count = INTERPRETER_DISPLAY_COLUMNS_COUNT_CHIP_8;
		Display_Rows_Count = INTERPRETER_DISPLAY_ROWS_COUNT_CHIP_8;
	}
	SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Emulation mode : %s, display columns count = %d, display rows count = %d.", Is_Super_Chip_8_Mode ? "SuperChip-8" : "Chip-8", Display_Columns_Count, Display_Rows_Count);

	while (1)
	{
		// Exit when the menu key is pressed
		if (KeyboardIsMenuKeyPressed()) return 0;

		// Make sure only the instruction address can't go out the array bounds
		Interpreter_Register_PC &= 0x0FFF;

		// Fetch the next instruction
		Instruction_High_Byte = Shared_Buffers.Interpreter_Memory[Interpreter_Register_PC];
		Instruction_Low_Byte = Shared_Buffers.Interpreter_Memory[Interpreter_Register_PC + 1];
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "**** Fetching instruction at address 0x%03X : 0x%02X 0x%02X.", Interpreter_Register_PC, Instruction_High_Byte, Instruction_Low_Byte);

		// Decode and execute the instruction
		switch (Instruction_High_Byte & 0xF0)
		{
			case 0x00:
			{
				switch (Instruction_Low_Byte)
				{
					// CLS
					case 0xE0:
						memset(Shared_Buffer_Display, 0, sizeof(Shared_Buffer_Display));
						DisplayDrawFullSizeBuffer(Shared_Buffer_Display);
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "CLS.");
						break;

					// RET
					case 0xEE:
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "RET (SP = %d).", Interpreter_Register_SP);
						// Make sure there is a valid address on the stack
						if (Interpreter_Register_SP == 0)
						{
							SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Virtual program error : stack underflow. Stopping interpreter.");
							while (1);
						}
						Interpreter_Register_SP--; // The CALL instruction increments the stack pointer after pushing, so the RET instruction needs to decrement the stack pointer before popping
						Interpreter_Register_PC = Interpreter_Stack[Interpreter_Register_SP];
						goto Next_Instruction; // Bypass PC incrementation

					default:
						goto Invalid_Instruction;
				}
				break;
			}
			
			// JP addr
			case 0x10:
				Interpreter_Register_PC = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				Interpreter_Register_PC |= Instruction_Low_Byte;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "JP 0x%03X.", Interpreter_Register_PC);
				goto Next_Instruction; // Bypass PC incrementation

			// CALL addr
			case 0x20:
			{
				unsigned short Address;

				// Extract the operands
				Address = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				Address |= Instruction_Low_Byte; // The address will by default be on 12 bits due to the instruction encoding, so no need to check for an overflow
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "CALL 0x%03X (SP = %d).", Address, Interpreter_Register_SP);

				// Make sure there is still room on the stack
				if (Interpreter_Register_SP >= INTERPRETER_STACK_SIZE)
				{
					SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Virtual program error : stack overflow. Stopping interpreter.");
					while (1);
				}
				Interpreter_Stack[Interpreter_Register_SP] = Interpreter_Register_PC + 2; // Store the address of the instruction following this one
				Interpreter_Register_SP++;

				// Jump to the function entry point
				Interpreter_Register_PC = Address;
				goto Next_Instruction; // Bypass PC incrementation
			}

			// SE Vx, byte
			case 0x30:
			{
				unsigned char Register_Index;

				// Extract the operands
				Register_Index = Instruction_High_Byte & 0x0F;

				// Skip the next instruction if Vx is equal to the immediate value
				if (Interpreter_Registers_V[Register_Index] == Instruction_Low_Byte) Interpreter_Register_PC += 2;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SE V%01X (= 0x%02X), 0x%02X.", Register_Index, Interpreter_Registers_V[Register_Index], Instruction_Low_Byte);
				break;
			}

			// SNE Vx, byte
			case 0x40:
			{
				unsigned char Register_Index;

				// Extract the operands
				Register_Index = Instruction_High_Byte & 0x0F;

				// Skip the next instruction if Vx is not equal to the immediate value
				if (Interpreter_Registers_V[Register_Index] != Instruction_Low_Byte) Interpreter_Register_PC += 2;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SNE V%01X (= 0x%02X), 0x%02X.", Register_Index, Interpreter_Registers_V[Register_Index], Instruction_Low_Byte);
				break;
			}

			// SE Vx, Vy
			case 0x50:
			{
				unsigned char Register_Index_1, Register_Index_2;

				// Extract the operands
				Register_Index_1 = Instruction_High_Byte & 0x0F;
				Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

				// Skip the next instruction if Vx is not equal to Vy
				if (Interpreter_Registers_V[Register_Index_1] == Interpreter_Registers_V[Register_Index_2]) Interpreter_Register_PC += 2;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SE V%01X (= 0x%02X), V%01X (= 0x%02X).", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
				break;
			}

			// LD Vx, byte
			case 0x60:
			{
				unsigned char Register_Index, Immediate_Value;

				// Extract the operands
				Register_Index = Instruction_High_Byte & 0x0F;
				Immediate_Value = Instruction_Low_Byte;

				Interpreter_Registers_V[Register_Index] = Immediate_Value;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, 0x%02X.", Register_Index, Immediate_Value);
				break;
			}

			// ADD Vx, byte
			case 0x70:
			{
				unsigned char Register_Index, Immediate_Value;

				// Extract the operands
				Register_Index = Instruction_High_Byte & 0x0F;
				Immediate_Value = Instruction_Low_Byte;

				Interpreter_Registers_V[Register_Index] += Immediate_Value;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "ADD V%01X, 0x%02X.", Register_Index, Immediate_Value);
				break;
			}

			case 0x80:
			{
				switch (Instruction_Low_Byte & 0x0F)
				{
					// LD Vx, Vy
					case 0x00:
					{
						unsigned char Register_Index_1, Register_Index_2;

						// Extract the operands
						Register_Index_1 = Instruction_High_Byte & 0x0F;
						Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

						Interpreter_Registers_V[Register_Index_1] = Interpreter_Registers_V[Register_Index_2];
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, V%01X (= 0x%02X).", Register_Index_1, Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
						break;
					}

					// OR Vx, Vy
					case 0x01:
					{
						unsigned char Register_Index_1, Register_Index_2;

						// Extract the operands
						Register_Index_1 = Instruction_High_Byte & 0x0F;
						Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "OR V%01X (= 0x%02X), V%01X (= 0x%02X).", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
						Interpreter_Registers_V[Register_Index_1] |= Interpreter_Registers_V[Register_Index_2];
						break;
					}

					// AND Vx, Vy
					case 0x02:
					{
						unsigned char Register_Index_1, Register_Index_2;

						// Extract the operands
						Register_Index_1 = Instruction_High_Byte & 0x0F;
						Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "AND V%01X (= 0x%02X), V%01X (= 0x%02X).", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
						Interpreter_Registers_V[Register_Index_1] &= Interpreter_Registers_V[Register_Index_2];
						break;
					}

					// XOR Vx, Vy
					case 0x03:
					{
						unsigned char Register_Index_1, Register_Index_2;

						// Extract the operands
						Register_Index_1 = Instruction_High_Byte & 0x0F;
						Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "XOR V%01X (= 0x%02X), V%01X (= 0x%02X).", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
						Interpreter_Registers_V[Register_Index_1] ^= Interpreter_Registers_V[Register_Index_2];
						break;
					}

					// ADD Vx, Vy
					case 0x04:
					{
						unsigned char Register_Index_1, Register_Index_2;
						unsigned short Sum;

						// Extract the operands
						Register_Index_1 = Instruction_High_Byte & 0x0F;
						Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

						Sum = Interpreter_Registers_V[Register_Index_1] + Interpreter_Registers_V[Register_Index_2]; // Compute the operation on a 16-bit variable to be able to detect an overflow
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "ADD V%01X (= 0x%02X), V%01X (= 0x%02X).", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
						Interpreter_Registers_V[Register_Index_1] = (unsigned char) Sum;

						// Set VF register if carry is set
						if (Sum & 0x0100) Interpreter_Registers_V[15] = 1;
						else Interpreter_Registers_V[15] = 0;
						break;
					}

					// SUB Vx, Vy
					case 0x05:
					{
						unsigned char Register_Index_1, Register_Index_2, Register_Value_1, Register_Value_2;

						// Extract the operands
						Register_Index_1 = Instruction_High_Byte & 0x0F;
						Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

						Register_Value_1 = Interpreter_Registers_V[Register_Index_1];
						Register_Value_2 = Interpreter_Registers_V[Register_Index_2];
						Interpreter_Registers_V[Register_Index_1] -= Register_Value_2;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SUB V%01X (= 0x%02X), V%01X (= 0x%02X).", Register_Index_1, Register_Value_1, Register_Index_2, Register_Value_2);

						// Set VF register if borrow is clear
						if (Register_Value_1 >= Register_Value_2) Interpreter_Registers_V[15] = 1;
						else Interpreter_Registers_V[15] = 0;
						break;
					}

					// SHR Vx
					case 0x06:
					{
						unsigned char Register_Index, Value, Is_Bit_Shifted_Out;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;

						Value = Interpreter_Registers_V[Register_Index];
						if (Value & 0x01) Is_Bit_Shifted_Out = 1; // Set VF if the Vx least significant bit is set
						else Is_Bit_Shifted_Out = 0;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SHR V%01X (= 0x%02X).", Register_Index, Value);
						Value >>= 1;
						Interpreter_Registers_V[Register_Index] = Value;

						// Set VF at the end, in case VF was the output register just before
						Interpreter_Registers_V[15] = Is_Bit_Shifted_Out;
						break;
					}

					// SUBN Vx, Vy
					case 0x07:
					{
						unsigned char Register_Index_1, Register_Index_2, Register_Value_1, Register_Value_2;

						// Extract the operands
						Register_Index_1 = Instruction_High_Byte & 0x0F;
						Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

						Register_Value_1 = Interpreter_Registers_V[Register_Index_1];
						Register_Value_2 = Interpreter_Registers_V[Register_Index_2];
						Interpreter_Registers_V[Register_Index_1] = Register_Value_2 - Register_Value_1;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SUBN V%01X (= 0x%02X), V%01X (= 0x%02X).", Register_Index_1, Register_Value_1, Register_Index_2, Register_Value_2);

						// Set VF register if borrow is clear
						if (Register_Value_2 >= Register_Value_1) Interpreter_Registers_V[15] = 1;
						else Interpreter_Registers_V[15] = 0;
						break;
					}

					// SHL Vx
					case 0x0E:
					{
						unsigned char Register_Index, Value, Is_Bit_Shifted_Out;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;

						Value = Interpreter_Registers_V[Register_Index];
						if (Value & 0x80) Is_Bit_Shifted_Out = 1; // Set VF if the Vx most significant bit is set
						else Is_Bit_Shifted_Out = 0;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SHL V%01X (= 0x%02X).", Register_Index, Value);
						Value <<= 1;
						Interpreter_Registers_V[Register_Index] = Value;

						// Set VF at the end, in case VF was the output register just before
						Interpreter_Registers_V[15] = Is_Bit_Shifted_Out;
						break;
					}

					default:
						goto Invalid_Instruction;
				}
				break;
			}

			// SNE Vx, Vy
			case 0x90:
			{
				unsigned char Register_Index_1, Register_Index_2, Register_Value_1, Register_Value_2;

				// Extract the operands
				Register_Index_1 = Instruction_High_Byte & 0x0F;
				Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

				if (Interpreter_Registers_V[Register_Index_1] != Interpreter_Registers_V[Register_Index_2]) Interpreter_Register_PC += 2;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SNE V%01X (= 0x%02X), V%01X (= 0x%02X).", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
				break;
			}

			// LD I, addr
			case 0xA0:
				Interpreter_Register_I = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				Interpreter_Register_I |= Instruction_Low_Byte;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD I, 0x%03X.", Interpreter_Register_I);
				break;

			// JP V0, nnn
			case 0xB0:
			{
				unsigned short Address;

				// Extract the operands
				Address = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;

				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "JP V0 (= 0x%02X), 0x%03X with PC = 0x%03X.", Interpreter_Registers_V[0], Address, Interpreter_Register_PC);
				Interpreter_Register_PC = Interpreter_Registers_V[0] + Address;
				Interpreter_Register_PC &= 0x0FFF; // Make sure the address does not cross the 4KB boundary
				goto Next_Instruction; // Bypass PC incrementation
			}

			// RND Vx, byte
			case 0xC0:
			{
				unsigned char Register_Index, Is_Least_Significant_Bit_Set;

				// Extract the operands
				Register_Index = Instruction_High_Byte & 0x0F;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "RND V%01X, 0x%02X.", Register_Index, Instruction_Low_Byte);

				// Use a 8-bit Galois LFSR to generate good pseudo-random numbers (see https://en.wikipedia.org/wiki/Linear-feedback_shift_register#Galois_LFSRs)
				if (Interpreter_Random_Seed & 0x01) Is_Least_Significant_Bit_Set = 1;
				else Is_Least_Significant_Bit_Set = 0;
				Interpreter_Random_Seed >>= 1;
				if (Is_Least_Significant_Bit_Set) Interpreter_Random_Seed ^= 0xB4;

				// The random byte is ANDed with a mask provided in the instruction
				Interpreter_Registers_V[Register_Index] = Interpreter_Random_Seed & Instruction_Low_Byte;
				break;
			}

			// DRW Vx, Vy, nibble
			case 0xD0:
			{
				unsigned char Register_Index_1, Register_Index_2, *Pointer_Sprite, *Pointer_Display, *Pointer_Display_Left_Over_Pixels, Sprite_Size, Shift_Offset, Row, Sprite_Row, Sprite_Column, Sprite_Byte, Previous_Byte_Value, Rendered_Byte_Value, Columns_Count_Byte, Is_Collision_Detected = 0;

				// Extract the operands
				Register_Index_1 = Instruction_High_Byte & 0x0F;
				Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;
				Sprite_Size = Instruction_Low_Byte & 0x0F;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "DRW V%01X (= 0x%02X), V%01X (= 0x%02X), %d with I = 0x%03X.", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2], Sprite_Size, Interpreter_Register_I);

				// Retrieve the sprite displaying coordinates
				Sprite_Column = Interpreter_Registers_V[Register_Index_1];
				Sprite_Row = Interpreter_Registers_V[Register_Index_2];

				if (Is_Super_Chip_8_Mode)
				{
					Sprite_Column &= INTERPRETER_DISPLAY_COLUMNS_COUNT_SUPER_CHIP_8 - 1;
					Sprite_Row &= INTERPRETER_DISPLAY_ROWS_COUNT_SUPER_CHIP_8 - 1;
				}
				else
				{
					Sprite_Column &= INTERPRETER_DISPLAY_COLUMNS_COUNT_CHIP_8 - 1;
					Sprite_Row &= INTERPRETER_DISPLAY_ROWS_COUNT_CHIP_8 - 1;
				}
				Pointer_Sprite = &Shared_Buffers.Interpreter_Memory[Interpreter_Register_I];

				// Determine the amount of bits the sprite must be shifted (in case it would not fit entirely in a frame buffer byte)
				Shift_Offset = Sprite_Column & 0x07;
				Columns_Count_Byte = Display_Columns_Count / 8; // Cache the amount of columns in bytes (instead of pixels)
				Sprite_Column /= 8; // A byte stores 8 horizontal pixels, so cache the column coordinate converted to bytes instead of pixels
				Row = Sprite_Row;

				// Find the location in the frame buffer where to draw the sprite
				Pointer_Display = &Shared_Buffer_Display[(Row * Columns_Count_Byte) + Sprite_Column];

				// Render the sprite
				while (Sprite_Size > 0) // The Sprite_Size value has already been checked and can't be invalid
				{
					// Cache the sprite data
					Sprite_Byte = *Pointer_Sprite;

					// Directly display the sprite byte if it is aligned with a frame buffer byte
					if (Shift_Offset == 0)
					{
						Previous_Byte_Value = *Pointer_Display;
						Rendered_Byte_Value = Previous_Byte_Value ^ Sprite_Byte;
						*Pointer_Display = Rendered_Byte_Value;
						if ((Rendered_Byte_Value & Previous_Byte_Value) != Previous_Byte_Value) Is_Collision_Detected = 1; // A collision occurred if any lighted pixel was turned off
					}
					// The sprite is not aligned, draw a part on the first frame buffer byte, and draw the remaining part on the following frame buffer byte
					else
					{
						// Render the first part of the sprite
						Previous_Byte_Value = *Pointer_Display;
						Rendered_Byte_Value = Previous_Byte_Value ^ (Sprite_Byte >> Shift_Offset);
						*Pointer_Display = Rendered_Byte_Value;
						if ((Rendered_Byte_Value & Previous_Byte_Value) != Previous_Byte_Value) Is_Collision_Detected = 1; // A collision occurred if any lighted pixel was turned off

						// If the right side of the display is reached, the left over pixels must wrap around the left side of the display
						if (Sprite_Column >= (Columns_Count_Byte - 1)) Pointer_Display_Left_Over_Pixels = Pointer_Display - (Columns_Count_Byte - 1);
						// Otherwise, just render the left over pixels to the following byte location
						else Pointer_Display_Left_Over_Pixels = Pointer_Display + 1;
						Previous_Byte_Value = *Pointer_Display_Left_Over_Pixels;
						Rendered_Byte_Value = Previous_Byte_Value ^ ((unsigned char) (Sprite_Byte << (8 - Shift_Offset)));
						*Pointer_Display_Left_Over_Pixels = Rendered_Byte_Value;
						if ((Rendered_Byte_Value & Previous_Byte_Value) != Previous_Byte_Value) Is_Collision_Detected = 1; // A collision occurred if any lighted pixel was turned off
					}

					// Draw the next sprite line
					Pointer_Sprite++;
					Sprite_Size--;

					// If the bottom side of the display is reached, the sprite must wrap over the top of the display
					Row++;
					if (Row >= Display_Rows_Count)
					{
						Row = 0;
						// Subtract all rows but the last one, keeping only the "column" component of the display address
						Pointer_Display -= (Display_Rows_Count - 1) * Columns_Count_Byte;
					}
					// Go to the next line
					else Pointer_Display += Columns_Count_Byte;
				}

				// The frame buffer must be transferred to the display at 60Hz
				if (Interpreter_Is_Fast_Rendering_Enabled) Is_Rendering_Needed = 1; // This variable will never be set if Interpreter_Is_Fast_Rendering_Enabled is false
				// Transfer the frame buffer at each DRW call because some games use this as a delay
				else
				{
					if (Is_Super_Chip_8_Mode) DisplayDrawFullSizeBuffer(Shared_Buffer_Display);
					else DisplayDrawHalfSizeBuffer(Shared_Buffer_Display);
				}

				// Set register VF if at least one already lighted pixel has been turned off
				Interpreter_Registers_V[15] = Is_Collision_Detected;
				break;
			}

			case 0xE0:
			{
				unsigned char Register_Index, Key_Code, Key_Mask, Is_Key_Pressed;

				// Retrieve whether the expected key is pressed, this is common to all 0xExxx instructions
				Register_Index = Instruction_High_Byte & 0x0F; // Retrieve the register containing the key code
				Key_Code = Interpreter_Registers_V[Register_Index]; // Get the key code
				Key_Mask = Interpreter_Keys_Table_From_Interpreter[Key_Code]; // Get the bit mask corresponding to the physical key switch
				if (KeyboardReadKeysMask() & Key_Mask) Is_Key_Pressed = 1;
				else Is_Key_Pressed = 0;

				switch (Instruction_Low_Byte)
				{
					// SKP Vx
					case 0x9E:
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SKP V%01X (= 0x%02X), is key pressed = %u.", Register_Index, Key_Code, Is_Key_Pressed);
						if (Is_Key_Pressed) Interpreter_Register_PC += 2;
						break;

					// SKNP Vx
					case 0xA1:
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SKNP V%01X (= 0x%02X), is key pressed = %u.", Register_Index, Key_Code, Is_Key_Pressed);
						if (!Is_Key_Pressed) Interpreter_Register_PC += 2;
						break;

					default:
						goto Invalid_Instruction;
				}
				break;
			}

			case 0xF0:
			{
				switch (Instruction_Low_Byte)
				{
					// LD Vx, DT
					case 0x07:
					{
						unsigned char Register_Index, Delay;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;

						// Retrieve the timer value
						if (T6CONbits.ON == 0) Delay = 0; // When the timer has overflowed, the counter register is reset and the ON bit is cleared
						else Delay = T6PR - T6TMR; // The hardware timer is incrementing while the Chip-8 timer is expected to decrement
						Interpreter_Registers_V[Register_Index] = Delay;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, DT (= 0x%02X).", Register_Index, Delay);
						break;
					}

					// LD Vx, K
					case 0x0A:
					{
						unsigned char Register_Index, Key_Mask, i;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, K.", Register_Index);

						// Stop the execution until a key is pressed
						do
						{
							// Exit when the menu key is pressed
							if (KeyboardIsMenuKeyPressed()) return 0;

							// Also run the rendering loop in case a DRW instruction was issued just before calling this instruction, and the tick was not present (so the frame buffer could not be transferred to the display before blocking in this instruction)
							if (Is_Rendering_Needed && NCO_60HZ_TICK())
							{
								// Display the picture according to the emulation mode display
								if (Is_Super_Chip_8_Mode) DisplayDrawFullSizeBuffer(Shared_Buffer_Display);
								else DisplayDrawHalfSizeBuffer(Shared_Buffer_Display);

								Is_Rendering_Needed = 0;
								NCO_CLEAR_TICK_INTERRUPT_FLAG(); // The interrupt flag must be manually cleared
							}

							Key_Mask = KeyboardReadKeysMask();
						} while (Key_Mask == 0);

						// Retrieve the corresponding Chip-8 key (the first one matching)
						for (i = 0; i < 8; i++)
						{
							if ((1 << i) & Key_Mask)
							{
								Interpreter_Registers_V[Register_Index] = Interpreter_Keys_Table_From_Console[i];
								break;
							}
						}
						break;
					}

					// LD DT, Vx
					case 0x15:
					{
						unsigned char Register_Index, Delay;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;

						// Retrieve the register value
						Delay = Interpreter_Registers_V[Register_Index];
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD DT, V%01X (= 0x%02X).", Register_Index, Delay);

						// Start the timer
						T6CONbits.ON = 0; // Stop the timer in case it is already running
						T6PR = Delay; // The timer will stop incrementing when the TMR register value will become equal to the PR register value
						T6TMR = 0;
						T6CONbits.ON = 1;
						break;
					}

					// LD ST, Vx
					case 0x18:
					{
						unsigned char Register_Index;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD ST, V%01X (= 0x%02X).", Register_Index, Interpreter_Registers_V[Register_Index]);

						SoundPlay(Interpreter_Registers_V[Register_Index]);
						break;
					}

					// ADD I, Vx
					case 0x1E:
					{
						unsigned char Register_Index;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;

						// Sum the values
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "ADD I (= 0x%04X), V%01X (= 0x%02X).", Interpreter_Register_I, Register_Index, Interpreter_Registers_V[Register_Index]);
						Interpreter_Register_I += Interpreter_Registers_V[Register_Index];
						break;
					}

					// LD F, Vx
					case 0x29:
					{
						unsigned char Register_Index, Digit;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD F, V%01X (= 0x%02X).", Register_Index, Interpreter_Registers_V[Register_Index]);

						// Retrieve the digit value
						Digit = Interpreter_Registers_V[Register_Index] & 0x0F; // The allowed digit values are 0 to 0xF

						// Each digit is stored on 5 bytes, and the first digit starts at the memory offset 0
						Interpreter_Register_I = Digit * 5;
						break;
					}

					// LD B, Vx
					case 0x33:
					{
						unsigned char Register_Index, Value, Digit, i, Divider;
						unsigned short Register_I;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;
						Value = Interpreter_Registers_V[Register_Index];
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD B, V%01X (= 0x%02X).", Register_Index, Value);
						Register_I = Interpreter_Register_I & 0x0FFF; // The I register value must not be changed at the end of this instruction execution (also make sure to prevent any interpreter memory access overflow)

						// Convert the and store it to the memory
						Divider = 100;
						for (i = 0; i < 3; i++)
						{
							// Extract the next digit
							if (Value >= Divider)
							{
								Digit = Value / Divider;
								Value -= Digit * Divider; // Subtract only the hundreds to the value
							}
							else Digit = 0;

							// Store the value in the memory pointed by the I register, making sure to prevent any interpreter memory access overflow
							Shared_Buffers.Interpreter_Memory[Register_I] = Digit;
							Register_I = (Register_I + 1) & 0x0FFF;

							// Extract next rank digit
							Divider /= 10;
						}
						break;
					}

					// LD [I], Vx
					case 0x55:
					{
						unsigned char Last_Register_Index, i;

						// Extract the operands
						Last_Register_Index = Instruction_High_Byte & 0x0F;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD [I] (= 0x%03X), V%01X.", Interpreter_Register_I, Last_Register_Index);

						// Store the requested amount of registers to the address pointed by the I register
						for (i = 0; i <= Last_Register_Index; i++)
						{
							Shared_Buffers.Interpreter_Memory[Interpreter_Register_I] = Interpreter_Registers_V[i];
							Interpreter_Register_I = (Interpreter_Register_I + 1) & 0x0FFF; // Avoid overflowing the interpreter memory buffer
						}
						break;
					}

					// LD Vx, [I]
					case 0x65:
					{
						unsigned char Last_Register_Index, i;

						// Extract the operands
						Last_Register_Index = Instruction_High_Byte & 0x0F;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, [I] (= 0x%03X).", Last_Register_Index, Interpreter_Register_I);

						// Store the requested amount of registers to the address pointed by the I register
						for (i = 0; i <= Last_Register_Index; i++)
						{
							Interpreter_Registers_V[i] = Shared_Buffers.Interpreter_Memory[Interpreter_Register_I];
							Interpreter_Register_I = (Interpreter_Register_I + 1) & 0x0FFF; // Avoid overflowing the interpreter memory buffer
						}
						break;
					}

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

Next_Instruction:
		// Transfer the frame buffer to the display only when it has changed and not faster than 60 times per second
		if (Is_Rendering_Needed && NCO_60HZ_TICK())
		{
			// Display the picture according to the emulation mode display
			if (Is_Super_Chip_8_Mode) DisplayDrawFullSizeBuffer(Shared_Buffer_Display);
			else DisplayDrawHalfSizeBuffer(Shared_Buffer_Display);

			Is_Rendering_Needed = 0;
			NCO_CLEAR_TICK_INTERRUPT_FLAG(); // The interrupt flag must be manually cleared
		}

		#if INTERPRETER_IS_DEBUGGER_ENABLED == 1
		{
			unsigned char i;

			// Display all registers
			SerialPortWriteString("Registers :\r\n");
			printf("PC=0x%04X, I=0x%04X, SP=0x%02X\r\n", Interpreter_Register_PC, Interpreter_Register_I, Interpreter_Register_SP);
			for (i = 0; i < INTERPRETER_REGISTERS_V_COUNT; i++) printf("V%01X=0x%02X, ", i, Interpreter_Registers_V[i]);
			SerialPortWriteString("\r\n");

			// Wait for user input
			SerialPortWriteString("Press 's' to continue.\r\n");
			while (SerialPortReadByte() != 's');
		}
		#endif
		continue; // Useless here but the compiler does not accept a label followed by no code
	}

Invalid_Instruction:
	{
		char String_Message[70];

		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Invalid instruction 0x%02X%02X at address 0x%03X. Stopping interpreter.", Instruction_High_Byte, Instruction_Low_Byte, Interpreter_Register_PC);
		memset(Shared_Buffer_Display, 0, sizeof(Shared_Buffer_Display));
		snprintf(String_Message, sizeof(String_Message), "Invalid instruction\n0x%02X%02X at address\n0x%03X.\n\n\nPress Menu to exit.", Instruction_High_Byte, Instruction_Low_Byte, Interpreter_Register_PC);
		DisplayDrawTextMessage(Shared_Buffer_Display, "Error", String_Message);
		while (!KeyboardIsMenuKeyPressed());
	}

	return 1;
}
