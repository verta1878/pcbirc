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


#include <stdio.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

static char Scale[51];
static int  ScaleYless1;
static int  ScaleY;
static int  ScaleLen;


/********************************************************************
 *
 *  Function:  setupscale()
 *
 *  Places a scale on the right side of the screen beginning on row Y and
 *  continuing for a length of Len characters of Ch.
 */

void LIBENTRY setupscale(int Len, char Ch, int Y) {
  memset(Scale,Ch,sizeof(Scale));
  Scale[0]     = 24;          /* top arrow    */
  Scale[Len+2] = 25;          /* bottom arrow */
  Scale[Len+3] = 0;
  ScaleYless1  = Y-1;
  ScaleY       = Y;
  ScaleLen     = Len;
}


/********************************************************************
 *
 *  Function:  scale()
 *
 *  Prints a new scale on the screen according to the Num passed to it
 *  and it's percentage of Total.
 */

void LIBENTRY scale(long Num, long Total) {
  int Loc;

  if (UpdateBox) {
    Loc = ScaleY;

    if (Total != 0)
      Loc += (unsigned) ((Num * ScaleLen) / Total);

    fastprintv(79,ScaleYless1,Scale,Colors[SCALECOLOR1]);
    fastprint(79,Loc,"\xB1",Colors[SCALECOLOR2]);
  }
}
