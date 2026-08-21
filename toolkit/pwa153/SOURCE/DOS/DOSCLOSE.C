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
#pragma inline
#include <model.h>
#endif

#include "dosfunc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


static void (LIBENTRY *dosclosecallback)(char *FileName,int Handle,int ExtendedError);

/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Returns :
*/

void LIBENTRY dosclose(int handle) {
  #ifdef __OS2__
    APIRET ExtendedError;
  #endif

  if (handle > 0 && handle <= MAXHANDLES && OpenFileNames[handle][0] != 0) {
    #ifdef __OS2__
      ExtendedError = DosClose(handle);
    #else
      ExtendedError = 0;

      asm mov ah,3Eh
      asm mov bx,handle

      int21();

      asm jnc end
      getextendederror();
      end:;
   #endif

    if (dosclosecallback != NULL)
      dosclosecallback(OpenFileNames[handle],handle,ExtendedError);

    OpenFileNames[handle][0] = 0;
  }
}


void LIBENTRY closedosopenfiles(void) {
  int X;

  for (X = MAXHANDLES; X > 0; X--)
    dosclose(X);
}


void LIBENTRY setdosclosecallback(void (LIBENTRY *cb)(char *FileName,int Handle,int ExtendedError)) {
  dosclosecallback = cb;
}
