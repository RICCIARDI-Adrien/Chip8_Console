# Use https://github.com/wernsey/chip8 to assemble these programs
AS = c8asm
ASFLAGS = -v

SOURCES = $(wildcard *.asm)
BINARIES = $(patsubst %.asm,%.CH8,$(SOURCES))

all: $(BINARIES)

Test_Collisions.CH8: Test_Collisions.asm
	$(AS) $(ASFLAGS) $< -o $@

Test_Sprites.CH8: Test_Sprites.asm
	$(AS) $(ASFLAGS) $< -o $@

Test_Stack_Underflow.CH8: Test_Stack_Underflow.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f *.CH8
