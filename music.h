#define NOTE(p, l)	(0x40 | (5 * p)), (l)

#define C(l)		NOTE(0x0, l)
#define c(l)		NOTE(0x1, l)
#define D(l)		NOTE(0x2, l)
#define d(l)		NOTE(0x3, l)
#define E(l)		NOTE(0x4, l)
#define F(l)		NOTE(0x5, l)
#define f(l)		NOTE(0x6, l)
#define G(l)		NOTE(0x7, l)
#define g(l)		NOTE(0x8, l)
#define A(l)		NOTE(0x9, l)
#define a(l)		NOTE(0xa, l)
#define B(l)		NOTE(0xb, l)

static const byte chord_D[] = { 20, 60, 90 };
static const byte chord_A[] = { 90, 130, 160 };

#define cD		0x80
#define cA		0x81

static const byte melody[] = {
    cD, A(2), f(2),
        A(2), f(2),
    cA, G(1), E(1), G(1), E(1),
    cD, f(2), f(2),

    cD, A(2), f(2),
        A(2), f(2),
    cA, G(1), E(1), G(1), E(1),
    cD, f(2), f(2),

    cA, E(1), E(1), E(1), E(1),
    cD, f(1), f(1), f(1), f(1),
    cA, G(1), f(1), G(1), E(1),
    cD, D(1), D(1), D(2),

    cA, E(1), E(1), E(1), E(1),
    cD, f(1), f(1), f(1), f(1),
    cA, G(1), f(1), G(1), E(1),
    cD, D(1), D(1), D(2),

    0,
};
