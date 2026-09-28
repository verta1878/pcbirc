/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* SETROWS.C — Set screen rows (25/43/50)                                    */
/* Clark Development Company, Inc. (C) 1996. All Rights Reserved.            */
/* Watcom C conversion by pcbrevival (GPL v3.0 for our additions)            */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "screen.h"
#ifdef __WATCOMC__
#include <i86.h>
#endif

#ifdef DEBUG
#include <memcheck.h>
#endif

void LIBENTRY setrows(int NumRows) {
  if (Scrn_Adapter != VID_EGA && Scrn_Adapter != VID_VGA)
    return;

#ifdef __OS2__
  setviolines(NumRows);
#elif defined(__WATCOMC__)
  /* Watcom DOS flat mode: screen rows via BIOS INT 10h */
  {
    union REGS r;
    if (NumRows == 25) {
      r.w.ax = 0x0003;  /* Set 80x25 text mode */
      int386(0x10, &r, &r);
    } else if (NumRows >= 43) {
      r.w.ax = 0x1112;  /* Load 8x8 font → 43/50 lines */
      r.h.bl = 0;
      int386(0x10, &r, &r);
    }
  }
#else
  /* Borland DOS real mode — inline ASM */

  asm    mov     bx,NumRows
  asm    cmp     bx,25
  asm    je      Use25
  asm    cmp     bx,28
  asm    je      Use28

  asm    mov     ax,1112h
  asm    xor     bl,bl
  asm    int     10h
  asm    jmp     short setrdone

Use28:
  asm    mov     ax,1111h
  asm    xor     bl,bl
  asm    int     10h
  asm    jmp     short setrdone

Use25:
  asm    mov     ax,0003h
  asm    int     10h

setrdone:

#endif
}

int LIBENTRY getactualrows(void) {
#if defined(__OS2__) || defined(__WATCOMC__)
  return 25; /* TODO: query actual rows */
#else
  int rows;
  asm    xor     bh,bh
  asm    mov     dl,24
  asm    mov     ax,1130h
  asm    int     10h
  asm    inc     dl
  asm    xor     dh,dh
  asm    mov     rows,dx
  return rows;
#endif
}
