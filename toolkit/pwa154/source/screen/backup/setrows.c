/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* The source code in this module is proprietary software belonging to       */
/* Clark Development Company and is part of the PCBoard source code library. */
/* You are granted the right to use this source code for the building of any */
/* of the PCBoard products you have licensed.  Any other usage is forbidden  */
/* without prior written consent from Clark Development Company, Inc.        */
/*                                                                           */
/* Be sure to read the source code license agreement before utilizing any    */
/* of the source code found herein.                                          */
/*                                                                           */
/* Copyright (C) 1996  Clark Development Company, Inc.  All Rights Reserved. */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/


#include "screen.h"

#ifndef __OS2__
  #pragma inline
#endif

void LIBENTRY setrows(int NumRows) {
  if (Scrn_Adapter != VID_EGA && Scrn_Adapter != VID_VGA)
    return;

#ifdef __OS2__
  setviolines(NumRows);
#else

  asm    mov     bx,NumRows
  asm    cmp     bx,25            /* are we setting it back to 25 lines? */
  asm    je      lines25          /*   yes, go do 25 lines */
  asm    cmp     bx,50            /* are we setting it for 50 lines? */
  asm    je      lines50          /*   yes, go do 25 lines */
  asm    cmp     bx,43            /* are we setting it for 43 lines? */
  asm    jne     end              /*   no, get out of here */

lines50:
  asm    mov     ah,0Fh           /* get video state */
  asm    int     10h
  asm    xor     ah,ah            /* set display mode */
  asm    int     10h
  asm    mov     ax,1112h         /* set character set */
  asm    xor     bl,bl
  asm    int     10h
  asm
  asm    xor     ax,ax
  asm    mov     es,ax
  asm    mov     es:[044Ch],al    /* (0000:044C = al) */
  asm    mov     dl,es:[0487h]    /* (dl = 0000:0487) */
  asm    mov     ah,1
  asm    or      es:[0487h],ah    /* (0000:0487) |= 1 */

  asm    mov     cx,607h          /* set cursor mode */
  asm    int     10h

  asm    mov     es:[0487h],dl    /* (0000:0487 = dl) */

  Scrn_BottomRow = (NumRows-1);
  goto end;

lines25:
  asm    mov     ah,0Fh
  asm    int     10h              /* Video display   ah=functn 0Fh */
                                  /*  get state, al=mode, bh=page  */
  asm    xor     ah,ah            /* Zero register                 */
  asm    int     10h              /* Video display   ah=functn 00h */
                                  /*  set display mode in al       */
  Scrn_BottomRow = 24;

end:;
#endif  /* ifdef __OS2__ */
}

char LIBENTRY getrows(void) {
#ifdef __OS2__
  return((char) getviolines());
#else
  asm xor ax,ax
  asm mov es,ax
  asm mov al,es:[0484h]
  asm inc al
  return(_AL);
#endif
}
