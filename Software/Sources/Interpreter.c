/** @file Interpreter.c
 * See Interpreter.h for description.
 * @author Adrien RICCIARDI
 */
#include <Display.h>
#include <Interpreter.h>
#include <Keyboard.h>
#include <Serial_Port.h>
#include <Sound.h>
#include <xc.h>

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

/** The Chip-8 default program entry point. */
#define INTERPRETER_PROGRAM_ENTRY_POINT 0x200

#define INTERPRETER_DISPLAY_COLUMNS_COUNT_CHIP_8 64
#define INTERPRETER_DISPLAY_ROWS_COUNT_CHIP_8 32

#define INTERPRETER_DISPLAY_COLUMNS_COUNT_SUPER_CHIP_8 128
#define INTERPRETER_DISPLAY_ROWS_COUNT_SUPER_CHIP_8 64

/** The total amount of keys supported by Chip-8. */
#define INTERPRETER_KEYS_COUNT 16

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

/** The buffer used to render the interpreter frames. It can hold an entire SuperChip-8 buffer. */
static unsigned char Interpreter_Frame_Buffer[DISPLAY_COLUMNS_COUNT * DISPLAY_ROWS_COUNT / 8] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFE, 0x00,
	0x01, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0x80,
	0x01, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xFF, 0xFF, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0xFF, 0x80, 0x00, 0x01, 0xF0, 0x03, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0xFF, 0xC0, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0xFF, 0xE0, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0xF0, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x70, 0x00, 0x01, 0xF0, 0x03, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x70, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x70, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x7F, 0xFF, 0xC0, 0x78, 0x0F, 0x00, 0xF0, 0xFF, 0x80, 0xFF, 0xFF, 0xC0,
	0x01, 0xE0, 0x00, 0x00, 0x7F, 0xFF, 0xC0, 0x78, 0x0F, 0x7F, 0xE0, 0xFF, 0x81, 0xF0, 0x03, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x7F, 0xFF, 0xC0, 0x78, 0x0F, 0x7F, 0xE0, 0xFF, 0x81, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x7F, 0x80, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x01, 0xE0, 0x01, 0xE0,
	0x01, 0xE0, 0x00, 0x00, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x01, 0xF0, 0x03, 0xE0,
	0x01, 0xFF, 0xFF, 0xC0, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xFF, 0xFF, 0xE0, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xC0,
	0x01, 0xFF, 0xFF, 0xE0, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xFF, 0x80,
	0x00, 0xFF, 0xFF, 0xC0, 0x78, 0x01, 0xC0, 0x78, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xFE, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xFF, 0xE1, 0x80, 0xC1, 0xFC, 0x0F, 0xFE, 0x18, 0x01, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xFF, 0xE1, 0xC0, 0xC3, 0xFE, 0x0F, 0xFE, 0x18, 0x01, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0xE0, 0xC3, 0x07, 0x0C, 0x06, 0x18, 0x01, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0xF0, 0xC3, 0x00, 0x0C, 0x06, 0x18, 0x01, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0xB8, 0xC3, 0xFE, 0x0C, 0x06, 0x18, 0x01, 0xBE, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0x9C, 0xC1, 0xFF, 0x0C, 0x06, 0x18, 0x01, 0xBE, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0x87, 0xC0, 0x03, 0x0C, 0x06, 0x18, 0x01, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x60, 0x00, 0xC0, 0x61, 0x83, 0xC3, 0x03, 0x0C, 0x06, 0x18, 0x01, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x7F, 0xF0, 0xFF, 0xE1, 0x81, 0xC3, 0xFF, 0x0F, 0xFE, 0x1F, 0xF1, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x7F, 0xF0, 0xFF, 0xE1, 0x80, 0xC1, 0xFE, 0x0F, 0xFE, 0x1F, 0xF1, 0xFF, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/** Hold a look-up table returning the physical switch code corresponding to a logical Chip-8 key code. This allows to remap Chip-8 program keys to the physical mapping of the console. */
static unsigned char Interpreter_Keys_Table[INTERPRETER_KEYS_COUNT] = { 0x00, 0x00, KEYBOARD_KEY_UP, 0x00, KEYBOARD_KEY_DOWN, 0x00, KEYBOARD_KEY_LEFT, 0x00, KEYBOARD_KEY_RIGHT }; // TEST mapping

