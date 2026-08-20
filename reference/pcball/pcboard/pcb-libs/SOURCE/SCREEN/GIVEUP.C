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

bool Novell = FALSE;
bool DisableGiveup = FALSE;

#ifdef __OS2__
#define INCL_DOSPROCESS
#include <os2.h>
taskertype Tasker  = OS2;

void LIBENTRY giveup(void) {
  DosSleep(1);  /* give up remainder of current time slice */
}

#else  /* ifdef __OS2__ */
#include <model.h>

#ifdef _MSC_VER
#include <borland.h>
#include <stdlib.h>
#else
#include <dos.h>
#pragma inline
#endif

taskertype Tasker = NMT;

long MosVect;
long Int15;
long Int28;


/*
    Checking for Windows/386 and Windows 3.0

    Mov  Ax,1600h
    Int  2Fh

    AL =  0h  means No Windows
    AL = 80h  means No Win/386 mode
    AL =  1h  means Win/386 2.xx running
    AL = FFh  means Win/386 2.xx running

    otherwise AL = Major Version and AH = Minor Version

    Giving up time slices is done by issuing:

    Mov  Ax,1680h
    Int  2Fh

    Starting a critical section is done by issuing:

    Mov  Ax,1681h
    Int  2Fh

    Leaving a critical section is done by issuing:

    Mov  Ax,1682h
    Int  2Fh
*/


void pascal giveup(void) {
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  asm  nop
  return;
}


static void pascal NoMultitasker(void) {
  return;
}


static void pascal DoubleDOS(void) {
  asm  Mov   Ax,0EE02h   /* Give away 2 timer cycles (55ms ea) */
  asm  Int   21h
}


static void pascal PCMos(void) {
  NEEDSEGPUSHDS;
  NEEDSEGGETDS(MosVect);
  asm  Mov   Ah,07h      /* Wait for event */
  asm  Mov   Al,11b      /* wait for 'next keystroke', 'timer ticks' */
  asm  Mov   Bx,02       /* give away 2 timer cycles */
  asm  Pushf
  asm  Cli
  asm  Call  dword ptr MosVect
  NEEDSEGPOPDS;
}

static void pascal Windows(void) {
  static int Count;

  /* if Windows 4.0 (i.w. Win95) then give up a time slice only once every */
  /* 8th call to give up time.  This reduces the friendliness, but also    */
  /* fixes the problem of Win95 taking away too many time slices and not   */
  /* leaving enough time slices left to get real work done                 */

  if (Tasker == WIN4) {
    if (++Count < 8)
      return;
    Count = 0;
  }

  asm  Mov   Ax,1680h    /* give away cycles */
  asm  Int   2Fh
}

static void pascal Os2(void) {

#ifdef NEEDSEG
  asm  Int   28h
#else
  asm  Pushf
  asm  Cli
  asm  Call  dword ptr Int28
#endif

  /* an alternate method which was found to be slightly more efficient */

#ifdef NEWMETHOD

#ifdef _MSC_VER
  asm  Int 28h
#else
  /* DX:AX is the number of milliseconds go give up */
  /* a setting of 0 says to give up remainder of current slice */
  /* a setting of -1 says to sleep forever */
  asm  xor dx,dx
  asm  mov ax,1
/*
  asm  mov ax,0FFFFh
  asm  mov dx,ax
*/
  asm  hlt
  asm  db  035h    /* signature used to differentiate between a normal HLT */
  asm  db  0CAh    /* instruction and the call to DosSleep() */
#endif
#endif
}

static void pascal TopView(void) {
  asm  Mov   Ax,1000h    /* give away timer cycles */
#ifdef NEEDSEG
  asm  Int   15h
#else
  asm  Pushf
  asm  Cli
  asm  Call  dword ptr Int15
#endif
}
#endif /* ifdef __OS2__ */


