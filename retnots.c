typedef signed char int8;
typedef unsigned char byte;
typedef unsigned short word;

#define NULL ((void *) 0)

#include "slope.hdr"

void sdcc_deps(void) __naked {
    __asm__(".area ZP (PAG)");
    __asm__("REGTEMP:		.ds 8");
    __asm__("DPTR:		.ds 2");
    __asm__("_ppu_ptr:		.ds 2");
    __asm__("_ppu_read: 	.ds 2");
    __asm__("_ppu_count:	.ds 1");
    __asm__("_ppu_buffer:	.ds 32");
    __asm__("_counter:		.ds 1");
    __asm__("_row_ptr:		.ds 2");
    __asm__("_row_idx:		.ds 1");
    __asm__("_control:		.ds 1");
    __asm__("_pending:		.ds 1");
    __asm__("_signal:		.ds 1");
    __asm__("_button:		.ds 1");
    __asm__("_scroll:		.ds 1");
    __asm__("_speed:		.ds 1");
    __asm__("_safe:		.ds 1");
    __asm__("_line:		.ds 1");
    __asm__("_bump:		.ds 1");
    __asm__("_pos:		.ds 1");

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
#define POS(x, y)	(0x2000 + ((y) << 5) + (x))
#define BYTE(v, i)	((byte *) &(v))[i]
#define SIZE(array)	(sizeof(array) / sizeof(*(array)))

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
#define PPUDATA_RD()	MEM_RD(0x2007)

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

#define BUTTON_SOME	(BUTTON_START | BUTTON_LEFT | BUTTON_RIGHT)

#define HEIGHT		240

extern byte oam[256];

extern volatile word ppu_ptr;
extern volatile word ppu_read;
extern volatile byte ppu_count;
extern volatile byte ppu_buffer[32];

extern volatile byte counter;
extern volatile byte control;
extern volatile byte signal;
extern volatile byte scroll;

extern void* const *row_ptr;
extern byte row_idx;
extern byte pending;
extern byte button;
extern byte speed;
extern byte line;
extern byte bump;
extern int8 safe;
extern byte pos;

static void wait_vblank(void) {
    while ((PPUSTATUS() & 0x80) == 0) { }
}

static void wait_signal(void) {
    signal = 1;
    while (signal) { }
}

static void ppu_set(byte x) {
    for (byte i = 0; i < 32; i++) ppu_buffer[i] = x;
}

static void ppu_cpy(const byte *ptr) {
    for (byte i = 0; i < 32; i++) ppu_buffer[i] = ptr[i];
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
    safe = 9;
    line = 9;
    bump = 0;
    speed = 0;
    scroll = 0;
    button = 0;
    counter = 0;
    pending = 0;
    ppu_count = 0;

    control = BIT(7) | BIT(3);

    wipe_sprites();
}

static void update_scroll(void) {
    byte update = scroll & 0x08;
    scroll += speed;
    if (scroll >= HEIGHT) {
	scroll = scroll - HEIGHT;
	control ^= BIT(1);
    }
    if (update ^ (scroll & 0x08)) {
	if (++line >= 60) line = 0;
	pending++;
    }
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

    PPUADDR(ppu_read >> 8);
    PPUADDR(ppu_read & 0xff);
    PPUDATA_RD(); /* read delay */
    ppu_buffer[0] = PPUDATA_RD();
    ppu_buffer[1] = PPUDATA_RD();

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

static void setup_palette(const byte *palette) {
    ppu_ptr = 0x3f00;
    ppu_cpy(palette);
    ppu_update(32);
}

static void wipe_palette(void) {
    ppu_ptr = 0x3f00;
    ppu_set(0xf);
    ppu_update(32);
}

static void wipe_vram(word ptr) {
    ppu_ptr = ptr;
    for (byte i = 0; i < 32; i++) {
	ppu_set(0x0);
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

static void wait_some_button(void) {
    while (!(check_button() & BUTTON_SOME)) { }
}

static byte char_to_tile(char c) {
    byte sym = c + 1;
    if (c == ' ') return 0;
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

static void print_msg(const char *msg, word pos) {
    byte i = 0;
    ppu_ptr = pos;
    while (msg[i] != 0) {
	ppu_buffer[i] = char_to_tile(msg[i]);
	i++;
    }
    ppu_update(i);
}

static const byte game_palette[] = {
    0x30, 0x2d, 0x3d, 0x0c,
    0x30, 0x2d, 0x3d, 0x0c,
    0x30, 0x2d, 0x3d, 0x0c,
    0x30, 0x2d, 0x3d, 0x0c,

    0x30, 0x0f, 0x2d, 0x3d,
    0x30, 0x2d, 0x3d, 0x0c,
    0x30, 0x2d, 0x3d, 0x0c,
    0x30, 0x2d, 0x3d, 0x0c,
};

static void show_title_screen(void) {
    print_msg("RETNOTS", POS(13, 12));
    setup_palette(game_palette);
}

static void reset_rows(void) {
    row_ptr = slope_addr;
    row_idx = 0;
}

static byte is_bottom(void) {
    return !BYTE(*row_ptr, 1);
}

static void update_row(void) {
    byte i = row_table[row_idx];
    BYTE(ppu_ptr, 0) = (i & 0xf0);
    BYTE(ppu_ptr, 1) = (i & 0x0f) | 0x20;
    row_idx = (row_idx + 1) & 0x3f;
    ppu_cpy(*row_ptr++);
    ppu_count = 32;
}

static void produce_new_row(void) {
    byte i = row_table[row_idx];
    if ((i & 0xc3) == 0xc3) pending++;

    if (pending > 0) {
	update_row();
	pending--;
	if (is_bottom()) {
	    reset_rows();
	    speed = 0;
	}
    }
}

static void prepare_readback(void) {
    byte i = line_table[line];
    BYTE(ppu_read, 0) = (i & 0xf0) + (pos >> 3);
    BYTE(ppu_read, 1) = (i & 0x0f) | 0x20;
    if (safe < 0) {
	bump = ppu_buffer[0] || ppu_buffer[1];
    }
    else {
	safe--;
    }
}

#define Y(y) ((y) + 4)
static const byte racoon_stand[] = {
    Y(0x30),0x00,0x00,0xf8, Y(0x30),0x01,0x00,0x00, Y(0x30),0x02,0x00,0x08,
    Y(0x38),0x10,0x00,0xf8, Y(0x38),0x11,0x00,0x00, Y(0x38),0x12,0x00,0x08,
    Y(0x40),0x20,0x00,0xf8, Y(0x40),0x21,0x00,0x00, Y(0x40),0x22,0x00,0x08,
};

static const byte racoon_down[] = {
    Y(0x30),0x06,0x00,0xf8, Y(0x30),0x07,0x00,0x00, Y(0x30),0x08,0x00,0x08,
    Y(0x38),0x16,0x00,0xf8, Y(0x38),0x17,0x00,0x00, Y(0x38),0x18,0x00,0x08,
    Y(0x40),0x26,0x00,0xf8, Y(0x40),0x27,0x00,0x00, Y(0x40),0x28,0x00,0x08,
};

static const byte racoon_right[] = {
    Y(0x30),0x03,0x00,0xf8, Y(0x30),0x04,0x00,0x00, Y(0x30),0x05,0x00,0x08,
    Y(0x38),0x13,0x00,0xf8, Y(0x38),0x14,0x00,0x00, Y(0x38),0x15,0x00,0x08,
    Y(0x40),0x23,0x00,0xf8, Y(0x40),0x24,0x00,0x00, Y(0x40),0x25,0x00,0x08,
};

static const byte racoon_left[] = {
    Y(0x30),0x05,0x40,0xf8, Y(0x30),0x04,0x40,0x00, Y(0x30),0x03,0x40,0x08,
    Y(0x38),0x15,0x40,0xf8, Y(0x38),0x14,0x40,0x00, Y(0x38),0x13,0x40,0x08,
    Y(0x40),0x25,0x40,0xf8, Y(0x40),0x24,0x40,0x00, Y(0x40),0x23,0x40,0x08,
};

static void animate_racoon(const byte *data) {
    for (byte i = 0; i < 36; i++) {
	oam[i] = *data++;
	if ((i & 3) == 3) {
	    oam[i] += pos;
	}
    }
}

static void start_new_game(void) {
    pos = 124;
    reset_rows();
    animate_racoon(racoon_stand);
    for (byte i = 0; i < 32; i++) {
	update_row();
	ppu_update(32);
    }
    wait_some_button();
    speed = 1;
}

static void check_controls(void) {
    check_button();
    if (button & BUTTON_RIGHT) {
	pos = pos + 1;
	animate_racoon(racoon_right);
    }
    else if (button & BUTTON_LEFT) {
	pos = pos - 1;
	animate_racoon(racoon_left);
    }
    else {
	animate_racoon(bump ? racoon_stand : racoon_down);
    }
}

static void game_loop(void) {
    for (;;) {
	wait_signal();
	update_scroll();
	prepare_readback();
	produce_new_row();
	check_controls();
    }
}

void game_startup(void) {
    hw_init();

    for (;;) {
	wipe_screen();
	show_title_screen();
	wait_some_button();
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
