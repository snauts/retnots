typedef signed char int8;
typedef unsigned char byte;
typedef unsigned short word;

#define NULL ((void *) 0)

#include "tables.hdr"

#include "music.h"

void sdcc_deps(void) __naked {
    __asm__(".area ZP (PAG)");
    __asm__("REGTEMP:		.ds 8");
    __asm__("DPTR:		.ds 2");
    __asm__("_control:		.ds 1");
    __asm__("_benchmark:	.ds 2");
    /* memset(0) from here */
    __asm__("_counter:		.ds 1");
    __asm__("_ppu_ptr:		.ds 2");
    __asm__("_ppu_read: 	.ds 2");
    __asm__("_ppu_count:	.ds 1");
    __asm__("_ppu_buffer:	.ds 32");
    __asm__("_row_ptr:		.ds 2");
    __asm__("_row_idx:		.ds 1");
    __asm__("_pending:		.ds 1");
    __asm__("_signal:		.ds 1");
    __asm__("_button:		.ds 1");
    __asm__("_scroll:		.ds 1");
    __asm__("_pages:		.ds 1");
    __asm__("_noise:		.ds 1");
    __asm__("_bling:		.ds 1");
    __asm__("_lives:		.ds 1");
    __asm__("_speed:		.ds 1");
    __asm__("_ticks:		.ds 4");
    __asm__("_score:		.ds 4");
    __asm__("_safe:		.ds 1");
    __asm__("_line:		.ds 1");
    __asm__("_bump:		.ds 1");
    __asm__("_drum:		.ds 1");
    __asm__("_note:		.ds 1");
    __asm__("_skip:		.ds 1");
    __asm__("_time:		.ds 1");
    /* memset(0) END */
    __asm__("_pos:		.ds 1");
    __asm__("_mute:		.ds 1");

    __asm__(".area OAM (PAG)");
    __asm__("_spr:");
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
#define MAX(x, y)	((x) > (y) ? (x) : (y))
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

#define TRI_CR(x)	MEM_WR(0x4008, x)
#define TRI_LO(x)	MEM_WR(0x400A, x)
#define TRI_HI(x)	MEM_WR(0x400B, x)

#define NOISE_VL(x)	MEM_WR(0x400C, x)
#define NOISE_LO(x)	MEM_WR(0x400E, x)
#define NOISE_HI(x)	MEM_WR(0x400F, x)

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

#define SKI_RIGHT	(BUTTON_RIGHT | BUTTON_A)
#define SKI_LEFT	(BUTTON_LEFT  | BUTTON_B)

#define BUTTON_SOME	(BUTTON_START | SKI_LEFT | SKI_RIGHT)

#define HEIGHT		240
#define SLIDE	 	10
#define JUMP		11
#define SPECIAL 	11
#define NAME_SIZE	7
#define ENTRY_SIZE	11

/* sprites */
#define HEAD	1
#define TAIL	1
#define DIGIT	9
#define DEBRIS	9
#define SNOW_L	10
#define SNOW_R	11
#define SCORE	16
#define LAST	19
#define LIVES	20
#define CARET	32

static struct Sprite {
    byte y;
    byte idx;
    byte cfg;
    byte x;
};

extern struct Sprite spr[64];
extern byte oam[256];

extern volatile word ppu_ptr;
extern volatile word ppu_read;
extern volatile byte ppu_count;
extern volatile byte ppu_buffer[32];

extern volatile word benchmark;
extern volatile byte counter;
extern volatile byte control;
extern volatile byte signal;
extern volatile byte scroll;

extern const byte *row_ptr;
extern byte score[4];
extern byte ticks[4];
extern byte row_idx;
extern byte pending;
extern byte button;
extern byte pages;
extern byte noise;
extern byte bling;
extern int8 lives;
extern byte speed;
extern byte line;
extern byte bump;
extern int8 safe;
extern byte drum;
extern byte skip;
extern byte note;
extern byte time;
extern byte mute;
extern byte pos;

static const char default_table[] = "HORACE.0300JENOTS.0200ARCHIE.0100";

static byte table[sizeof(default_table)];

static void play_music(void);
static byte char_to_tile(char c);

static void wait_vblank(void) {
    while ((PPUSTATUS() & 0x80) == 0) { }
}

static void wait_signal(void) {
    signal = 1;
    word cycles = 0;
    while (signal) { cycles++; }
    if (benchmark > cycles) benchmark = cycles;
}

static void memset(byte *buf, byte val, byte count) {
    while (count-- > 0) { *buf++ = val; }
}

static void memcpy(byte *dst, byte *src, byte count) {
    while (count-- > 0) { *dst++ = *src++; }
}

static void wipe_sprites(void) {
    byte i = 0;
    do { oam[i++] = 0xff; } while (i != 0);
}

static void reset_game(void) {
    memset(&counter, 0, &pos - &counter);
    lives = 3;
    safe = 9;
    line = 9;
}

static void init_memory(void) {
    benchmark = 0xffff;

#ifdef testing_pcx
    mute = 1;
#else
    mute = 0;
#endif

    reset_game();
    wipe_sprites();

    control = BIT(7) | BIT(3);

    for (byte i = 0; i < sizeof(default_table); i++) {
	table[i] = char_to_tile(default_table[i]);
    }
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
    ppu_ctrl();

    /* enable pulse */
    MEM_WR(0x4001, 0x08);
    MEM_WR(0x4005, 0x08);
}

static void ppu_update(byte amount) {
    ppu_count = amount;
    while (ppu_count > 0) { }
    ppu_ptr += amount;
}

static void wipe_palette(void) {
    ppu_ptr = 0x3f00;
    memset(ppu_buffer, 0x30, 32);
    ppu_update(32);
}

static void wipe_vram(word ptr) {
    ppu_ptr = ptr;
    for (byte i = 0; i < 32; i++) {
	memset(ppu_buffer, 0x00, 32);
	ppu_update(32);
    }
}

static void wipe_screen(void) {
    wipe_palette();
    wipe_sprites();
    wipe_vram(0x2000);
    wipe_vram(0x2800);
    control &= ~BIT(1);
    scroll = 0;
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

static byte no_button_push(void) {
    return !(check_button() & BUTTON_SOME);
}

static void wait_button(void) {
    while (no_button_push()) { }
}

static const char special[] = ".:?";

static byte char_to_tile(char c) {
    byte sym = c + START_OF_fonts;
    if (c == ' ') return 0;
    for (byte i = 0; i < sizeof(special) - 1; i++) {
	if (special[i] == c) return sym - c;
	sym++;
    }
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
    0x30, 0x11, 0x31, 0x05,
    0x30, 0x3d, 0x1a, 0x0a,
    0x30, 0x3d, 0x18, 0x08,

    0x30, 0x0f, 0x2d, 0x3d,
    0x30, 0x0f, 0x05, 0x15,
    0x30, 0x30, 0x30, 0x30,
    0x30, 0x30, 0x30, 0x30,
};

static void setup_palette(void) {
    ppu_ptr = 0x3f00;
    memcpy(ppu_buffer, game_palette, 32);
    ppu_update(32);
}

static void show_highscore_table(void) {
    byte i = 0;
    for (byte y = 0; y < 3; y++) {
	ppu_ptr = POS(10, 17) + (y << 6);
	for (byte x = 0; x < 13; x++) {
	    byte spacing = (7 <= x && x <= 8);
	    ppu_buffer[x] = spacing ? char_to_tile('.') : table[i++];
	}
	ppu_update(13);
    }
}

static byte head, tail;
static byte window[256];

#define ENOUGH() (((head - tail) & 0xff) < 32)

static void decompress(void) {
    while (ENOUGH() && *row_ptr) {
        byte n = *row_ptr & 0x7f;
        if (*(row_ptr++) & 0x80) {
	    while (n-- > 0) {
		byte from = head - *row_ptr;
		window[head] = window[from];
		head++;
	    }
	    row_ptr++;
        }
        else {
	    while (n-- > 0) {
		window[head] = *row_ptr;
		row_ptr++;
		head++;
	    }
        }
    }
}

static byte is_bottom(void) {
    return head == tail && *row_ptr == 0;
}

static void update_row(void) {
    byte i = row_table[row_idx];
    BYTE(ppu_ptr, 0) = (i & 0xf0);
    BYTE(ppu_ptr, 1) = (i & 0x0f) | 0x20;
    row_idx = (row_idx + 1) & 0x3f;
    decompress();
    for (byte i = 0; i < 32; i++) {
	ppu_buffer[i] = window[tail++];
    }
    ppu_count = 32;
}

static void reset_rows(const byte *ptr, byte n) {
    row_ptr = ptr;
    row_idx = 0;
    head = 0;
    tail = 0;

    while (n-- > 0) {
	update_row();
	ppu_update(32);
    }
}

static void produce_new_row(void) {
    byte i = row_table[row_idx];
    if ((i & 0xc3) == 0xc3) {
	pending++;
	pages++;
    }

    if (pending > 0) {
	update_row();
	pending--;
	if (is_bottom()) {
	    lives |= 0x80;
	    speed = 0;
	}
    }
}

static void inc_score(byte amount) {
    for (byte i = LAST; i >= SCORE; i--) {
	spr[i].idx += amount;
	if (spr[i].idx >= 10) {
	    spr[i].idx -= 10;
	    amount = 1;
	}
	else break;
    }
}

static void sound_effect(void) {
    static const byte sfx[] = {
	0x35, 0x47, 0x54, 0x6a,
    };
    if (noise) {
	ticks[3] = 0;
	NOISE_LO(0x0a);
	NOISE_HI(0xf8);
	NOISE_VL(0x30 | --noise);
    }
    if (bling) {
	TRI_LO(sfx[bling >> 2]);
	TRI_HI(0x00);
	TRI_CR(0x0f);
	bling--;
    }
}

static void animate_score(void) {
    if (spr[DIGIT].idx != 0xff) {
	if (spr[DIGIT].y-- < 0x10) {
	    spr[DIGIT].idx = 0xff;
	}
	else if (spr[DIGIT].y == 0x14) {
	    spr[DIGIT].idx = 0x0b;
	}
    }
}

static void get_score(byte amount) {
    spr[DIGIT].x = spr[HEAD].x;
    spr[DIGIT].y = spr[HEAD].y;
    spr[DIGIT].idx = amount;
    spr[DIGIT].cfg = 0x01;
    inc_score(amount);
    bling = 0x0f;
}

static void loose_live(void) {
    if (--lives >= 0) spr[LIVES + lives].idx = 0xff;
}

static const byte tile_score[] = {
    0, 1, 2, 3, 4,
    SLIDE, SLIDE, SLIDE, SLIDE, SLIDE, SLIDE, SLIDE,
    SLIDE, SLIDE, SLIDE, SLIDE, SLIDE, SLIDE, SLIDE,
    JUMP, JUMP, JUMP, JUMP, JUMP, JUMP, JUMP,
    0, 0, 0, 0, 0, 0, 0, 0,
    /* speed 2x */
    1, 2, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static void hit_tile(byte hit) {
    bump = hit;
    if (bump) {
	safe = 9;
	if (bump < SLIDE) {
	    get_score(bump);
	}
	else if (bump == SLIDE) {
	    get_score(1);
	}
	else if (bump == JUMP) {
	    safe = 40;
	}
	else {
	    loose_live();
	    noise = 0x10;
	}
    }
}

static void prepare_readback(void) {
    byte i = line_table[line];
    BYTE(ppu_read, 0) = (i & 0xf0) + (pos >> 3);
    BYTE(ppu_read, 1) = (i & 0x0f) | 0x20;

    byte hit = MAX(ppu_buffer[0], ppu_buffer[1]);
    if (hit < SIZE(tile_score)) {
	if (safe < 0) {
	    speed = hit < START_OF_speed2x ? 1 : 2;
	}
	hit = tile_score[hit];
    }

    if (safe < 0) {
	hit_tile(hit);
    }
    else if (hit < SIZE(tile_score)) {
	safe -= speed;
    }
}

static const byte racoon_img[] = {
    0x34,0x10,0x00,0xf8, 0x34,0x11,0x00,0x00, 0x34,0x12,0x00,0x08,
    0x3c,0x20,0x00,0xf8, 0x3c,0x21,0x00,0x00, 0x3c,0x22,0x00,0x08,
    0x44,0x30,0x00,0xf8, 0x44,0x31,0x00,0x00, 0x44,0x32,0x00,0x08,
};

static byte adjust_img(byte i, byte offset) {
    switch (i & 3) {
    case 1:
	return offset;
    case 3:
	return pos;
    default:
	return 0;
    }
}

static void animate_racoon(byte offset) {
    for (byte i = 0; i < SIZE(racoon_img); i++) {
	oam[i] = racoon_img[i] + adjust_img(i, offset);
    }
}

static const byte score_img[] = {
    0x10,0x00,0x01,0x08,
    0x10,0x00,0x01,0x10,
    0x10,0x00,0x01,0x18,
    0x10,0x00,0x01,0x20,

    0x10,0x0a,0x01,0xf0,
    0x10,0x0a,0x01,0xe8,
    0x10,0x0a,0x01,0xe0,
};

static void show_score(void) {
    memcpy(oam + 4 * SCORE, score_img, SIZE(score_img));
}

#ifdef slope_pcx
#define LEVEL slope
#else
#define LEVEL testing
#endif

static void setup_snow(void) {
    spr[SNOW_L].y = spr[SNOW_R].y = 0x44;
}

static void start_new_game(void) {
    pos = 124;
    wipe_palette();
    reset_game();
    show_score();
    reset_rows(LEVEL, 36);
    animate_racoon(0);
    setup_palette();
    setup_snow();
    wait_button();
    speed = 1;
}

static void hide_snow(void) {
    spr[SNOW_L].idx = spr[SNOW_R].idx = 0xff;
}

#define ADJUST_SNOW(dx1, dx2, a1, a2) \
    spr[SNOW_L].x = pos - dx1; \
    spr[SNOW_R].x = pos + dx2; \
    spr[SNOW_L].cfg = a1; \
    spr[SNOW_R].cfg = a2;

static void animate_snow(byte n) {
    byte frame = (counter & 0xc) >> 2;
    spr[SNOW_L].idx = 12 + frame;
    spr[SNOW_R].idx = 12 + ((frame + 2) & 3);

    if (n == 3) {
	ADJUST_SNOW(6, 2, 0x00, 0x00);
    }
    else if (n == 9) {
	ADJUST_SNOW(2, 6, 0x40, 0x40);
    }
    else {
	ADJUST_SNOW(8, 8, 0x00, 0x40);
    }
}

static void check_controls(void) {
    byte i = bump ? 0x36 : 0x06;

    if (check_button() & BUTTON_SELECT) {
	mute = !mute;
    }

    if (button & SKI_RIGHT && pos < 236) {
	pos = pos + speed;
	i -= 3;
    }
    else if (button & SKI_LEFT && pos > 12) {
	pos = pos - speed;
	i += 3;
    }

    hide_snow();
    if (bump == SLIDE) {
	i = counter & 0x10 ? 0x0c : 0x3c;
    }
    else if (bump == JUMP) {
	i = counter & 0x08 ? 0x66 : 0x96;
    }
    else if (bump > SPECIAL) {
	i = 0x60;
    }
    else {
	animate_snow(i & 0xf);
    }

    animate_racoon(i);
}

static void game_loop(void) {
    wait_vblank();
    while (lives >= 0) {
	wait_signal();
	update_scroll();
	prepare_readback();
	produce_new_row();
	check_controls();
	animate_score();
	sound_effect();
	play_music();
    }
}

static const char *finish_str(void) {
    return is_bottom() ? "GAME DONE" : "GAME OVER";
}

#define ENTER_X	13
#define ENTER_Y	14

static void move_caret(byte caret) {
    spr[CARET].x = 8 * ENTER_X + (caret << 3);
    spr[CARET].y = 8 * ENTER_Y + 2;
    spr[CARET].idx = counter & 0x10 ? 0x1f : 0x2f;
    spr[CARET].cfg = 0x0;
}

static void update_char(byte *ptr, byte dir) {
    *ptr += dir;
    byte A = char_to_tile('A');
    byte Z = char_to_tile('Z');
    byte D = char_to_tile('.');
    if (*ptr == D + 1) {
	*ptr = A;
    }
    else if (*ptr == D - 1) {
	*ptr = Z;
    }
    else if (*ptr < A || *ptr > Z) {
	*ptr = D;
    }
}

static void display_char(byte *name, byte caret) {
    ppu_ptr = POS(ENTER_X, ENTER_Y) + caret;
    ppu_buffer[0] = name[caret];
    ppu_count = 1;
}

static void enter_new_record_name(byte *name) {
    byte caret = 0;

    memset(name, char_to_tile('.'), NAME_SIZE);
    memcpy(ppu_buffer, name, NAME_SIZE);
    ppu_ptr = POS(ENTER_X, ENTER_Y);
    ppu_count = NAME_SIZE;

    for (;;) {
	wait_signal();
	move_caret(caret);
	byte state = check_button();

	if (state & BUTTON_START) {
	    spr[CARET].idx = 0xff;
	    break;
	}
	else if (state & SKI_LEFT) {
	    caret = caret == 0 ? (NAME_SIZE - 1) : caret - 1;
	}
	else if (state & SKI_RIGHT) {
	    caret = caret == (NAME_SIZE - 1) ? 0 : caret + 1;
	}
	else if (state & BUTTON_DOWN) {
	    update_char(name + caret, 255);
	}
	else if (state & BUTTON_UP) {
	    update_char(name + caret, 1);
	}

	display_char(name, caret);
    }
}

static byte compare_score(byte *s1, byte *s2) {
    for (byte i = 0; i < sizeof(score); i++) {
	if (s1[i] > s2[i]) return 1;
	if (s1[i] < s2[i]) return 0;
    }
    return 0;
}

static void move_scores_down(int8 n) {
    for (int8 i = ENTRY_SIZE; i >= n; i -= ENTRY_SIZE) {
	memcpy(table + i + ENTRY_SIZE, table + i, ENTRY_SIZE);
    }
}

static void insert_score(byte *entry) {
    print_msg("CONGRATULATIONS", POS(9, 10));
    print_msg("ENTER YOUR NAME", POS(9, 12));
    memcpy(entry + NAME_SIZE, score, SIZE(score));
    enter_new_record_name(entry);
}

static void update_table(void) {
    for (int8 n = 0; n < 3 * ENTRY_SIZE; n += ENTRY_SIZE) {
	byte *entry = table + n;
	if (compare_score(score, entry + NAME_SIZE)) {
	    wipe_screen();
	    setup_palette();
	    move_scores_down(n);
	    insert_score(entry);
	    break;
	}
    }
}

static void copy_score_from_oam(void) {
    for (byte i = 0; i < SIZE(score); i++) {
	score[i] = char_to_tile(spr[SCORE + i].idx + '0');
    }
}

static const byte debris_img[] = {
    0x3c,0x4f,0x00,0xf8, 0x3c,0x4f,0x40,0x08,
    0x34,0x6f,0x00,0xf8, 0x34,0x6f,0x40,0x08,
};

static void setup_debris_sprites(void) {
    memcpy(oam + 4 * DEBRIS, debris_img, SIZE(debris_img));
    for (byte i = DEBRIS; i < DEBRIS + 4; i++) spr[i].x += pos;
}

static void update_oam(struct Sprite *s, int8 dir) {
    byte x = s->x;
    if (x > 0 && x < 248) {
	s->x = x + dir;
	if ((counter & 7) == 0) {
	    s->idx ^= 0x10;
	}
    }
    else {
	s->idx = 0xff;
    }
}

static void failure_slide(void) {
    animate_racoon(0x90);
    setup_debris_sprites();
    for (byte n = 0; n < 64; n++) {
	wait_signal();
	sound_effect();
	spr[TAIL].cfg = (n & 8) ? BIT(6) : 0;
	for (byte i = 0; i < 44; i += 4) {
	    oam[i]++;
	}
	struct Sprite *s = spr + DEBRIS;
	update_oam(s++, -1);
	update_oam(s++, +1);
	update_oam(s++, -1);
	update_oam(s++, +1);
    }
}

static void convert_live(void) {
    if (lives > 0 && spr[DIGIT].idx == 0xff) {
	get_score(9);
	loose_live();
	spr[DIGIT].idx = 10;
    }
    else if (spr[DIGIT].y == 0x24) {
	spr[DIGIT].idx = 9;
    }
}

static void animate_pump(void) {
    animate_racoon(counter & 16 ? 0x00 : 0x30);
}

static void victory_scene(void) {
    lives &= ~0x80;
    do {
	wait_signal();
	animate_pump();
	animate_score();
	convert_live();
	sound_effect();
    }
    while (spr[DIGIT].idx != 0xff);

    while (no_button_push()) {
	wait_signal();
	animate_pump();
    }
}

static void end_game_scene(void) {
    if (is_bottom()) {
	victory_scene();
    }
    else {
	failure_slide();
    }
    copy_score_from_oam();
}

static void show_final_score(void) {
    print_msg("SCORE:", POS(11, 15));
    memcpy(ppu_buffer, score, SIZE(score));
    ppu_ptr = POS(18, 15);
    ppu_update(SIZE(score));
}

static void stop_game(void) {
    print_msg(finish_str(), POS(12, 13));
    show_final_score();
    setup_palette();
    wait_button();
}

static void show_title_screen(void) {
#ifndef testing_pcx
    reset_rows(title, 30);
    show_highscore_table();
    setup_palette();
    wait_button();
#endif
}

static void play_note(byte cmd) {
    byte i = cmd & 0x7e;
    byte n = cmd & 0x01;
    byte r = (n << 2);
    MEM_WR(0x4002 | r, notes[i + 0]);
    MEM_WR(0x4003 | r, notes[i + 1]);
    ticks[n] = 12;
}

static void play_drum(byte val) {
    if (noise == 0) {
	NOISE_LO(val);
	NOISE_HI(0xf8);
	ticks[3] = 12;
    }
}

static void drum_hits(void) {
    if (drum == 0) {
	play_drum(0);
	drum = 24;
    }
    else if (drum == 12) {
	play_drum(3);
    }
    drum--;
}

static void music_notes(void) {
    byte cmd;
    for (;;) {
	cmd = melody[note++];
	if (cmd & 0x80) {
	    play_note(cmd);
	}
	else {
	    time = cmd;
	    break;
	}
    }
}

static const byte envelope[] = {
    0x30, 0x31, 0x32, 0x33,
    0x34, 0x36, 0x38, 0x3a,
    0x3c, 0x3e, 0x3f, 0x3e,

    0x30, 0x31, 0x31, 0x31,
    0x32, 0x32, 0x34, 0x36,
    0x38, 0x39, 0x3a, 0x3b,
};

static void stop_music(void) {
    for (byte reg = 0; reg < 0x10; reg += 4) {
	MEM_WR(0x4000 | reg, 0);
    }
}

static void set_volume(void) {
    static const byte offsets[] = { 0, 12, 0, 12 };
    for (byte i = 0; i < SIZE(ticks); i++) {
	if (ticks[i]) {
	    byte volume = envelope[--ticks[i] + offsets[i]];
	    MEM_WR(0x4000 + (i << 2), volume);
	}
    }
}

static void rewind_music(void) {
    if (melody[note] == REWIND) {
	if  (skip < note++) {
	    byte next = note;
	    note = skip;
	    skip = next;
	}
    }
}

static void play_melody(void) {
    if (time == 0) {
	music_notes();
	rewind_music();
	if (!melody[note]) {
	    skip = note = 0;
	}
    }
    time--;
}

static void play_music(void) {
    if (!mute) {
	drum_hits();
	play_melody();
    }
    set_volume();
}

void game_startup(void) {
    hw_init();

    for (;;) {
	wipe_screen();
	show_title_screen();
	start_new_game();
	game_loop();
	hide_snow();
	stop_music();
	end_game_scene();
	update_table();
	wipe_screen();
	stop_game();
    }
}

/* must be very last */
void end_of_code(void) __naked {
    __asm__(".area VECTOR (PAG)");
    __asm__("nmi_ptr::	.dw _nmi");
    __asm__("rst_ptr::	.dw _rst");
    __asm__("irq_ptr::	.dw _irq");
}
