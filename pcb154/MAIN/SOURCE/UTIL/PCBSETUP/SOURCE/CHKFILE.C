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
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


int backslash(int Before) {
  if (Before)
    return(0);

  stripright(SaveFldPtr->Answer,' ');
  addbackslash(SaveFldPtr->Answer,SaveFldPtr->Flen);
  return(0);
}


void pascal findfilename(int *Len, int *ExtLen) {
  char Name[40];
  char *p;
  char *q;

  maxstrcpy(Name,SaveFldPtr->Answer,sizeof(Name));
  stripright(Name,' ');

  p = strrchr(Name,'\\');        /* search for last \ in path name */
  if (p == NULL) {
    p = strchr(Name,':');        /* if \ was not found then search for : */
    if (p == NULL) {
      p = Name;                  /* if : was not found then take entire name */
    } else {
      p++;                       /* otherwise point to character AFTER the : */
    }
  } else {
    p++;                         /* if \ was found point to char AFTER the \ */
  }

  *Len = strlen(p);              /* find length of name    */

  q = strchr(p,'.');             /* search for . in name   */
  if (q == NULL) {
    *ExtLen = -1;                /* if not found, return ExtLen = -1 */
  } else {
    q++;                         /* if found, jump over the . and return     */
    *ExtLen = strlen(q);         /* ExtLen = to length of filename extension */
    *Len -= (*ExtLen + 1);       /* subtract length of extension from Len    */
  }
}


int checkfiles6(int Before) {
  int  ExtLen;
  int  Len;

  if (Before)
    return(0);

  findfilename(&Len,&ExtLen);

  if (ExtLen != -1) {                  /* files cannot have an extension */
    beep();
    showhelp(ERREXTENSION);
    return(-1);
  }

  if (Len > 6) {                       /* files cannot be more than 6 chars */
    beep();
    showhelp(ERRLENGTH6);
    return(-1);
  }

  return(0);
}


int checkfiles7(int Before) {
  int  ExtLen;
  int  Len;

  if (Before)
    return(0);

  findfilename(&Len,&ExtLen);

  if (ExtLen != -1) {                  /* files cannot have an extension */
    beep();
    showhelp(ERREXTENSION);
    return(-1);
  }

  if (Len > 7 ) {                      /* files cannot be more than 7 chars */
    beep();
    showhelp(ERRLENGTH7);
    return(-1);
  }

  return(0);
}


int checkfiles8(int Before) {
  int  ExtLen;
  int  Len;

  if (Before)
    return(0);

  findfilename(&Len,&ExtLen);

  if (ExtLen > 3) {                    /* ext cannot be more than 3 chars */
    beep();
    showhelp(ERREXTLEN);
    return(-1);
  }

  if (Len > 8) {                       /* file cannot be more than 8 chars */
    beep();
    showhelp(ERRLENGTH8);
    return(-1);
  }

  return(0);
}


int checkfiles8n(int Before) {
  int  ExtLen;
  int  Len;

  if (Before)
    return(0);

  findfilename(&Len,&ExtLen);

  if (ExtLen != -1) {                  /* file cannot have an extension */
    beep();
    showhelp(ERREXTENSION);
    return(-1);
  }

  if (Len > 8) {                       /* file cannot be more than 8 chars */
    beep();
    showhelp(ERRLENGTH8);
    return(-1);
  }

  return(0);
}
