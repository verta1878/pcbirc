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


#ifdef __OS2__
#define INCL_DOSFILEMGR
#include <os2.h>
#else
/*#pragma inline */
#include "model.h"
#endif

#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


int LIBENTRY dosunlinkcheck(char *path) {
  unsigned Retry;
  #ifdef __OS2__
  APIRET     rc;
  os2errtype Os2Error;
  #endif

  Retry = 0;

j1:
  #ifdef __OS2__
    rc = DosDelete(path);
    if (rc != 0)
      goto error;
    return(0);
  #else
  #ifdef LDATA
    asm push ds
    asm lds  dx, path
  #else
    asm mov  dx, path
  #endif
    asm mov  ah,41h

    int21();

  #ifdef LDATA
    asm pop  ds
  #endif
    asm jc  error
    return(0);
  #endif

error:
  #ifdef __OS2__
    Os2Error.ExtendedError = rc;
    getextendederror(&Os2Error);
  #else
    getextendederror();
    if (ExtendedError == 83 && Int24Error == 12) {
      ExtendedError = 5;        /* did INT24 call it a 'General Failure?' */
      ExtendedAction = 2;       /* change it to 'Access Denied' because   */
    }                           /* the file is simply IN USE right now    */
  #endif

  if ((Retry = retrycount(Retry,path,"Deleting" POS2ERROR)) != 0xFFFF)
    goto j1;
  return(-1);
}
