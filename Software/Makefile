PATH_BINARIES = $(shell realpath .)/Binaries
PATH_INCLUDES = $(shell realpath .)/Includes
PATH_OBJECTS = $(shell realpath .)/Objects
PATH_SOURCES = $(shell realpath .)/Sources

FIRMWARE_VERSION=1.1

BINARY_NAME = Chip8_Console_Firmware.hex
SOURCES = \
	$(PATH_SOURCES)/Battery.c \
	$(PATH_SOURCES)/Display.c \
	$(PATH_SOURCES)/EEPROM.c \
	$(PATH_SOURCES)/FAT.c \
	$(PATH_SOURCES)/INI_Parser.c \
	$(PATH_SOURCES)/Interpreter.c \
	$(PATH_SOURCES)/LED.c \
	$(PATH_SOURCES)/Log.c \
	$(PATH_SOURCES)/Keyboard.c \
	$(PATH_SOURCES)/Main.c \
	$(PATH_SOURCES)/MBR.c \
	$(PATH_SOURCES)/NCO.c \
	$(PATH_SOURCES)/SD_Card.c \
	$(PATH_SOURCES)/Serial_Port.c \
	$(PATH_SOURCES)/Shared_Buffer.c \
	$(PATH_SOURCES)/Sound.c \
	$(PATH_SOURCES)/SPI.c

CC = xc8-cc
# The -Wa,-a argument tells the assembler to generate a listing file
# The -mundefints=ignore argument disable the compiler warning messages about the unassigned interrupt handlers in the vector table
CFLAGS = -mcpu=18F27K42 -D_XTAL_FREQ=64000000 -muse-ivt -mundefints=ignore -O2 -DMAKEFILE_FIRMWARE_VERSION=\\\"$(FIRMWARE_VERSION)\\\" -Wa,-a

all: $(PATH_BINARIES) $(PATH_OBJECTS)
	cd $(PATH_OBJECTS) && $(CC) $(CFLAGS) -I$(PATH_INCLUDES) $(SOURCES) -o $(BINARY_NAME)
	mv $(PATH_OBJECTS)/$(BINARY_NAME) $(PATH_BINARIES)

debug: CFLAGS += -DLOG_IS_ENABLED
debug: all

$(PATH_BINARIES):
	mkdir -p $(PATH_BINARIES)

$(PATH_OBJECTS):
	mkdir -p $(PATH_OBJECTS)

clean:
	rm -rf $(PATH_BINARIES) $(PATH_OBJECTS)

FLASH_TOOL_PATH=$(shell find /opt/microchip/mplabx -iname ipecmd.jar)
flash:
	@# -TPPK3 selects the PICkit 3 programmer
	@# -OL allows to release the reset signal
	@# -Z allows to preserve the data EEPROM memory content
	java -jar $(FLASH_TOOL_PATH) -P18F27K42 -TPPK3 -M -F$(PATH_BINARIES)/$(BINARY_NAME) -OL -Z310000-3103FF

cppcheck:
	cppcheck -I $(PATH_INCLUDES) --platform=pic8-enhanced --check-level=exhaustive $(PATH_SOURCES)

doxygen:
	doxygen Doxyfile