void LIBENTRY checkmultitaskers(void) {
#ifndef __OS2__
  int DosVer;

  NEEDSEGPUSHDS;

  Tasker = NMT;
  if (DisableGiveup)
    goto Exit;

  /* check first for OS/2 ... because it will be more efficient to directly */
  /* give up time slices to OS/2 than to let an "emulator" such as OS2SPEED */
  /* cause us to use a different method, such as the DESQview method        */

#ifdef _MSC_VER
  DosVer = _osmajor;
#else
  DosVer = _version & 0x00FF;
#endif

  if (DosVer >= 20) {
    Tasker = OS2;

    asm Mov  Ax,3528h
    asm Int  21h

    NEEDSEGGETDS(Int28);
    asm Mov  word ptr [Int28],Bx
    asm Mov  word ptr [Int28+2],Es
    goto Exit;
  }

  /* Novell is a variable that is set if the PCB environment variable      */
  /* contains the /NMT switch.  This lets the sysop ensure that PCBoard    */
  /* does not make use of DoubleDOS calls while Netware is up and running. */
  /*                                                                       */
  /* In case the user forgets to put /NMT in there, however, we will try   */
  /* to detect Netware on our own and, if found, then again we will skip   */
  /* over the check for DoubleDOS.                                         */

  NEEDSEGGETDS(Novell);

asm             Cmp   Novell,0      /* if Novell software is being used    */
asm             Jne   CheckTV       /* then skip the check for DDOS        */

asm             Mov   Ax,0C601h     /* look for Netware                    */
asm             Int   21h
asm             Cmp   Al,01h        /* Assume Netware present if AL = 1    */
asm             Je    CheckTV

asm             Mov   Ah,0E4h       /* Check for DoubleDOS active          */
asm             Int   21h           /*                                     */
asm             And   Al,Al         /*                                     */
asm             Jz    CheckTV       /* Jump if it isn't to Next            */
                                    /*                                     */
                Tasker = DDOS;      /*                                     */
asm             Jmp   Exit          /*                                     */

CheckTV:
asm             Mov   Ax,2B01h      /* Check for DesqView                  */
asm             Mov   Cx,4445h      /*  DE                                 */
asm             Mov   Dx,5351h      /*  SQ                                 */
asm             Int   21h           /*                                     */
asm             Cmp   Al,0FFh       /* if Al <> 0FFh then DESQview is      */
asm             Jne   FoundIt       /* active so go get the video address  */

TV:
asm             Mov   Ax,1022H      /*                                     */
asm             Xor   Bx,Bx         /*                                     */
asm             Int   15H           /*                                     */
asm             And   Bx,Bx         /*                                     */
asm             Jz    CheckMos      /* if Bx=0 then no TopView or TaskView */

FoundIt:
                Tasker = TV;        /* indicate TV is active               */

  NEEDSEGGETDS(Scrn_Addr);

asm             Les   Di,dword ptr Scrn_Addr  /* go find out where the virtual scrn  */
asm             Mov   Ah,0FEh                 /* is located in DESQview              */
asm             Int   10h
asm             Mov   word ptr [Scrn_Addr+2],Es  /* update our screen addresses         */
asm             Mov   word ptr [Scrn_Addr],Di
asm             Mov   Scrn_Rtrc,0   /* we won't need to check retracing    */


  NEEDSEGGETDS(Int15);

asm             Mov   Ax,3515h      /* find out where Int 15h is stored    */
asm             Int   21h
asm             Mov   word ptr [Int15],Bx
asm             Mov   word ptr [Int15+2],Es
asm             Jmp   Exit

CheckMos:
asm             Mov   Ax,3000h
asm             Mov   Bx,Ax
asm             Mov   Cx,Ax
asm             Mov   Dx,Ax
asm             Int   21h
asm             Push  Ax
asm             Mov   Ax,3099h
asm             Int   21h
asm             Pop   Bx
asm             Cmp   Bx,Ax
asm             Je    CheckWin3
                Tasker = MOS;
asm             Mov   Ah,34h
asm             Int   21h
asm             Les   Bx,Es:[Bx-18h]

  NEEDSEGGETDS(MosVect);

asm             Mov   word ptr [MosVect],Bx
asm             Mov   word ptr [MosVect+2],Es
asm             Jmp   short Exit

CheckWin3:
asm             Mov  Ax,1600h
asm             Int  2Fh
asm             Cmp  Al,0           /* AL = 0   means No Windows    */
asm             Je   CheckDOS5
asm             Cmp  Al,80h         /* AL = 80h means no "386 mode" */
asm             Je   CheckDOS5
asm             Cmp  Al,1h          /* AL = 1   means Win/386 v2.xx */
asm             Je   CheckDOS5      /*          don't want it       */
asm             Cmp  Al,0FFh        /* AL = FFh means Win/386 v2.xx */
asm             Je   CheckDOS5      /*          don't want it       */

Win4:
asm             Cmp  Al,4           /* AL = 4 means Win4 aka Win95  */
asm             Jne  WinMethod

                Tasker = WIN4;
                goto Exit;

WinMethod:
                Tasker = WIN;       /* must be Win 3.0 in 386 mode  */
                goto Exit;

CheckDOS5:
  if (DosVer >= 5)
    goto WinMethod;

Exit:;
  NEEDSEGPOPDS;

  switch(Tasker) {
    case DDOS:
                asm Mov Si, offset DoubleDOS
                break;
    case TV  :
                asm Mov Si, offset TopView
                break;
    case MOS :
                asm Mov Si, offset PCMos
                break;
    case WIN :
                asm Mov Si, offset Windows
                break;
    case OS2 :
                asm Mov Si, offset Os2
                break;
    default  :
    case NMT :  asm Mov Si, offset NoMultitasker
                break;
  }

  asm  Push Ds
  asm  Mov  Ax, Cs
  asm  Mov  Es, Ax
  asm  Mov  Ds, Ax
  asm  Mov  Di, offset giveup
  asm  Mov  Cx, 12
  asm  Cld
  asm  Rep  Movsw
  asm  Pop  Ds
#endif /* ifdef __OS2__ */
}




#ifdef COMMENTOUT
void pascal updatevirtualscreen(void) {
asm           Cmp   Tasker,TV      /* Check for TopView active           */
asm           Je    Exit           /* Jump if not                        */
asm           Mov   Ah,0FFh
asm           Les   Di,dword ptr [Scrn_Addr] /* point ES:DI to screen  */
asm           Mov   Cx,2000      /* 2000 is the number of WORDS        */
asm           Int   10h          /* now go update it                   */
Exit:;
}
#endif

#ifdef TEST
#include <stdio.h>

long Scrn_Addr;                   /* Address of Screen Memory (seg:ofs)   */
char Scrn_Rtrc;                   /* True: Perform Screen Retrace         */

int pascal bgetkey(char Option) {
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

void main(void) {
  char *Names[6] = { "NONE", "DDOS", "TV/DV", "PCMOS", "WIN3/DOS5", "OS/2" };

  printf("Testing keyboard polling loop, press any key to continue...");
  while (! bgetkey(1));
  bgetkey(0);

  printf("\n\nChecking for multitasker type\n");
  checkmultitaskers();
  printf("\nFound: %s\n",Names[Tasker/2]);

  printf("\nPress any key to exit...");
  while (! bgetkey(1))
    giveup();
  bgetkey(0);
  printf("\n");
}
#endif
