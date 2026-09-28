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


#include <stdlib.h>
#include "screen.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define MAXSAVE    25

static int NumSaved = 0;
static savescrntype *Screens[MAXSAVE];


static int _NEAR_ LIBENTRY findfreescreen(void) {
  int X;

  for (X = 0; X < MAXSAVE; X++) {
    if (Screens[X] == NULL)
      return(X);
  }
  return(-1);
}


int LIBENTRY memsavescreen(void) {
  int NumBytesInVidBuf;
  int ScreenNum;

  if (NumSaved >= MAXSAVE || (ScreenNum = findfreescreen()) == -1)
    return(-1);

  NumBytesInVidBuf = (Scrn_BottomRow+1) * 160;

  if ((Screens[ScreenNum] = (savescrntype *) malloc(NumBytesInVidBuf)) == NULL)
    return(-1);

  savescreen(Screens[ScreenNum]);
  NumSaved++;
  return(ScreenNum);
}


void LIBENTRY memfreescreen(int ScreenNum) {
  if (NumSaved <= 0 || ScreenNum < 0 || ScreenNum >= MAXSAVE)
    return;

  if (Screens[ScreenNum] == NULL)
    return;

  free(Screens[ScreenNum]);
  Screens[ScreenNum] = NULL;
  NumSaved--;
}


void * LIBENTRY memscreenptr(int ScreenNum) {
  if (NumSaved <= 0 || ScreenNum < 0 || ScreenNum >= MAXSAVE)
    return(NULL);
  return(Screens[ScreenNum]);
}


void LIBENTRY memrestorescreen(int ScreenNum, savefreetype Status) {
  if (NumSaved <= 0 || ScreenNum < 0 || ScreenNum >= MAXSAVE)
    return;

  if (Screens[ScreenNum] == NULL)
    return;

  restorescreen(Screens[ScreenNum]);

  if (Status == FREESCREEN)
    memfreescreen(ScreenNum);
}


void LIBENTRY memfreeallscreens(void) {
  int X;

  for (X = 0; X < MAXSAVE; X++) {
    if (Screens[X] != NULL) {
      free(Screens[X]);
      Screens[X] = NULL;
    }
  }
  NumSaved = 0;
}
