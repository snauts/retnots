#define R(x)		(0xc0 | (x))
#define N(p)		(0x40 | (4 * p))

#define cD		(0x80 | (4 * 2))
#define cFs		(0x80 | (4 * 6))
#define cA		(0x80 | (4 * 9))
#define cCs		(0x80 | (4 * 1 + 1))
#define cE		(0x80 | (4 * 4 + 1))

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
    cD, An, R(0), 3, cFs, 3, cA, 6,     R(9), 12, Fs, R(0), 12,     R(9), 12,
        An, R(0),               12,     R(9), 12, Fs, R(0), 12,     R(9), 12,
    cA, Gn, R(0), 3, cCs, 3, cE, 6, En, R(9), 12, En, R(0), 12, Gn, R(9), 12,
    cD, Fs, R(0), 3, cFs, 3, cA, 6,     R(9), 12, Fs, R(0), 12,     R(9), 12,

    cD, An, R(0), 3, cFs, 3, cA, 6,     R(9), 12, Fs, R(0), 12,     R(9), 12,
        An, R(0),               12,     R(9), 12, Fs, R(0), 12,     R(9), 12,
    cA, Gn, R(0), 3, cCs, 3, cE, 6, En, R(9), 12, En, R(0), 12, Gn, R(9), 12,
    cD, Fs, R(0), 3, cFs, 3, cA, 6,     R(9), 12, Fs, R(0), 12,     R(9), 12,

    cA, En, R(0), 3, cCs, 3, cE, 6, En, R(9), 12, En, R(0), 12, En, R(9), 12,
    cD, Fs, R(0), 3, cFs, 3, cA, 6, Fs, R(9), 12, Fs, R(0), 12, Fs, R(9), 12,
    cA, Gn, R(0), 3, cCs, 3, cE, 6, Fs, R(9), 12, Gn, R(0), 12, En, R(9), 12,
    cD, Dn, R(0), 3, cFs, 3, cA, 6, Dn, R(9), 12, Dn, R(0), 12,     R(9), 12,

    cA, En, R(0), 3, cCs, 3, cE, 6, En, R(9), 12, En, R(0), 12, En, R(9), 12,
    cD, Fs, R(0), 3, cFs, 3, cA, 6, Fs, R(9), 12, Fs, R(0), 12, Fs, R(9), 12,
    cA, Gn, R(0), 3, cCs, 3, cE, 6, Fs, R(9), 12, Gn, R(0), 12, En, R(9), 12,
    cD, Dn, R(0), 3, cFs, 3, cA, 6, Dn, R(9), 12, Dn, R(0), 12,     R(9), 12,
    0,
};
