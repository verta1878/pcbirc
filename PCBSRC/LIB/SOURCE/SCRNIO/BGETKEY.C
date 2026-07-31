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


#ifndef __OS2__
//#pragma inline
#endif

#include <scrnio.h>

#ifdef __OS2__
#define INCL_KBD
#include <os2.h>

short KbdShiftStatus;

void LIBENTRY enablekbdmonitor(void) {
  KBDINFO Kbd;

  Kbd.cb = sizeof(Kbd);
  KbdGetStatus(&Kbd,0);
  Kbd.fsMask |= (unsigned short) KEYBOARD_SHIFT_REPORT|KEYBOARD_BINARY_MODE;
  KbdSetStatus(&Kbd,0);
}

void LIBENTRY disablekbdmonitor(void) {
  KBDINFO Kbd;

  Kbd.cb = sizeof(Kbd);
  KbdGetStatus(&Kbd,0);
  Kbd.fsMask &= (unsigned short) ~KEYBOARD_SHIFT_REPORT;
  KbdSetStatus(&Kbd,0);
}


int LIBENTRY bgetkey(char Option) {
  KBDKEYINFO Kbd;

  if (Option == 1) {
    KbdPeek(&Kbd,0);
    KbdShiftStatus = Kbd.fsState;
    return(Kbd.fbStatus & 0x40);
  }

  KbdCharIn(&Kbd,0,0);
  KbdShiftStatus = Kbd.fsState;
  if (Kbd.fbStatus & 0x40) {
    if (Kbd.chChar == 0xE0)
      Kbd.chChar = 0;
    return(Kbd.chChar + ((int) Kbd.chScan << 8));
  }
  return(0);
}

#else  /* ifdef __OS2__ */

int LIBENTRY bgetkey(char Option) {
asm   Cmp   byte ptr Option,1   /* are we merely TESTING the keyboard? */
asm   Jne   GetKey              /*   no, go get the keystroke          */
asm   Mov   Ah,1                /*   yes, ask BIOS for the scan code   */
asm   Int   16h
asm   Jnz   GotOne              /* if Z-flag is set then we don't have */
      return(0);                /* one so return FALSE otherwise       */
GotOne:                         /* return TRUE to indicate there's a   */
      return(1);                /* keystroke waiting to be picked up   */

GetKey:
asm  Mov    Ah,Option
asm  Int    16h
     return(_AX);
}
#endif
