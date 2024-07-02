# Use https://github.com/wernsey/chip8 to assemble these programs
AS = c8asm
ASFLAGS = -v

SOURCES = $(wildcard *.asm)
BINARIES = $(patsubst %.asm,%.CH8,$(SOURCES))

all: $(BINARIES)

Test_Sprites.CH8: Test_Sprites.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f *.CH8
