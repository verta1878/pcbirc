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
/*#pragma inline */
#endif

#include "system.h"

#ifdef TEST
  #include <stdio.h>
#endif

#ifdef __OS2__
#define INCL_KBD
#define INCL_DOSDEVICES
#define INCL_DOSFILEMGR
#include <os2.h>

#ifdef DEBUG
  #include <memcheck.h>
#endif

uint KbdShiftStatus;
static bool ShiftMonitorEnabled = FALSE;
static bool ScrollLockEnabled = FALSE;

static uint  KbdHandle = 0;  /* default to system kbd handle */


void LIBENTRY openkbdhandle(void) {
/*KbdOpen(&KbdHandle); */
/*KbdGetFocus(0,KbdHandle); */
}


void LIBENTRY closekbdhandle(void) {
/*KbdFreeFocus(KbdHandle); */
/*KbdClose(KbdHandle); */
/*KbdHandle = 0; */
}


void LIBENTRY setkbdbinarymode(void) {
  KBDINFO Kbd;

  Kbd.cb = sizeof(Kbd);
  KbdGetStatus(&Kbd,KbdHandle);
  Kbd.fsMask = (unsigned short) KEYBOARD_BINARY_MODE;
  KbdSetStatus(&Kbd,KbdHandle);
}


void LIBENTRY enablekbdmonitor(void) {
  KBDINFO    Kbd;
  KBDKEYINFO Key;

  Kbd.cb = sizeof(Kbd);
  KbdGetStatus(&Kbd,KbdHandle);
  Kbd.fsMask = (unsigned short) KEYBOARD_BINARY_MODE;
  KbdSetStatus(&Kbd,KbdHandle);
  Kbd.fsMask |= (unsigned short) KEYBOARD_SHIFT_REPORT;
  KbdSetStatus(&Kbd,KbdHandle);
  ShiftMonitorEnabled = TRUE;

  KbdPeek(&Key,KbdHandle);
  KbdShiftStatus = Kbd.fsState;
}


void LIBENTRY disablekbdmonitor(void) {
  KBDINFO Kbd;

  Kbd.cb = sizeof(Kbd);
  KbdGetStatus(&Kbd,KbdHandle);
  Kbd.fsMask &= (unsigned short) ~KEYBOARD_SHIFT_REPORT;
  KbdSetStatus(&Kbd,KbdHandle);
  ShiftMonitorEnabled = FALSE;
}


uint LIBENTRY getkbdstatus(void) {
  if (ScrollLockEnabled)
    return((uint) (KbdShiftStatus | KBDSTF_SCROLLLOCK_ON));
  return((uint) (KbdShiftStatus & (~KBDSTF_SCROLLLOCK_ON)));
}


void LIBENTRY setkbdstatus(uint Status) {
/* we'd use the KbdSetStatus() except that it does NOT work in a */
/* WINDOWED session, so we'll just have to fake it and hold our own */
/* shift status (for now, just the scroll lock) */

  ScrollLockEnabled = (bool) (Status & KBDSTF_SCROLLLOCK_ON);
}


uint LIBENTRY bgetkey(ubyte Option) {
  KBDKEYINFO Kbd;

  if (Option == 1) {
    KbdPeek(&Kbd,KbdHandle);
    KbdShiftStatus = Kbd.fsState;
    return((unsigned short) (Kbd.fbStatus & 0x40));
  }

  KbdCharIn(&Kbd,0,KbdHandle);
  KbdShiftStatus = Kbd.fsState;

/* only process final characters, is this a final character? */
  if (Kbd.fbStatus & KBDTRF_FINAL_CHAR_IN) {
/* yes, it's a final character */
/* is it a shift key, and is the ScrollLock key CURRENTLY being pressed? */
    if ((Kbd.fbStatus & KBDTRF_SHIFT_KEY_IN) && (KbdShiftStatus & KBDSTF_SCROLLLOCK)) {
      ScrollLockEnabled = (bool) (! ScrollLockEnabled);
    } else {
      if (Kbd.chChar == 0xE0)
        Kbd.chChar = 0;
      return((unsigned short) (Kbd.chChar + ((unsigned short) Kbd.chScan << 8)));
    }
  }
  return(0);
}

#else  /* ifdef __OS2__ */

#include <dos.h>

extern uint far *KbdStatus;


uint LIBENTRY getkbdstatus(void) {
  return(*KbdStatus);
}


void LIBENTRY setkbdstatus(uint Status) {
  *KbdStatus = Status;
}


uint LIBENTRY bgetkey(ubyte Option) {
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


#ifdef TEST
typedef union {
  int  W;
  struct {
    char LoB,HiB;
  } B;
} keytype;

void main(void) {
  keytype Key;

  openkbdhandle();
  enablekbdmonitor();

  while (1) {
    Key.W = bgetkey(0);
    if (Key.B.LoB == 27)
      break;

    if (Key.B.LoB == 'A')
      setkbdstatus(0);

    printf("Key = %04X, %3d / %3d.  Shift = %d\n",Key.W,Key.B.HiB,Key.B.LoB,getkbdstatus());
  }
}
#endif
