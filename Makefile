NAME   = retnots
MAKE   = make --no-print-directory

CFLAGS = --nostdinc --nostdlib --no-std-crt0 --no-zp-spill --opt-code-speed
LFLAGS = -n -b OAM=0x200 -b BSS=0x300 -b CODE=0x8000 -b VECTOR=0xfffa

TOOL_FILES = pcx-dump.c

all: build

run: build
	fceux $(NAME).nes

pal:
	TFLAGS=-DPAL make build

size:
	echo -n $(NAME) $(SEG) "size "
	grep ^$(SEG).*bytes $(NAME).map | sed "s/[. ] */ /g" | cut -f 5 -d " "

.SILENT build:
	echo Compile pcx-dump
	gcc $(TOOL_FILES) $(TFLAGS) -lm -o pcx-dump
	./pcx-dump -r tiles.chr
	./pcx-dump -t fonts.pcx
	./pcx-dump -l slope.pcx
	./pcx-dump -p tiles.chr
	./pcx-dump -s sprites.pcx
	echo Compile $(NAME).c
	sdcc -mmos6502 $(CFLAGS) $(NAME).c -c
	echo Link $(NAME).ihx
	sdld $(LFLAGS) -m -i $(NAME).ihx $(NAME).rel
	echo Convert $(NAME).prg
	hex2bin -e prg $(NAME).ihx > /dev/null
	cat tiles.chr sprites.chr > $(NAME).chr
	cat header.rom $(NAME).prg $(NAME).chr > $(NAME).nes
	SEG=CODE $(MAKE) size
	SEG=RODATA $(MAKE) size

clean:
	rm -f *.asm *.ihx *.lst *.map *.rel *.sym *.chr *.hdr *.prg *.nes
	rm -f pcx-dump
