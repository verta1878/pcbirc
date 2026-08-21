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


#include <dos.h>
#ifndef __WATCOMC__
#include <dir.h>
#endif
#include <stdio.h>
#include <string.h>
#include "misc.h"
#include "validate.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#if !defined(__OS2__) && !defined(__WATCOMC__)  /* need to finish  --   CHANGE TO dosfindfirst() */

static void _NEAR_ LIBENTRY addslashtoname(char *Str) {
  int L;

  L = strlen(Str);
  if (L > 0 && Str[L-1] != '\\') {
    Str[L]   = '\\';
    Str[L+1] = 0;
  }
}

int LIBENTRY copyfilesandpaths(char *SrcePath, char *DestPath, void (*display)(char *FileName)) {
  char FileMask[66];
  char Dest[66];
  char Dummy[66];
  char *p;
  char *q;
  int  RetVal;
  int  CopyVal;
  struct ffblk F;

  strcpy(FileMask,SrcePath);
  addslashtoname(FileMask);
  p = &FileMask[strlen(FileMask)];
  strcpy(p,"*.*");

  strcpy(Dest,DestPath);
  addslashtoname(Dest);
  q = &Dest[strlen(Dest)];

  RetVal = findfirst(FileMask,&F,FA_DIREC|FA_HIDDEN|FA_SYSTEM|FA_RDONLY);
  while (RetVal != -1) {
    if (F.ff_name[0] != '.') {
      strcpy(p,F.ff_name);
      strcpy(q,F.ff_name);
      if (F.ff_attrib != FA_DIREC) {
        switch (validatepath(NULL,Dest,Dummy,VALIDATE_FILE)) {
          case ERRNOTDIR      :
          case ERRNOTFILE     :
          case ERRDIRNOTFOUND :
          case ERRDISKINVALID : break;
          case ERRNOTFOUND    :
          default             : display(F.ff_name);
                                if ((CopyVal = copyfile(FileMask,Dest,FALSE)) != 0)
                                  return(CopyVal);
                                break;
        }
      } else {
        if ((CopyVal = copyfilesandpaths(FileMask,Dest,display)) != 0)
          return(CopyVal);
      }
    }
    RetVal = findnext(&F);
  }
  return(0);
}

#endif
