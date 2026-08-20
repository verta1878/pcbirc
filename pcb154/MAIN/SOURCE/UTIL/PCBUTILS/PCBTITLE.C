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


#include <string.h>
#include <stdlib.h>

#pragma inline

char Title[] = "NODE - XXXX";

void main(void) {
  char *p;

  if (getenv("PCBOS2") != NULL) {
    if ((p = getenv("PCBNODE")) != NULL) {
      strcpy(Title+7,p);
      asm mov  ax,0x6400
      asm xor  bx,bx
      asm mov  cx,0x636c
      asm mov  dx,1
      asm mov  di,offset Title
      asm int  0x21
    }
  }
}
