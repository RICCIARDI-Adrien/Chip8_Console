AS = c8asm
ASFLAGS = -v

SOURCES = $(wildcard *.asm)
BINARIES = $(patsubst %.asm,%.ch8,$(SOURCES))

all: $(BINARIES)

Test_Sprites.ch8: Test_Sprites.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -f *.ch8