//-------------------------------------------------------------------------------------------------
// Public functions
//-------------------------------------------------------------------------------------------------
void InterpreterInitialize(void)
{
	// Use timer 6 as the Chip-8 delay timer
	T6CLK = 0x09; // Clock the timer by the NCO module
	T6HLT = 0xA8; // Prescaler output is synchronized with Fosc/4, also synchronize the ON bit with the timer clock input, select the one-shot mode with one-shot operation and software start
	T6CON = 0; // Do not enable the timer yet, do not enable any prescaler or postscaler (the timer is already clocked at the desired frequency)

	DisplayDrawFullSizeBuffer(Interpreter_Frame_Buffer);
	__delay_ms(2000);
}

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

	/*Interpreter_Memory[0x200] = 0x60; Interpreter_Memory[0x201] = 60; // LD V0, 0
	Interpreter_Memory[0x202] = 0x61; Interpreter_Memory[0x203] = 5; // LD V1, 0
	Interpreter_Memory[0x204] = 0xA0; Interpreter_Memory[0x205] = 0; // LD I, 0
	Interpreter_Memory[0x206] = 0xD0; Interpreter_Memory[0x207] = 0x15; // LD I, 0
	Interpreter_Memory[0x208] = 0x12; Interpreter_Memory[0x209] = 0x08; // JP 0x208*/

	// Load the file
	if (FATReadFile(Pointer_File_Information, &Interpreter_Memory[INTERPRETER_PROGRAM_ENTRY_POINT], INTERPRETER_MEMORY_SIZE - INTERPRETER_PROGRAM_ENTRY_POINT) != 0)
	{
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Error : failed to load the program from the file named \"%s\".\r\n", Pointer_File_Information->String_Short_Name);
		return 1;
	}

	// Configure the registers for the program execution
	Interpreter_Register_PC = INTERPRETER_PROGRAM_ENTRY_POINT; // The default entry point
	Interpreter_Register_SP = 0; // Clear the stack

	// Clear the frame buffer
	memset(Interpreter_Frame_Buffer, 0, sizeof(Interpreter_Frame_Buffer));

	return 0;
}

