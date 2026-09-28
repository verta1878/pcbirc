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
/*
 * Putsec writes the Bytes characters of Buf to receive file fout.
 *  If not in binary mode, carriage returns, and all characters
 *  starting with CPMEOF are discarded.
 */
int pascal zmPutSector(char *Buf, int Bytes) {

  if (Bytes == 0)
    return(0);

  return(dosfwrite(Buf, Bytes, &in));

//if (BinaryFile) {
//  dosfwrite(Buf, Bytes, &out);    //lint !e534
//} else {
//  if (EndOfFile)
//    return;
//  for (p = Buf; --Bytes >= 0; ++p) {
//    if (*p == '\r')
//      continue;
//    if (*p == CPMEOF) {
//      EndOfFile = TRUE;
//      return;
//    }
//    dosfwrite(p, 1, &out);   //lint !e534
//  }
//}
}
