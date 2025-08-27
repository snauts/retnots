#define NOTE(p, l)	(l), (14 * (p))
#define DONE		NOTE(0, 0)

#define C(l)		NOTE(0x0, l)
#define c(l)		NOTE(0x1, l)
#define D(l)		NOTE(0x2, l)
#define d(l)		NOTE(0x3, l)
#define E(l)		NOTE(0x4, l)
#define F(l)		NOTE(0x5, l)
#define f(l)		NOTE(0x6, l)
#define G(l)		NOTE(0x7, l)
#define Gs(l)		NOTE(0x8, l)
#define A(l)		NOTE(0x9, l)
#define a(l)		NOTE(0xa, l)
#define B(l)		NOTE(0xb, l)

static const byte melody[] = {
    A(2), f(2),
    A(2), f(2),
    G(1), E(1), G(1), E(1),
    f(2), f(2),

    E(1), E(1), E(1), E(1),
    f(1), f(1), f(1), f(1),
    G(1), f(1), G(1), E(1),
    D(1), D(1), D(2),
};
