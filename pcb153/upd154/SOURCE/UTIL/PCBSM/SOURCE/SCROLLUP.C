/*  SCROLLUP.C — plain-C replacement for Clark's inline-asm scrollup.        */
/*  Uses BIOS INT 10h AH=06 to scroll a rectangular region.                  */
/*  hexadecimal, v0.032.                                                    */

#include <dos.h>

void pascal scrollup(int X1, int Y1, int X2, int Y2, unsigned char Color) {
    union REGS r;
    r.h.ah = 0x06;      /* scroll up */
    r.h.al = 0x01;      /* one line */
    r.h.bh = Color;
    r.h.ch = (unsigned char)Y1;
    r.h.cl = (unsigned char)X1;
    r.h.dh = (unsigned char)Y2;
    r.h.dl = (unsigned char)X2;
    int86(0x10, &r, &r);
}
