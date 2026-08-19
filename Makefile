NAME   = retnots
LEVEL  ?= slope.pcx
MAKE   = make --no-print-directory
CROP   ?= 130

CFLAGS = --nostdinc --nostdlib --no-std-crt0 --no-zp-spill --opt-code-speed
LFLAGS = -n -b OAM=0x200 -b BSS=0x300 -b RODATA=0x8000 -b VECTOR=0xfffa

TOOL_FILES = pcx-dump.c lz.c

all: build

run: build
	fceux $(NAME).nes

test:
	magick slope.pcx -crop 256x6000+0+$$((32 * $(CROP))) testing.pcx
	LEVEL=testing.pcx make run

mame: build
	mame nes -cart $(NAME).nes

pal:
	TFLAGS=-DPAL make build

size:
	echo -n $(NAME) $(SEG) "size "
	grep ^$(SEG).*bytes $(NAME).map | sed "s/[. ] */ /g" | cut -f 5 -d " "

.SILENT build:
	echo Compile pcx-dump
	gcc $(TOOL_FILES) $(TFLAGS) -lm -o pcx-dump
	echo "#define $(subst .,_,$(LEVEL))" > tables.hdr
	./pcx-dump -r tiles.chr
	./pcx-dump -t special.pcx	>> tables.hdr
	./pcx-dump -t speed2x.pcx	>> tables.hdr
	./pcx-dump -t fonts.pcx		>> tables.hdr
	./pcx-dump -l title.pcx		>> tables.hdr
	./pcx-dump -l $(LEVEL)		>> tables.hdr
	./pcx-dump -p tiles.chr
	./pcx-dump -g tables.hdr	>> tables.hdr
	./pcx-dump -s sprites.pcx
	echo Compile $(NAME).c
	sdcc -mmos6502 $(CFLAGS) $(NAME).c -c
	echo Link $(NAME).ihx
	sdld $(LFLAGS) -m -i $(NAME).ihx $(NAME).rel
	echo Convert $(NAME).prg
	makebin -p -yo A -o 0x8000 retnots.ihx retnots.prg
	cat tiles.chr sprites.chr > $(NAME).chr
	cat header.rom $(NAME).prg $(NAME).chr > $(NAME).nes
	SEG=CODE $(MAKE) size
	SEG=RODATA $(MAKE) size

clean:
	rm -f *.asm *.ihx *.lst *.map *.rel *.sym *.chr *.hdr *.prg *.nes
	rm -f pcx-dump testing.pcx
