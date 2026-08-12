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
#include "zmodem.h"

#ifdef MEMCHECK
#include <memcheck.h>
#endif


/***************************************************************************
 ***************************************************************************/
int pascal zmGetInSync(int Flag) {
  register    c;

  while (TRUE) {
    switch (c = zmGetHeader(rxHeader)) {
      case ZCAN    : zmAbort = TRUE;
                     //lint !e616 fall thru
      case ZABORT  :
      case ZFIN    :
      case TIMEOUT : zmError(1,"Got %s sending data", FrameTypes[c + ZFTOFFSET]);
                     return ERROR;
      case ZRPOS   : if ((dosfseek(&in, rxPosition, SEEK_SET)) == -1)
                       return ERROR;
                     EndOfFile = FALSE;
                     ByteCount = lrxPosition = txPosition = rxPosition;
                     zmError(1,"ZRPOS");
/*** LDZ 03/08/93: removed the following so block sizes won't kick down ****/
/****
                     if (LastSync == rxPosition) {
                       zmDisplay(60, 24, 0x0F, "Been here : %d", BeenHere);
                       if (++BeenHere > 12) {
                         zmError(1,"Can't send block");
                         return ERROR;
                       }
                       if (BeenHere > 4) {
                         if (BlockLength > 32) {
                           BlockLength /= 2;
                           zmError(0,"Resizing Block Size");
                           zmDisplay(15, 5, ZM_DISPLAY, "%-4d", BlockLength);
                         }
                       }
                     }
****/
                     LastSync = rxPosition;
                     return c;
      case ZACK    : lrxPosition = rxPosition;
                     if (Flag || txPosition == rxPosition)
                       return ZACK;
                     continue;
      case ZRINIT  : return c;
      case ZSKIP   : zmError(0,"File skipped by receiver request...");
                     return c;
      case ERROR   : //lint !e616  fall thru  (should it???)
      default      : zmSendBinaryHeader(4, ZNAK, txHeader);
                     continue;
    }
  }

/*** unreachable code ******************************************************
    return ERROR;
 ***************************************************************************/
}
