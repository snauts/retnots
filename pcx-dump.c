#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

#define MAX_SIZE 4096

static char *file_name;

static unsigned char tileset[MAX_SIZE];
static int tileset_size;
static char option;

struct Header {
    unsigned short w, h;
} header;

static int buf_size(void) {
    return header.w * header.h;
}

static int file_size(const char *file) {
    struct stat st;
    return stat(file, &st) != 0 ? -1 : st.st_size;
}

static void replace_ext(char *name, const char *ext) {
    strcpy(name, file_name);
    strcpy(name + strlen(file_name) - strlen(ext), ext);
}

static unsigned char *read_pcx(const char *file) {
    int palette_offset = 16;
    int size = file_size(file);

    if (size < 0) {
	fprintf(stderr, "ERROR while opening PCX-file \"%s\"\n", file);
	return NULL;
    }

    unsigned char *buf = malloc(size);
    int in = open(file, O_RDONLY);
    read(in, buf, size);
    close(in);

    header.w = (* (unsigned short *) (buf + 0x8)) + 1;
    header.h = (* (unsigned short *) (buf + 0xa)) + 1;
    if (buf[3] == 8) palette_offset = size - 768;
    int unpacked_size = buf_size() / (buf[3] == 8 ? 1 : 2);
    unsigned char *pixels = malloc(unpacked_size);

    int i = 128, j = 0;
    while (j < unpacked_size) {
	if ((buf[i] & 0xc0) == 0xc0) {
	    int count = buf[i++] & 0x3f;
	    while (count-- > 0) {
		pixels[j++] = buf[i];
	    }
	    i++;
	}
	else {
	    pixels[j++] = buf[i++];
	}
    }

    free(buf);
    return pixels;
}

static unsigned char get_bit_line(unsigned char *buf, int plane) {
    unsigned char byte = 0;
    for (int i = 0; i < 8; i++) {
	if (buf[i] & plane) {
	    byte |= (0x80 >> i);
	}
    }
    return byte;
}

static void get_bit_plane(unsigned char *buf, unsigned char *ptr, int plane) {
    for (int y = 0; y < 8; y++) {
	ptr[y] = get_bit_line(buf + y * header.w, plane);
    }
}

static void save_sprites(unsigned char *buf) {
    char tile_name[256];
    replace_ext(tile_name, "chr");
    int fd = open(tile_name, O_CREAT | O_RDWR, 0644);
    for (int y = 0; y < header.h; y += 8) {
	for (int x = 0; x < header.w; x += 8) {
	    unsigned char result[16];
	    unsigned char *ptr = buf + y * header.w + x;
	    get_bit_plane(ptr, result + 0, 1);
	    get_bit_plane(ptr, result + 8, 2);
	    write(fd, result, 16);
	}
    }
    close(fd);
}

static int reset_tiles(void) {
    unsigned char buf[16];
    int fd = open(file_name, O_CREAT | O_TRUNC | O_RDWR, 0644);
    memset(buf, 0, sizeof(buf));
    write(fd, buf, sizeof(buf));
    close(fd);
    return 0;
}

static int pad_tiles(void) {
    int size = file_size(file_name);

    if (size < 0 || size > MAX_SIZE) {
	fprintf(stderr, "ERROR invalid tile file \"%s\"\n", file_name);
	return -ENOENT;
    }

    unsigned char *buf = malloc(MAX_SIZE);
    memset(buf, 0, MAX_SIZE);

    int fd = open(file_name, O_RDWR, 0644);
    read(fd, buf, size);
    lseek(fd, 0, SEEK_SET);
    write(fd, buf, MAX_SIZE);
    close(fd);
    return 0;
}

static void load_tileset(void) {
    tileset_size = file_size("tiles.chr");
    int fd = open("tiles.chr", O_RDWR, 0644);
    read(fd, tileset, tileset_size);
    close(fd);
}

static void save_tileset(void) {
    int fd = open("tiles.chr", O_RDWR, 0644);
    write(fd, tileset, tileset_size);
    close(fd);
}

static int tile_cmp(unsigned char *a, unsigned char *b) {
    for (int i = 0; i < 16; i++) {
	if (a[i] != b[i]) return 0;
    }
    return 1;
}

static int look_up_tile(unsigned char *buf) {
    for (int i = 0; i < tileset_size; i += 16) {
	if (tile_cmp(buf, tileset + i)) {
	    return i / 16;
	}
    }
    if (tileset_size < MAX_SIZE) {
	memcpy(tileset + tileset_size, buf, 16);
	tileset_size += 16;
    }
    return (tileset_size / 16) - 1;
}

static int get_equal(unsigned char *buf, int max) {
    for (int i = 1; i < max; i++) {
	if (buf[0] != buf[i]) {
	    return i;
	}
    }
    return max;
}

static int get_diffs(unsigned char *buf, int max) {
    for (int i = 0; i < max; i++) {
	if (get_equal(buf + i, MIN(max, 3)) > 2) {
	    return i;
	}
    }
    return max;
}

static void print_hex(FILE *fp, void *ptr, int *size, int w) {
    ptr += *size * w;
    switch (w) {
    case 1:
	fprintf(fp, " 0x%02x,", * (unsigned char *) ptr);
	break;
    case 2:
	fprintf(fp, " 0x%04x,", * (unsigned short *) ptr);
	break;
    }
    if (((*size)++ & 0x7) == 0x7) {
	fprintf(fp, "\n");
    }
}

