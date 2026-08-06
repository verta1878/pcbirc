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


#if defined(__OS2__) || defined(__WATCOMC__)
  #define INCL_DOSPROCESS
  #include <os2.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
  #else
    #pragma inline
  #endif
#endif

#include "screen.h"

#if !defined(__OS2__) && !defined(__WATCOMC__)
unsigned static delaycnt;
#endif

/*****************************************************************************
    Sets up the timer constant for asmdelay
*****************************************************************************/

void LIBENTRY setdelay(void) {
#if !defined(__OS2__) && !defined(__WATCOMC__)
asm           Push  Ds
asm           Xor   Ax,Ax
asm           Mov   Ds,Ax
asm           Mov   Di,046Ch

asm           Xor   Bx,Bx         /* Initialize counter to 0 */

    StpRetry:
asm           Sti                 /* RAMBOOST leaves interrupts disabled! turn them back on now */
asm           Mov   Ax,[Di]

    StpLoop1:
asm           Cmp   Ax,[Di]       /* loop until the next clock tick */
asm           Je    StpLoop1

  /* set a goal (target number of ticks) which is current plus 4 */

#ifdef CPU386
asm           Mov   EAx,[Di]      /* without using CLI, get ticks */
asm           Add   EAx,4         /* EAX = clock ticks + 4 */
#else
asm           Les   Ax,[Di]       /* without using CLI, get ticks */
asm           Mov   Dx,Es         /* DX:AX now equals current timer */
asm           Add   Ax,4          /* DX:AX = DX:AX + 4 ticks */
asm           Adc   Dx,0
#endif

  /* 1800B0h is midnight                     */
  /* This is a kludge to check and see if    */
  /* the clock tick is up above 180000h and  */
  /* if so we will continually retry until   */
  /* we get past midnight                    */

#ifdef CPU386
asm           Cmp   EAx,180000h
#else
asm           Cmp   Dx,18h
#endif
asm           Jge   StpRetry


    StpLoop2:
asm           Inc   Bx            /* BX will count how many times we do this */
asm           Mov   Cx,200        /*                                         */
    StpLoop3:                     /*                                         */
asm           Loop  StpLoop3      /* Count to 200 real fast                  */

  /* check to see if 4 clock ticks have passed yet */

#ifdef CPU386
asm           Cmp   EAx,[Di]      /* check without using CLI, if goal    */
asm           Jg    StpLoop2      /* is greater than current ticks, loop */
#else
asm           Cli
asm           Mov   Si,[Di+2]
asm           Mov   Cx,[Di]
asm           Sti
asm           Cmp   Dx,Si         /* if top half of DX:AX greater than SI:CX */
asm           Jg    StpLoop2      /* then loop back up */
asm           Cmp   Ax,Cx         /* if bottom of DX:AX greater than SI:CX */
asm           Jg    StpLoop2      /* then loop back up */
#endif

  /* We waited for 4 clock ticks.  That is approximately 4/18.2's of a      */
  /* second, or 0.2197 seconds.  During that time, we counted the number    */
  /* of times that the outer loop ran and stored that value in BX.          */
  /*                                                                        */
  /* Now to find out how many outer loops are needed, for 1/100th of a      */
  /* second, we divide the count (BX) by 21.97.  We can do that with more   */
  /* precision, using integer math, by multiplying by 455 and then dividing */
  /* by 10000.                                                              */


asm           Mov   Ax,455
asm           Mul   Bx
asm           Mov   Bx,10000
asm           Div   Bx

asm           Pop   Ds
asm           Mov   [delaycnt],Ax    /* store outer loop count in delaycnt */
#endif
;
}


/*****************************************************************************
    Takes the "count" passed to it on the stack and waits for that many
    hundredths of a second.  In other words, if passed the value 300 then
    it will wait 3.0 seconds.
*****************************************************************************/

/* NOTE: this function may be called from an assembly language */
/* routine - so we MUST save all of the used registers!        */

void LIBENTRY mydelay(int Hundredths) {
#if defined(__OS2__) || defined(__WATCOMC__)
  DosSleep((long) Hundredths * 10);
#else
#ifdef CPU386
asm           Push  EAx
asm           Push  ECx
#else
asm           Push  Ax
asm           Push  Cx
#endif
asm           Push  Dx

asm           Mov   Ax,Hundredths
asm           Or    Ax,Ax
asm           Jz    Exit

#ifdef CPU386
asm           Cwde                     /* convert AX to EAX */
asm           Movzx ECx,[delaycnt]     /* load ECX with delay count */
asm           Mul   ECx                /* multiply EAX by ECX */
#else
asm           Mov   Cx,[delaycnt]
asm           Xor   Dx,Dx              /* Zero extend DX:AX value */
asm           Mul   Cx                 /* Divide DX:AX by CX */
#endif


    DlyLoop1:
asm           Mov   Cx,200
    DlyLoop2:
asm           Loop  DlyLoop2

#ifdef CPU386
asm           Dec   EAx
asm           Jnz   DlyLoop1
#else
asm           Sub   Ax,1
asm           Sbb   Dx,0
asm           Jnz   DlyLoop1
asm           Or    Ax,Ax
asm           Jnz   DlyLoop1
#endif

    Exit:;
asm           Pop   Dx
#ifdef CPU386
asm           Pop   ECx
asm           Pop   EAx
#else
asm           Pop   Cx
asm           Pop   Ax
#endif
#endif
;
}

#ifdef TEST
#include <stdio.h>
void main(void) {
  setdelay();
  printf("Delay Value = %u\n",delaycnt);
  printf("Delaying 3 seconds...");
  mydelay(300);
  printf("done.\n");
}
#endif
