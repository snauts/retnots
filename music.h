#define R(x)		(0xc0 | (x))
#define N(p)		(0x40 | (4 * p))

#define cD		(0x80 | (4 * 2))
#define cA		(0x80 | (4 * 9))

#define Cn		N(0x0)
#define Cs		N(0x1)
#define Dn		N(0x2)
#define Ds		N(0x3)
#define En		N(0x4)
#define Fn		N(0x5)
#define Fs		N(0x6)
#define Gn		N(0x7)
#define Gs		N(0x8)
#define An		N(0x9)
#define As		N(0xa)
#define Bn		N(0xb)

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
