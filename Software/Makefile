PATH_BINARIES = $(shell realpath .)/Binaries
PATH_INCLUDES = $(shell realpath .)/Includes
PATH_OBJECTS = $(shell realpath .)/Objects
PATH_SOURCES = $(shell realpath .)/Sources

BINARY_NAME = Chip8_Console_Firmware.hex
SOURCES = $(PATH_SOURCES)/Main.c $(PATH_SOURCES)/Serial_Port.c

CC = xc8-cc
CFLAGS = -mcpu=18F27K42 -D_XTAL_FREQ=64000000

all: $(PATH_BINARIES) $(PATH_OBJECTS)
	cd $(PATH_OBJECTS) && $(CC) $(CFLAGS) -I$(PATH_INCLUDES) $(SOURCES) -o $(BINARY_NAME)
	mv $(PATH_OBJECTS)/$(BINARY_NAME) $(PATH_BINARIES)

$(PATH_BINARIES):
	mkdir -p $(PATH_BINARIES)

$(PATH_OBJECTS):
	mkdir -p $(PATH_OBJECTS)

clean:
	rm -rf $(PATH_BINARIES) $(PATH_OBJECTS)
