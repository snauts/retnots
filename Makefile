NAME   = retnots

CFLAGS = --nostdinc --nostdlib --no-std-crt0 --no-zp-spill --opt-code-speed
LFLAGS = -b OAM=0x200 -b BSS=0x300 -b CODE=0x8000 -b VECTOR=0xfffa

TOOL_FILES = pcx-dump.c

all: build

run: build
	fceux $(NAME).nes

pal:
	TFLAGS=-DPAL make build

build:
	gcc $(TOOL_FILES) $(TFLAGS) -lm -o pcx-dump
	./pcx-dump -r tiles.chr
	./pcx-dump -t fonts.pcx
	./pcx-dump -p tiles.chr
	./pcx-dump -s sprites.pcx
	@echo Compile $(NAME).c
	@sdcc -mmos6502 $(CFLAGS) $(NAME).c -c
	@echo Link $(NAME).ihx
	@sdld $(LFLAGS) -m -i $(NAME).ihx $(NAME).rel
	hex2bin -e prg $(NAME).ihx > /dev/null
	cat tiles.chr sprites.chr > $(NAME).chr
	cat header.rom $(NAME).prg $(NAME).chr > $(NAME).nes

clean:
	rm -f *.asm *.ihx *.lst *.map *.rel *.sym *.chr *.hdr *.prg *.nes
	rm -f pcx-dump
