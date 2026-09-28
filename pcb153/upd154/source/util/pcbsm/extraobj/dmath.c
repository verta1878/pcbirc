/*  DMATH.C — 64-bit multiply/divide helpers, plain-C reimplementation.       */
/*  Replaces Clark's inline-asm version.  hexadecimal, v0.032.               */

typedef struct { long lo; long hi; } dlong;

void pascal dmul(long Arg1, long Arg2, dlong *Product) {
    unsigned long a, b, p0, p1, p2, p3, lo, mid, hi, rlo, rhi;
    int neg = 0;
    long aa = Arg1, bb = Arg2;
    if (aa < 0) { neg ^= 1; aa = -aa; }
    if (bb < 0) { neg ^= 1; bb = -bb; }
    a = (unsigned long)aa; b = (unsigned long)bb;
    p0 = (a & 0xFFFFL) * (b & 0xFFFFL);
    p1 = (a & 0xFFFFL) * (b >> 16);
    p2 = (a >> 16) * (b & 0xFFFFL);
    p3 = (a >> 16) * (b >> 16);
    lo = p0 & 0xFFFFL;
    mid = (p0 >> 16) + (p1 & 0xFFFFL) + (p2 & 0xFFFFL);
    hi = (mid >> 16) + (p1 >> 16) + (p2 >> 16) + p3;
    rlo = lo | ((mid & 0xFFFFL) << 16);
    rhi = hi;
    if (neg) { rlo = ~rlo + 1; rhi = ~rhi + (rlo == 0 ? 1 : 0); }
    Product->lo = (long)rlo;
    Product->hi = (long)rhi;
}

long pascal ddiv(dlong *Dividend, long Divisor) {
    long q = 0, r = 0;
    unsigned long lo, hi;
    int i, neg = 0;
    if (Divisor == 0) return 0;
    lo = (unsigned long)Dividend->lo;
    hi = (unsigned long)Dividend->hi;
    if ((long)hi < 0) { neg ^= 1; lo = ~lo + 1; hi = ~hi + (lo == 0 ? 1 : 0); }
    if (Divisor < 0) { neg ^= 1; Divisor = -Divisor; }
    for (i = 63; i >= 0; i--) {
        r <<= 1;
        if (i >= 32) { if (hi & (1UL << (i-32))) r |= 1; }
        else         { if (lo & (1UL << i))       r |= 1; }
        if ((unsigned long)r >= (unsigned long)Divisor) {
            r -= Divisor;
            if (i < 32) q |= (1L << i);
        }
    }
    return neg ? -q : q;
}
