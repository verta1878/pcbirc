/* PCBTITLE — set OS/2 console window title via INT 21h/6400h */
/* Watcom port: replaced Borland inline asm with int386x */
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include <i86.h>

char Title[] = "NODE - XXXX";

void main(void) {
  char *p;
  if (getenv("PCBOS2") != NULL) {
    if ((p = getenv("PCBNODE")) != NULL) {
      union REGS r;
      struct SREGS sr;
      strncpy(Title+7, p, 4); Title[11] = 0;
      r.w.ax = 0x6400;
      r.w.bx = 0;
      r.w.cx = 0x636C;
      r.w.dx = 1;
      r.x.edi = (unsigned int)Title;
      segread(&sr);
      int386x(0x21, &r, &r, &sr);
    }
  }
}