unsigned char InterpreterRunProgram(void)
{
	unsigned char Display_Rows_Count, Display_Columns_Count, Instruction_High_Byte, Instruction_Low_Byte, Is_Super_Chip_8_Mode = 0; // TODO Initialize Is_Super_Chip_8_Mode according to the program to run

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
	SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Emulation mode : %s, display columns count = %d, display rows count = %d.\r\n", Is_Super_Chip_8_Mode ? "SuperChip-8" : "Chip-8", Display_Columns_Count, Display_Rows_Count);

	while (1)
	{
		// Make sure only the instruction address can't go out the array bounds
		Interpreter_Register_PC &= 0x0FFF;

		// Fetch the next instruction
		Instruction_High_Byte = Interpreter_Memory[Interpreter_Register_PC];
		Instruction_Low_Byte = Interpreter_Memory[Interpreter_Register_PC + 1];
		SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Fetching instruction at address 0x%03X : 0x%02X 0x%02X.\r\n", Interpreter_Register_PC, Instruction_High_Byte, Instruction_Low_Byte);

		// Decode and execute the instruction
		switch (Instruction_High_Byte & 0xF0)
		{
			case 0x00:
			{
				switch (Instruction_Low_Byte)
				{
					// CLS
					case 0xE0:
						memset(Interpreter_Frame_Buffer, 0, sizeof(Interpreter_Frame_Buffer));
						DisplayDrawFullSizeBuffer(Interpreter_Frame_Buffer);
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "CLS.\r\n");
						break;

					// RET
					case 0xEE:
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "RET (SP = %d).\r\n", Interpreter_Register_SP);
						// Make sure there is a valid address on the stack
						if (Interpreter_Register_SP == 0)
						{
							SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Virtual program error : stack underflow. Stopping interpreter.\r\n");
							while (1);
						}
						Interpreter_Register_SP--; // The CALL instruction increments the stack pointer after pushing, so the RET instruction needs to decrement the stack pointer before popping
						Interpreter_Register_PC = Interpreter_Stack[Interpreter_Register_SP];
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
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "JP 0x%03X.\r\n", Interpreter_Register_PC);
				continue; // Bypass PC incrementation

			// CALL addr
			case 0x20:
			{
				unsigned short Address;

				// Extract the operands
				Address = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				Address |= Instruction_Low_Byte; // The address will by default be on 12 bits due to the instruction encoding, so no need to check for an overflow
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "CALL 0x%03X (SP = %d).\r\n", Address, Interpreter_Register_SP);

				// Make sure there is still room on the stack
				if (Interpreter_Register_SP >= INTERPRETER_STACK_SIZE)
				{
					SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Virtual program error : stack overflow. Stopping interpreter.\r\n");
					while (1);
				}
				Interpreter_Stack[Interpreter_Register_SP] = Interpreter_Register_PC + 2; // Store the address of the instruction following this one
				Interpreter_Register_SP++;

				// Jump to the function entry point
				Interpreter_Register_PC = Address;
				continue; // Bypass PC incrementation
			}

			// SE Vx, byte
			case 0x30:
			{
				unsigned char Register_Index;

				// Extract the operands
				Register_Index = Instruction_High_Byte & 0x0F;

				// Skip the next instruction if Vx is equal to the immediate value
				if (Interpreter_Registers_V[Register_Index] == Instruction_Low_Byte) Interpreter_Register_PC += 2;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SE V%01X (= 0x%02X), 0x%02X.\r\n", Register_Index, Interpreter_Registers_V[Register_Index], Instruction_Low_Byte);
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
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SNE V%01X (= 0x%02X), 0x%02X.\r\n", Register_Index, Interpreter_Registers_V[Register_Index], Instruction_Low_Byte);
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
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SE V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
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
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, 0x%02X.\r\n", Register_Index, Immediate_Value);
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
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "ADD V%01X, 0x%02X.\r\n", Register_Index, Immediate_Value);
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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, V%01X (= 0x%02X).\r\n", Register_Index_1, Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
						break;
					}

					// OR Vx, Vy
					case 0x01:
					{
						unsigned char Register_Index_1, Register_Index_2;

						// Extract the operands
						Register_Index_1 = Instruction_High_Byte & 0x0F;
						Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;

						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "OR V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
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

						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "AND V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
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

						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "XOR V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "ADD V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SUB V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Register_Index_1, Register_Value_1, Register_Index_2, Register_Value_2);

						// Set VF register if borrow is clear
						if (Instruction_High_Byte > Instruction_Low_Byte) Interpreter_Registers_V[15] = 1;
						else Interpreter_Registers_V[15] = 0;
						break;
					}

					// SHR Vx
					case 0x06:
					{
						unsigned char Register_Index, Value;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;

						Value = Interpreter_Registers_V[Register_Index];
						if (Value & 0x01) Interpreter_Registers_V[15] = 1; // Set VF if the Vx least significant bit is set
						else Interpreter_Registers_V[Register_Index] = 0;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SHR V%01X (= 0x%02X).\r\n", Register_Index, Value);
						Value >>= 1;
						Interpreter_Registers_V[Register_Index] = Value;
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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SUBN V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Register_Index_1, Register_Value_1, Register_Index_2, Register_Value_2);
						// Set VF register if borrow is clear
						if (Instruction_Low_Byte > Register_Value_1) Interpreter_Registers_V[15] = 1;
						else Interpreter_Registers_V[15] = 0;
						break;
					}

					// SHL Vx
					case 0x0E:
					{
						unsigned char Register_Index, Value;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;

						Value = Interpreter_Registers_V[Register_Index];
						if (Value & 0x80) Interpreter_Registers_V[15] = 1; // Set VF if the Vx most significant bit is set
						else Interpreter_Registers_V[15] = 0;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SHL V%01X (= 0x%02X).\r\n", Register_Index, Value);
						Value <<= 1;
						Interpreter_Registers_V[Register_Index] = Value;
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
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SNE V%01X (= 0x%02X), V%01X (= 0x%02X).\r\n", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2]);
				break;
			}

			// LD I, addr
			case 0xA0:
				Interpreter_Register_I = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;
				Interpreter_Register_I |= Instruction_Low_Byte;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD I, 0x%03X.\r\n", Interpreter_Register_I);
				break;

			// JP V0, nnn
			case 0xB0:
			{
				unsigned short Address;

				// Extract the operands
				Address = (unsigned short) (Instruction_High_Byte & 0x0F) << 8;

				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "JP V0 (= 0x%02X), 0x%03X with PC = 0x%03X.\r\n", Interpreter_Registers_V[0], Address, Interpreter_Register_PC);
				Interpreter_Register_PC = Interpreter_Registers_V[0] + Address;
				Interpreter_Register_PC &= 0x0FFF; // Make sure the address does not cross the 4KB boundary
				continue; // Bypass PC incrementation
			}

			// RND Vx, byte
			case 0xC0:
				// TODO
				break;

			// DRW Vx, Vy, nibble
			case 0xD0:
			{
				unsigned char Register_Index_1, Register_Index_2, *Pointer_Sprite, *Pointer_Display, Sprite_Size, Shift_Offset, Column, Row, Sprite_Row, Sprite_Column, Byte, Columns_Count_Byte;

				// Extract the operands
				Register_Index_1 = Instruction_High_Byte & 0x0F;
				Register_Index_2 = (Instruction_Low_Byte >> 4) & 0x0F;
				Sprite_Size = Instruction_Low_Byte & 0x0F;
				SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "DRW V%01X (= 0x%02X), V%01X (= 0x%02X), %d with I = 0x%03X.\r\n", Register_Index_1, Interpreter_Registers_V[Register_Index_1], Register_Index_2, Interpreter_Registers_V[Register_Index_2], Sprite_Size, Interpreter_Register_I);

				// Retrieve the sprite displaying coordinates and make sure they do not cross the display boundaries (screen wraping is not handled for now)
				Sprite_Column = Interpreter_Registers_V[Register_Index_1];
				Sprite_Row = Interpreter_Registers_V[Register_Index_2];
				if (Is_Super_Chip_8_Mode)
				{
					Sprite_Column &= 0x7F; // Limit to 128 horizontal values in SuperChip-8 mode
					Sprite_Row &= 0x3F; // Limit to 64 vertical values in SuperChip-8 mode
				}
				else
				{
					Sprite_Column &= 0x3F; // Limit to 64 horizontal values in Chip-8 mode
					Sprite_Row &= 0x1F; // Limit to 32 vertical values in Chip-8 mode
				}
				Pointer_Sprite = &Interpreter_Memory[Interpreter_Register_I];

				// Determine the amount of bits the sprite must be shifted (in case it would not fit entirely in a frame buffer byte)
				Shift_Offset = Sprite_Column & 0x07;
				Columns_Count_Byte = Display_Columns_Count / 8; // Cache the amount of columns in bytes (instead of pixels)
				Sprite_Column /= 8; // A byte stores 8 horizontal pixels, so cache the column coordinate converted to bytes instead of pixels
				for (Row = Sprite_Row; Row < Display_Rows_Count; Row++)
				{
					// Stop when the requested amount of sprite bytes has been displayed
					if (Sprite_Size == 0) break; // Start with this check in case the specified sprite size is 0, so the loop immediately exits

					// Find the location in the frame buffer where to draw the sprite
					Pointer_Display = &Interpreter_Frame_Buffer[(Row * Columns_Count_Byte) + Sprite_Column];
					Byte = *Pointer_Sprite;

					// Directly display the sprite byte if it is aligned with a frame buffer byte
					if (Shift_Offset == 0) *Pointer_Display ^= Byte;
					// The sprite is not aligned, draw a part on the first frame buffer byte, and draw the remaining part on the following frame buffer byte
					else
					{
						*Pointer_Display ^= Byte >> Shift_Offset;
						if (Sprite_Column < (Columns_Count_Byte - 1)) *(Pointer_Display + 1) ^= (unsigned char) (Byte << (8 - Shift_Offset));
					}

					// Draw the next sprite line
					Pointer_Sprite++;
					Sprite_Size--;
				}

				// Display the picture according to the emulation mode display
				if (Is_Super_Chip_8_Mode) DisplayDrawFullSizeBuffer(Interpreter_Frame_Buffer);
				else DisplayDrawHalfSizeBuffer(Interpreter_Frame_Buffer);
				break;
			}

			case 0xE0:
			{
				unsigned char Register_Index, Key_Code, Key_Mask, Is_Key_Pressed;

				// Retrieve whether the expected key is pressed, this is common to all 0xExxx instructions
				Register_Index = Instruction_High_Byte & 0x0F; // Retrieve the register containing the key code
				Key_Code = Interpreter_Registers_V[Register_Index]; // Get the key code
				Key_Mask = Interpreter_Keys_Table[Key_Code]; // Get the bit mask corresponding to the physical key switch
				if (KeyboardReadKeysMask() & Key_Mask) Is_Key_Pressed = 1;
				else Is_Key_Pressed = 0;

				switch (Instruction_Low_Byte)
				{
					// SKP Vx
					case 0x9E:
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SKP V%01X (= 0x%02X).\r\n", Register_Index, Key_Code);
						if (Is_Key_Pressed) Interpreter_Register_PC += 2;
						break;

					// SKNP Vx
					case 0xA1:
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "SKNP V%01X (= 0x%02X).\r\n", Register_Index, Key_Code);
						if (!Is_Key_Pressed) Interpreter_Register_PC += 2;
						break;
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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, DT (= 0x%02X).\r\n", Register_Index, Delay);
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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD DT, V%01X (= 0x%02X).\r\n", Register_Index, Delay);

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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD ST, V%01X (= 0x%02X).\r\n", Register_Index, Interpreter_Registers_V[Register_Index]);

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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "ADD I (= 0x%04X), V%01X (= 0x%02X).\r\n", Interpreter_Register_I, Register_Index, Interpreter_Registers_V[Register_Index]);
						Interpreter_Register_I += Interpreter_Registers_V[Register_Index];
						break;
					}

					// LD F, Vx
					case 0x29:
					{
						unsigned char Register_Index, Digit;

						// Extract the operands
						Register_Index = Instruction_High_Byte & 0x0F;
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD F, V%01X (= 0x%02X).\r\n", Register_Index, Interpreter_Registers_V[Register_Index]);

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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD B, V%01X (= 0x%02X).\r\n", Register_Index, Value);
						Register_I = Interpreter_Register_I & 0x0FFF; // The I register value must not be changed at the end of this instruction execution (also make sure to prevent any interpreter memory access overflow)

						// Convert the and store it to the memory
						Divider = 100;
						for (i = 0; i < 3; i++)
						{
							// Extract the next digit
							if (Value >= Divider)
							{
								Digit = Value / Divider;
								Value -= Digit * Divider; // Subtract only the hudreds to the value
							}
							else Digit = 0;

							// Store the value in the memory pointed by the I register, making sure to prevent any interpreter memory access overflow
							Interpreter_Memory[Register_I] = Digit;
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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD [I] (= 0x%03X), V%01X.\r\n", Interpreter_Register_I, Last_Register_Index);

						// Store the requested amount of registers to the address pointed by the I register
						for (i = 0; i <= Last_Register_Index; i++)
						{
							Interpreter_Memory[Interpreter_Register_I] = Interpreter_Registers_V[i];
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
						SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "LD V%01X, [I] (= 0x%03X).\r\n", Last_Register_Index, Interpreter_Register_I);

						// Store the requested amount of registers to the address pointed by the I register
						for (i = 0; i <= Last_Register_Index; i++)
						{
							Interpreter_Registers_V[i] = Interpreter_Memory[Interpreter_Register_I];
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
	}

Invalid_Instruction:
	// Always display this error, even if logging is not enabled
	SERIAL_PORT_LOG(INTERPRETER_IS_LOGGING_ENABLED, "Invalid instruction 0x%02X%02X at address 0x%03X. Stopping interpreter.\r\n", Instruction_High_Byte, Instruction_Low_Byte, Interpreter_Register_PC);
	while (1);

	return 0;
}
