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


#include <pcbtools.h>
#include <stdio.h>
#include <string.h>
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif

/***************************************************************************
 ***************************************************************************/
void pascal zmGraph(unsigned long bCount) {
    char  Scale[51];
    int   Len;
    int   Color;

    Color = (Scrn_Mode ? ZM_GRAPH : 0x07);

    if (bCount < LastByteCount || bCount <= 1024) {
      memset(Scale,' ',50);
      Scale[50] = 0;
      fastprint(ZM_GRAPHX,ZM_GRAPHY,Scale,Color);
    }

    if (bCount > FileSize)
      bCount = FileSize;

    LastByteCount = bCount;
    Len = ((int) ((bCount * 100) / FileSize)) / 2;
    memset(Scale,'ß',Len);
    Scale[Len] = 0;

    fastprint(ZM_GRAPHX,ZM_GRAPHY,Scale,Color);
}
