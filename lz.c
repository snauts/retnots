#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define SIZE(array) (sizeof(array) / sizeof(*(array)))

static int estimate(int size) {
    return 2 * size + 128;
}

static int min(int a, int b) {
    return a < b ? a : b;
}

struct Node {
    int weight;
    int length;
    int offset;
};

static int update(struct Node *next, int estimate, int i, int j) {
    if (next->weight > estimate) {
	next->weight = estimate;
	next->length = i;
	next->offset = j;
    }
}

static int encode(void *dst, void *src, struct Node *nodes, int i) {
    int total = nodes[i].weight + 1;
    memset(dst, 0, total);

    while (i > 0) {
	unsigned char *ptr = dst;
	struct Node *next = nodes + i;
	if (next->offset < 1) {
	    ptr += next->weight - next->length;
	    memcpy(ptr, src - next->offset, next->length);
	    ptr[-1] = next->length;
	}
	else {
	    ptr += next->weight - 2;
	    ptr[0] = 0x80 | next->length;
	    ptr[1] = next->offset;
	}
	i -= next->length;
    }

    return total;
}

static int report(int size, int total) {
    int percent = total * 100 / size;
    fprintf(stderr, "compress %d to %d (%d%)\n", size, total, percent);
    return total;
}

int compress(void *dst, void *src, int size) {
    int max = estimate(size);
    struct Node nodes[max];

    nodes[0].weight = 0;
    for (int i = 1; i < max; i++) {
	nodes[i].weight = max;
    }

    for (int pos = 0; pos <= size; pos++) {
	unsigned char *ptr = src + pos;
	struct Node *current = nodes + pos;
	for (int i = 1; i < min(128, size - pos + 1); i++) {
	    struct Node *next = current + i;
	    if (next->weight <= current->weight) break;
	    update(next, current->weight + i + 1, i, -pos);
	    for (int j = 1; j < min(256, pos + 1); j++) {
		if (memcmp(ptr - j, ptr, i) == 0) {
		    update(next, current->weight + 2, i, j);
		}
	    }
	}
    }

    return report(size, encode(dst, src, nodes, size));
}
