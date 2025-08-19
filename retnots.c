typedef signed char int8;
typedef unsigned char byte;
typedef unsigned short word;

#define NULL ((void *) 0)

void sdcc_deps(void) __naked {
    __asm__(".area ZP (PAG)");
    __asm__("REGTEMP:		.ds 8");
    __asm__("DPTR:		.ds 2");
    __asm__("_ppu_ptr:		.ds 2");
    __asm__("_ppu_count:	.ds 1");
    __asm__("_ppu_buffer:	.ds 32");
    __asm__("_counter:		.ds 1");
    __asm__("_signal:		.ds 1");
    __asm__("_button:		.ds 1");
    __asm__("_scroll:		.ds 1");

    __asm__(".area OAM (PAG)");
    __asm__("_oam:		.ds 256");

    __asm__(".area CODE");
}

void irq(void) __naked {
    __asm__("rti");
}

void nmi(void) __naked {
    __asm__("pha");
    __asm__("txa");
    __asm__("pha");
    __asm__("tya");
    __asm__("pha");

    __asm__("jsr _irq_handler");

    __asm__("pla");
    __asm__("tay");
    __asm__("pla");
    __asm__("tax");
    __asm__("pla");

    __asm__("rti");
}

void rst(void) __naked {
    __asm__("sei");
    __asm__("cld");
    __asm__("ldx #0xff");
    __asm__("txs");

    __asm__("jsr _game_startup");
}

#define BIT(n)		(((byte) 1) << (n))

#define MEM_RD(a)	(* (volatile byte *) (a))
#define MEM_WR(a, x)	(* (volatile byte *) (a) = (x))

#define PPUCTRL(x)	MEM_WR(0x2000, x)
#define PPUMASK(x)	MEM_WR(0x2001, x)
#define PPUSTATUS()	MEM_RD(0x2002)
#define OAMADDR(x)	MEM_WR(0x2003, x)
#define OAMDATA(x)	MEM_WR(0x2004, x)
#define PPUSCROLL(x)	MEM_WR(0x2005, x)
#define PPUADDR(x)	MEM_WR(0x2006, x)
#define PPUDATA(x)	MEM_WR(0x2007, x)

#define DMCFREQ(x)	MEM_WR(0x4010, x)
#define OAMDMA(x)	MEM_WR(0x4014, x)
#define SND_CHN(x)	MEM_WR(0x4015, x)
#define JOY1_RD()	MEM_RD(0x4016)
#define JOY1_WR(x)	MEM_WR(0x4016, x)
#define JOY2_WR(x)	MEM_WR(0x4017, x)

#define BUTTON_A	BIT(7)
#define BUTTON_B	BIT(6)
#define BUTTON_SELECT	BIT(5)
#define BUTTON_START	BIT(4)
#define BUTTON_UP	BIT(3)
#define BUTTON_DOWN	BIT(2)
#define BUTTON_LEFT	BIT(1)
#define BUTTON_RIGHT	BIT(0)

extern byte oam[256];

extern volatile word ppu_ptr;
extern volatile byte ppu_count;
extern volatile byte ppu_buffer[32];

extern volatile byte counter;
extern volatile byte signal;
extern volatile byte button;
extern volatile byte scroll;

static void wait_vblank(void) {
    while ((PPUSTATUS() & 0x80) == 0) { }
}

static void wait_signal(void) {
    signal = 1;
    while (signal) { }
}

static void memset(byte *buf, byte val, word count) {
    while (count-- > 0) *buf++ = val;
}

static void memcpy(byte *dst, const byte *src, word count) {
    while (count-- > 0) *dst++ = *src++;
}

static void clear_palette(void) {
    PPUADDR(0x3f);
    PPUADDR(0x00);
    for (byte i = 0; i < 32; i++) {
	PPUDATA(0x0f);
    }
}

static void init_memory(void) {
    scroll = 0;
    button = 0;
    counter = 0;
    ppu_count = 0;

    memset(oam, 255, 0x100);
}

static void ppu_ctrl(void) {
    PPUSCROLL(0x00);
    PPUSCROLL(scroll);
    PPUCTRL(BIT(7) | BIT(3));
}

void irq_handler(void) {
    byte i;
    PPUADDR(ppu_ptr >> 8);
    PPUADDR(ppu_ptr & 0xff);
    for (i = 0; i < ppu_count; i++) {
	PPUDATA(ppu_buffer[i]);
    }
    ppu_count = 0;
    counter++;

    PPUMASK(0x00);
    OAMADDR(0x00);
    OAMDMA(0x02); /* oam addr high byte */
    ppu_ctrl();
    PPUMASK(0x1e);
    signal = 0;
}

static void hw_init(void) {
    JOY2_WR(0x40);
    PPUCTRL(0x00);
    PPUMASK(0x00);
    DMCFREQ(0x00);
    SND_CHN(0x0f);

    wait_vblank();
    init_memory();
    wait_vblank();
    clear_palette();
    wait_vblank();
    ppu_ctrl();
}

static void ppu_update(byte amount) {
    ppu_count = amount;
    while (ppu_count > 0) { }
    ppu_ptr += amount;
}

static void setup_palette(const byte *ptr, byte offset, byte amount) {
    ppu_ptr = 0x3f00 + offset;
    memcpy(ppu_buffer, ptr, amount);
    ppu_update(amount);
}

static void wipe_palette(void) {
    ppu_ptr = 0x3f00;
    memset(ppu_buffer, 0xf, 32);
    ppu_update(32);
}

static void wipe_screen(void) {
    ppu_ptr = 0x2000;
    memset(oam, 255, 0x100);
    memset(ppu_buffer, 0, 32);
    for (byte i = 0; i < 32; i++) {
	ppu_update(32);
    }
    wipe_palette();
}

static byte check_button(void) {
    JOY1_WR(0x01);
    JOY1_WR(0x00);

    byte press, state = 0;
    for (byte i = 0; i < 8; i++) {
	state = state << 1;
	state |= JOY1_RD() & 1;
    }
    press = (button ^ state) & state;
    button = state;
    return press;
}

static void wait_start_button(void) {
    while (!(check_button() & BUTTON_START)) { }
}

static const char special[] = " @:";
static byte char_to_tile(char c) {
    byte sym;
    for (sym = 0; sym < sizeof(special) - 1; sym++) {
	if (special[sym] == c) return sym;
    }

    sym = sym + c;
    if (c >= '0' && c <= '9') {
	return sym - '0';
    }

    sym = sym + 10;
    if (c >= 'A' && c <= 'Z') {
	return sym - 'A';
    }
    if (c >= 'a' && c <= 'z') {
	return sym - 'a';
    }
    return 0;
}

static void print_msg(const char *msg, byte x, word y) {
    byte i = 0;
    ppu_ptr = 0x2020 + (y << 5) + x;
    while (msg[i] != 0) {
	ppu_buffer[i] = char_to_tile(msg[i]);
	i++;
    }
    ppu_update(i);
}

static const byte title_palette[] = {
    0x0f, 0x12, 0x22, 0x32,
};

void game_startup(void) {
    hw_init();

    for (;;) {
	wipe_screen();
	print_msg("@ RETNOTS @", 11, 12);
	setup_palette(title_palette, 0, sizeof(title_palette));
	wait_start_button();
    }
}

/* must be very last */
void jump_vectors(void) __naked {
    __asm__(".area VECTOR (PAG)");
    __asm__("nmi_ptr::	.dw _nmi");
    __asm__("rst_ptr::	.dw _rst");
    __asm__("irq_ptr::	.dw _irq");
}
