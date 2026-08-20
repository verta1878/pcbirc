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


#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <malloc.h>
#else
  #include <alloc.h>
#endif

#include <mem.h>
#include <stdio.h>
#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

void * LIBENTRY mallochk(unsigned size) {
  void *p;

  if ((p = malloc(size)) != NULL) {
    memset(p,0,size);
    return(p);
  }

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.Save    = TRUE;
  MsgData.AutoBox = TRUE;
  MsgData.Line1   = 18;
  MsgData.Msg1    = "Insufficient memory to perform requested operation";
  MsgData.Color1  = 0x0f;
  showmessage();
  return(NULL);
}
