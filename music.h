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

#define cD		(0x80 | (5 * 2))
#define cA		(0x80 | (5 * 9))

#define R(x)		(0xc0 | (x))

static const byte melody[] = {
    cD, An, R(0), 24,               Fs, R(9), 24,
        An, R(0), 24,               Fs, R(9), 24,
    cA, Gn, R(0), 12, En,       12, Gn, R(9), 12, En,       12,
    cD, Fs, R(0), 24,               Fs, R(9), 24,

    cD, An, R(0), 24,               Fs, R(9), 24,
        An, R(0), 24,               Fs, R(9), 24,
    cA, Gn, R(0), 12, En,       12, Gn, R(9), 12, En,       12,
    cD, Fs, R(0), 24,               Fs, R(9), 24,

    cA, En, R(0), 12, En, R(9), 12, En, R(0), 12, En, R(9), 12,
    cD, Fs, R(0), 12, Fs, R(9), 12, Fs, R(0), 12, Fs, R(9), 12,
    cA, Gn, R(0), 12, Fs, R(9), 12, Gn, R(0), 12, En, R(9), 12,
    cD, Dn, R(0), 12, Dn, R(9), 12, Dn, R(9), 24,

    cA, En, R(0), 12, En, R(9), 12, En, R(0), 12, En, R(9), 12,
    cD, Fs, R(0), 12, Fs, R(9), 12, Fs, R(0), 12, Fs, R(9), 12,
    cA, Gn, R(0), 12, Fs, R(9), 12, Gn, R(0), 12, En, R(9), 12,
    cD, Dn, R(0), 12, Dn, R(9), 12, Dn, R(9), 24,

    0,
};
