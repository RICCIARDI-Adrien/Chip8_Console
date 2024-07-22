PATH_BINARIES = $(shell realpath .)/Binaries
PATH_INCLUDES = $(shell realpath .)/Includes
PATH_OBJECTS = $(shell realpath .)/Objects
PATH_SOURCES = $(shell realpath .)/Sources

BINARY_NAME = Chip8_Console_Firmware.hex
SOURCES = \
	$(PATH_SOURCES)/Display.c \
	$(PATH_SOURCES)/FAT.c \
	$(PATH_SOURCES)/INI_Parser.c \
	$(PATH_SOURCES)/Interpreter.c \
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
CFLAGS = -mcpu=18F27K42 -D_XTAL_FREQ=64000000 -muse-ivt -mundefints=ignore -O2 -Wa,-a

all: $(PATH_BINARIES) $(PATH_OBJECTS)
	cd $(PATH_OBJECTS) && $(CC) $(CFLAGS) -I$(PATH_INCLUDES) $(SOURCES) -o $(BINARY_NAME)
	mv $(PATH_OBJECTS)/$(BINARY_NAME) $(PATH_BINARIES)

debug: CFLAGS += -DSERIAL_PORT_ENABLE_LOGGING
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
	java -jar $(FLASH_TOOL_PATH) -P18F27K42 -TPPK3 -M -F$(PATH_BINARIES)/$(BINARY_NAME) -OL
