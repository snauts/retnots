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
    __asm__("_control:		.ds 1");
    __asm__("_signal:		.ds 1");
    __asm__("_button:		.ds 1");
    __asm__("_scroll:		.ds 1");
    __asm__("_speed:		.ds 1");

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

#define HEIGHT		240

extern byte oam[256];

extern volatile word ppu_ptr;
extern volatile byte ppu_count;
extern volatile byte ppu_buffer[32];

extern volatile byte counter;
extern volatile byte control;
extern volatile byte signal;
extern volatile byte button;
extern volatile byte scroll;
extern volatile byte speed;

static void wait_vblank(void) {
    while ((PPUSTATUS() & 0x80) == 0) { }
}

static void wait_signal(void) {
    signal = 1;
    while (signal) { }
}

static void set_ppu_buffer(byte x) {
    for (byte i = 0; i < 32; i++) ppu_buffer[i] = x;
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

static void wipe_sprites(void) {
    byte i = 0;
    do { oam[i++] = 0xff; } while (i != 0);
}

static void init_memory(void) {
    speed = 0;
    scroll = 0;
    button = 0;
    counter = 0;
    ppu_count = 0;

    control = BIT(7) | BIT(3);

    wipe_sprites();
}

static byte update_scroll(void) {
    byte update = scroll & 0x08;
    scroll += speed;
    if (scroll >= HEIGHT) {
	scroll = scroll - HEIGHT;
	control ^= BIT(1);
    }
    return update ^ (scroll & 0x08);
}

static void ppu_ctrl(void) {
    PPUSCROLL(0x00);
    PPUSCROLL(scroll);
    PPUCTRL(control);
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
    set_ppu_buffer(0xf);
    ppu_update(32);
}

static void wipe_vram(word ptr) {
    ppu_ptr = ptr;
    set_ppu_buffer(0x0);
    for (byte i = 0; i < 32; i++) {
	ppu_update(32);
    }
}

static void wipe_screen(void) {
    wipe_sprites();
    wipe_vram(0x2000);
    wipe_vram(0x2800);
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

static byte char_to_tile(char c) {
    byte sym = c + 1;
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
    0x0f, 0x2d, 0x3d, 0x30,
};

static void show_title_screen(void) {
    print_msg("RETNOTS", 13, 12);
    setup_palette(title_palette, 0, sizeof(title_palette));
}

static void start_new_game(void) {
    for (byte i = 0; i < 30; i++) {
	print_msg("OOO", i, i);
    }
}

static void game_loop(void) {
    for (;;) {
	wait_signal();
	update_scroll();
	if (check_button() & BUTTON_START) speed++;
    }
}

void game_startup(void) {
    hw_init();

    for (;;) {
	wipe_screen();
	show_title_screen();
	wait_start_button();
	start_new_game();
	game_loop();
    }
}

/* must be very last */
void jump_vectors(void) __naked {
    __asm__(".area VECTOR (PAG)");
    __asm__("nmi_ptr::	.dw _nmi");
    __asm__("rst_ptr::	.dw _rst");
    __asm__("irq_ptr::	.dw _irq");
}
