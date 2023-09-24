PATH_BINARIES = $(shell realpath .)/Binaries
PATH_OBJECTS = $(shell realpath .)/Objects
PATH_SOURCES = $(shell realpath .)/Sources

BINARY_NAME = Chip8_Console_Firmware.hex
SOURCES = $(PATH_SOURCES)/Main.c

CC = xc8-cc
#CFLAGS = -mcpu=18F26K22 -D_XTAL_FREQ=64000000 --objdir=$(PATH_OBJECTS) --outdir=$(PATH_OBJECTS)
CFLAGS = -mcpu=18F26K22 -D_XTAL_FREQ=64000000

all: $(PATH_BINARIES) $(PATH_OBJECTS)
	cd $(PATH_OBJECTS) && $(CC) $(CFLAGS) $(SOURCES) -o $(BINARY_NAME)
	mv $(PATH_OBJECTS)/$(BINARY_NAME) $(PATH_BINARIES)

$(PATH_BINARIES):
	mkdir -p $(PATH_BINARIES)

$(PATH_OBJECTS):
	mkdir -p $(PATH_OBJECTS)

clean:
	rm -rf $(PATH_BINARIES) $(PATH_OBJECTS)
