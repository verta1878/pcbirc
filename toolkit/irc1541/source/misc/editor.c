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


/* this module has not been converted to OS/2 */

#ifndef __OS2__

#include <mem.h>
#include <alloc.h>
#include "screen.h"
#include "scrnio.h"
#include "scrnio.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


/********************************************************************
*
*  Function: editinsline()
*
*  Params  : void **Array - is a pointer to a pointer to a structure
*            int  Num     - is the current offset from *Array (current line #)
*            int  *Total  - points to an integer which is the total # of lines
*            int  RecSize - is the size of each record in the structure
*            void pascal (*blankrec)(void *p) - a function to empty the record
*                           pointed to by *p
*
*  Desc    : Adds a blank line to the table at the current location plus one,
*            adjusting the array size at the same time.
*/

int pascal editinsline(void **Array, int Num, int *Total, int RecSize, void pascal (*blankrec)(void *p)) {
  char *p;

  if ((p = (char *) realloc(*Array,((*Total)+1) * RecSize)) != NULL)
    *Array = p;
  else {
    beep();
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox = TRUE;
    MsgData.Save    = TRUE;
    MsgData.Line1   = 18;
    MsgData.Color1  = Colors[HEADING];
    if ((*Array = realloc(*Array,*Total * RecSize)) == NULL) {
      MsgData.Msg1 = "Insufficient memory, memory pointers lost ...";
      showmessage();
      return(-1);
    }
    MsgData.Msg1 = "Insufficient memory to insert a line";
    showmessage();
    return(0);
  }

  p = (char *) *Array;
  p += (Num * RecSize) + RecSize;
  memmove(p + RecSize,p,(*Total-Num-1) * RecSize);
  blankrec(p);
  (*Total)++;
  return(0);
}


/********************************************************************
*
*  Function: editdelline()
*
*  Params  : void **Array - is a pointer to a pointer to a structure
*            int  Num     - is the current offset from *Array (current line #)
*            int  *Total  - points to an integer which is the total # of lines
*            int  RecSize - is the size of each record in the structure
*
*  Desc    : Deletes the current line from the table and adjusts the size of
*            the array when done.
*/

void pascal editdelline(void **Array, int Num, int *Total, int RecSize) {
  char *p;

  if (*Total == 1) {
    beep();
    return;
  }

  p = (char *) *Array;
  p += Num * RecSize;
  memmove(p,p + RecSize,(*Total-Num-1) * RecSize);
  (*Total)--;
  *Array = realloc(*Array,*Total * RecSize);
}


#endif  /* ifndef __OS__ */
