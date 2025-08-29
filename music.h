#define REWIND		0xff
#define N(p)		(0x80 | (8 * p))

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

#define cD		(1 | Dn)
#define cFs		(1 | Fs)
#define cA		(1 | An)
#define cCs		(1 | Cs + (byte) 2)
#define cE		(1 | En + (byte) 2)

static const byte melody[] = {
    cD, An, 3, cFs, 3, cA, 18, Fs, 24,
        An,                24, Fs, 24,
    cA, Gn, 3, cCs, 3, cE,  6, En, 12, En, 12, Gn, 12,
    cD, Fs, 3, cFs, 3, cA, 18, Fs, 24,
    REWIND,

    cA, En, 3, cCs, 3, cE, 6, En, 12, En, 12, En, 12,
    cD, Fs, 3, cFs, 3, cA, 6, Fs, 12, Fs, 12, Fs, 12,
    cA, Gn, 3, cCs, 3, cE, 6, Fs, 12, Gn, 12, En, 12,
    cD, Dn, 3, cFs, 3, cA, 6, Dn, 12, Dn, 24,
    REWIND,

    0,
};
