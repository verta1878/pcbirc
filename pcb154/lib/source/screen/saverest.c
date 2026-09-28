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


#ifdef __OS2__
  #include <mem.h>
  #include <inline.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
//  #pragma inline
  #endif
  #ifdef BIOS
    #include "ansi.h"
  #endif
  #include "model.h"
#endif

#include "screen.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
 *
 *  Function:  savescreen()
 *
 *  Save the contents of the screen buffer to a variable
 */

void LIBENTRY savescreen(savescrntype _FAR_ *TempScrnBuff) {
#ifdef __OS2__
  memcpy(TempScrnBuff->Screen,Scrn_Buf,4000);
#elif defined(__WATCOMC__)
  /* Watcom: save screen to buffer */
  {
    unsigned short *vram = (unsigned short *)0xB8000;
    unsigned short *buf = (unsigned short *)TempScrnBuff;
    int i;
    for (i = 0; i < Scrn_Size16; i++)
      buf[i] = vram[i];
  }
#else  /*ifdef __OS2__ */
  asm  Push Ds
  asm  Cld

  NEEDSEGGETDS(Scrn_Addr);

  asm  Mov  Ax,80                  /* calculate the number of words in the */
  asm  Mov  Bl,Scrn_BottomRow      /* video memory according to the number */
  asm  Inc  Bl                     /* of lines currently on the screen.    */
  asm  Mul  Bl
  asm  Mov  Cx,Ax                  /* CX = number of WORDS in video memory */

  asm  Mov  Al,Scrn_Rtrc
  asm  Les  Di,TempScrnBuff        /* Es:Di points to TempScrnBuff */
  asm  Lds  Si,dword ptr Scrn_Addr /* Ds:Si points to Screen buffer */

  asm  And  Al,Al
  asm  Jz   J2
  asm  Mov  Dx,03DAh
  asm  Mov  Ah,01

J1:
  asm  In   Al,Dx
  asm  And  Al,Ah
  asm  Jz   J1
  asm  Movsw
  asm  Loop J1
  asm  Jmp  short J3

J2:
  asm  Rep  Movsw

J3:
  asm  Pop  Ds
#endif  /* ifdef __OS2__ */
}


/********************************************************************
 *
 *  Function:  restorescreen()
 *
 *  Restore the contents of the screen buffer from a variable
 */

void LIBENTRY restorescreen(savescrntype _FAR_ *TempScrnBuff) {
#ifdef __OS2__

  memcpy(Scrn_Buf,TempScrnBuff->Screen,Scrn_SizeBytes);
  updatelines(Scrn_BottomRow == 24 ? UPDATE_25LINES : UPDATE_25LINES,0,Scrn_BottomRow);

#elif defined(__WATCOMC__)
  {
    unsigned short *vram = (unsigned short *)0xB8000;
    unsigned short *buf = (unsigned short *)TempScrnBuff;
    int i;
    for (i = 0; i < Scrn_Size16; i++)
      vram[i] = buf[i];
  }
#else  /*ifdef __OS2__ */

#ifdef BIOS
  int  LastColor;
  int  NextColor;
  int  Count;
  char far *p;

  LastColor = 0xffff;
  ansi_save();
  gotoxy(0,0);

  p = (char far *) TempScrnBuff;
  for (Count = 0; Count < 1999; Count++, p+=2) {  /* don't print the last   */
    NextColor = *(p+1);                           /* one cuz it will scroll */
    if (NextColor != LastColor)
      ansi_color(NextColor);

    _DL = *p;

    asm Mov  Ah,2
    asm Int  21h;
  }
  ansi_rest();

#else  /* ifdef BIOS */

  asm  Push Ds
  asm  Cld

  NEEDSEGGETDS(Scrn_Addr);

  asm  Mov  Ax,80                  /* calculate the number of words in the */
  asm  Mov  Bl,Scrn_BottomRow      /* video memory according to the number */
  asm  Inc  Bl                     /* of lines currently on the screen.    */
  asm  Mul  Bl
  asm  Mov  Cx,Ax                  /* CX = number of WORDS in video memory */

  asm  Mov  Al,Scrn_Rtrc
  asm  Les  Di,dword ptr Scrn_Addr /* Es:Di points to Screen Buffer */
  asm  Lds  Si,TempScrnBuff        /* Ds:Si points to TempScrnBuff */

  asm  And  Al,Al
  asm  Jz   J2
  asm  Mov  Dx,03DAh
  asm  Mov  Ah,01

J1:
  asm  In   Al,Dx
  asm  And  Al,Ah
  asm  Jz   J1
  asm  Movsw
  asm  Loop J1
  asm  Jmp  short J3

J2:
  asm  Rep  Movsw

J3:
  asm  Pop  Ds
#endif  /* ifdef BIOS */
#endif  /* ifdef __OS2__ */
}