static int generate_data_map(unsigned char *buf, unsigned char *map) {
    int count = 0;

    for (int y = 0; y < header.h; y += 8) {
	for (int x = 0; x < header.w; x += 8) {
	    unsigned char result[16];
	    unsigned char *ptr = buf + y * header.w + x;
	    get_bit_plane(ptr, result + 0, 1);
	    get_bit_plane(ptr, result + 8, 2);
	    int tile_id = look_up_tile(result);
	    if (map) map[count++] = tile_id;
	}
    }

    return count;
}

static int look_up_attr(unsigned char *buf, int chunk, int offset) {
    int attr = 0;

    if (offset < header.w * chunk) {
	buf += offset;
    }
    else {
	return 0;
    }

    for (int y = 0; y < 16; y++) {
	for (int x = 0; x < 16; x++) {
	    int offset = x + y * header.w;
	    attr = MAX(attr, (buf[offset] >> 2) & 3);
	}
    }
    return attr;
}

static int attr_block(unsigned char *buf, int chunk, int where) {
    int result = 0, offset[] = {
	header.w * 16 + 16, header.w * 16, 16, 0
    };
    for (int i = 0; i < 4; i++) {
	result = (result << 2) | look_up_attr(buf, chunk, where + offset[i]);
    }
    return result;
}

static int generate_attr_map(unsigned char *buf, unsigned char *map) {
    int i = 0;
    int h = header.h;
    while (h > 0) {
	int chunk = MIN(240, h);
	for (int y = 0; y < chunk; y += 32) {
	    for (int x = 0; x < header.w; x += 32) {
		map[i++] = attr_block(buf, chunk, y * header.w + x);
	    }
	}
	buf += header.w * 240;
	h -= chunk;
    }
    return i;
}

static void save_hex(FILE *fp, void *map, int count, int w) {
    int i = 0;
    while (i < count) {
	print_hex(fp, map, &i, w);
    }
    if (i & 7) {
	fprintf(fp, "\n");
    }
}

static void save_map(FILE *fp, void *map, int count) {
    save_hex(fp, map, count, 1);
}

static unsigned char pack_addr(int a) {
    return (a & 0xf0) | ((a >> 8) & 0xf);
}

static int generate_rows(unsigned char *rows, int attrs) {
    int i = 0;
    for (int p = 0; p < 2; p++) {
	for (int n = 0; n < 30; n++) {
	    int base = 0x2000 + (p << 11);
	    if (attrs && (n & 0xf) == 0) {
		rows[i++] = pack_addr(base + 0x3c0 + ((n & 0x10) << 1));
	    }
	    rows[i++] = pack_addr(base + (n << 5));
	}
    }
    return i;
}

int compress(void *dst, void *src, int size);

static void save_row_tables(FILE *fp) {
    unsigned char row_table[64];
    fprintf(fp, "static const byte row_table[] = {\n");
    save_map(fp, row_table, generate_rows(row_table, 1));
    fprintf(fp, "};\n");

    fprintf(fp, "static const byte line_table[] = {\n");
    save_map(fp, row_table, generate_rows(row_table, 0));
    fprintf(fp, "};\n");
}

static void generate_level_data(unsigned char *buf) {
    char name[256];
    replace_ext(name, "hdr");
    FILE *fp = fopen(name, "w");
    name[strlen(name) - 4] = 0;

    int addr[header.h];

    int max_tiles = buf_size() / 64;
    unsigned char tile[max_tiles];
    unsigned char attr[max_tiles];

    int tile_count = generate_data_map(buf, tile);
    int attr_count = generate_attr_map(buf, attr);

    unsigned char data[2 * max_tiles];
    unsigned char done[2 * max_tiles];

    int size = 0, chunk = 512;
    unsigned char *pt = tile;
    unsigned char *pa = attr;
    void *copy_chunk(void *dst, void *src, int n) {
	memcpy(dst + size, src, n);
	size = size + n;
	return src + n;
    }
    while (tile_count > 0) {
	pa = copy_chunk(data, pa, 32);
	pt = copy_chunk(data, pt, chunk);
	tile_count -= chunk;
	chunk = 960 - chunk;
    }

    fprintf(fp, "static const byte %s[] = {\n", name);
    save_map(fp, done, compress(done, data, size));
    fprintf(fp, "};\n");

    if (strcmp(file_name, "slope.pcx") == 0) {
	save_row_tables(fp);
    }

    fclose(fp);
}

static void output_font_offset(void) {
    int offset = tileset_size / 16;
    FILE *fp = fopen("fonts.hdr", "w");
    fprintf(fp, "#define FONT_START %d\n", offset);
    fclose(fp);
}

static void save_tiles(unsigned char *buf) {
    load_tileset();
    if (strcmp(file_name, "fonts.pcx") == 0) {
	output_font_offset();
    }
    if (option == 'l') {
	generate_level_data(buf);
    }
    else {
	generate_data_map(buf, NULL);
    }
    save_tileset();
}

int main(int argc, char **argv) {
    if (argc < 3) {
	printf("USAGE: pcx-dump [option] file.pcx\n");
	printf("  -r   reset tiles\n");
	printf("  -t   save tiles\n");
	printf("  -l   save level\n");
	printf("  -p   pad tiles\n");
	printf("  -s   save sprites\n");
	return 0;
    }

    file_name = argv[2];
    option = argv[1][1];

    printf("pcx-dump -%c %s\n", option, argv[2]);

    switch (option) {
    case 'r':
	return reset_tiles();
    case 'p':
	return pad_tiles();
    }

    unsigned char *buf = read_pcx(file_name);
    if (buf == NULL) return -ENOENT;

    switch (option) {
    case 't':
    case 'l':
	save_tiles(buf);
	break;
    case 's':
	save_sprites(buf);
	break;
    }

    free(buf);
    return 0;
}
