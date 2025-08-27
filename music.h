#define NOTE(p)		(0x40 | (5 * p))

#define Cn		NOTE(0x0)
#define Cs		NOTE(0x1)
#define Dn		NOTE(0x2)
#define Ds		NOTE(0x3)
#define En		NOTE(0x4)
#define Fn		NOTE(0x5)
#define Fs		NOTE(0x6)
#define Gn		NOTE(0x7)
#define Gs		NOTE(0x8)
#define An		NOTE(0x9)
#define As		NOTE(0xa)
#define Bn		NOTE(0xb)

static const byte chord_D[] = { 20, 60, 90 };
static const byte chord_A[] = { 90, 130, 160 };

#define cD		0x80
#define cA		0x81

static const byte melody[] = {
    cD, An, 2, Fs, 2,
        An, 2, Fs, 2,
    cA, Gn, 1, En, 1, Gn, 1, En, 1,
    cD, Fs, 2, Fs, 2,

    cD, An, 2, Fs, 2,
        An, 2, Fs, 2,
    cA, Gn, 1, En, 1, Gn, 1, En, 1,
    cD, Fs, 2, Fs, 2,

    cA, En, 1, En, 1, En, 1, En, 1,
    cD, Fs, 1, Fs, 1, Fs, 1, Fs, 1,
    cA, Gn, 1, Fs, 1, Gn, 1, En, 1,
    cD, Dn, 1, Dn, 1, Dn, 2,

    cA, En, 1, En, 1, En, 1, En, 1,
    cD, Fs, 1, Fs, 1, Fs, 1, Fs, 1,
    cA, Gn, 1, Fs, 1, Gn, 1, En, 1,
    cD, Dn, 1, Dn, 1, Dn, 2,

    0,
};
