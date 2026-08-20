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
  #include <direct.h>
#else
  #include <dir.h>
#endif

#include <io.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include "dosfunc.h"
#include "misc.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif

#define MAXFILES 10

void LIBENTRY deletefiles(char *FileSpec) {
  struct ffblk f[MAXFILES];
  struct ffblk *pf;
  int    Found;
  int    Count;
  char  *p;
  char   Temp[66];
  #ifdef __OS2__
  int    DirHandle = MAKEDIRHANDLE;
  #endif

  Found = MAXFILES;
  if (dosfindfirst(FileSpec,f,_A_ARCH|_A_NORMAL,&Found PDIRHANDLE) == 0) {
    strcpy(Temp,FileSpec);
    if ((p = strrchr(Temp,'\\')) != NULL || (p = strrchr(Temp,':')) != NULL)
      p++;
    else
      p = Temp;

    while (1) {
      for (Count = Found, pf = f; Count > 0; Count--, pf++) {
        strcpy(p,pf->ff_name);
        unlink(Temp);
      }

      /* if the number found is less than the max, then there are no more */
      /* files to go looking for */
      if (Found < MAXFILES)
        break;

      #ifndef __OS2__
        /* the dos version needs the info of the last file found in order */
        /* for it to find the next file */
        f[0] = f[Found-1];
      #endif

      if (dosfindnext(f,&Found PDIRHANDLE2) == -1)
        break;
    }

    #ifdef __OS2__
      dosclosedirhandle(DirHandle);
    #endif
  }
}
