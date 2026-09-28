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


#ifndef USE_OLD_CODE

#include <stdlib.h>
#include <string.h>
#include <types.hpp>
void LIBENTRY fullyqualifiedname(char *Q, char *File, int QLen) {
  _fullpath(Q,File,QLen);
  strupr(Q);
}


#else // below is the old code

#include <string.h>
#include <pcb.h>
#include <dosfunc.h>
#include <misc.h>


void LIBENTRY fullyqualifiedname(char *Q, char *File, int QLen) {
  char  Drive;
  char *p;
  char *Slash;
  char *PrevSlash;
  char *PrevSlash2;

  /* get the drive letter by seeing if one is already specified, if it is, */
  /* then use the one specified, otherwise find out the current drive.     */

  if (File[1] == ':') {
    Drive = File[0];
    File += 2;
  } else
    Drive = (char) (dosgetcurdrive() + 'A' - 1);

  Q[0]  = Drive;
  Q[1]  = ':';

  /* if a file starts out as ".\FILENAME" then the ".\" can be removed, */
  /* because the ".\" just says it's in the current directory.          */

  if (File[0] == '.' && File[1] == '\\')
    File++;

  /* now remove any other "\.\" sections of the path, because "\.\" will */
  /* evaluate to no change in the path as it's redundant.                */

  substitute(File,"\\.\\","",strlen(File));

  /* now check to see if the path starts with a backslash indicating it is a */
  /* fully qualified path, if so, and if there aren't any parent paths (..)  */
  /* then we can just copy the path and get out of here, otherwise we'll     */
  /* need to figure out the path ourselves <UGH!>                            */

  if (File[0] == '\\' && strstr(File,"..") == NULL) {
    maxstrcpy(Q+2,File,QLen-2);
    strupr(Q);
    return;
  }

  dosgetcurpath(Drive - 'A' + 1,Q,QLen-1);
  addbackslash(Q,QLen);
//addchar(Q,'\\');
  QLen -= strlen(Q);
  maxstrcpy(Q+strlen(Q),File,QLen);
  strupr(Q);

  /* at this point we have a fully qualified path, but if there are any */
  /* parent paths (..) then we need to collapse them down to make them  */
  /* non-relative/absolute paths from the root.                         */

  Slash = PrevSlash = PrevSlash2 = NULL;

  for (p = Q + 1; *p != 0; p++) {
    if (*p == '\\') {
      PrevSlash2 = PrevSlash;
      PrevSlash  = Slash;
      Slash      = p;
      continue;
    }

    if (*p == '.' && *(p+1) == '.' && *(p+2) == '\\' && PrevSlash != NULL) {
      /* since the ".." in "\path\..\" gets rid of "path", just take both of */
      /* them out at once by copying from the next slash down to the first   */
      memmove(PrevSlash,p+2,strlen(p+2)+1);
      PrevSlash = PrevSlash2;
      Slash     = PrevSlash;
      p         = PrevSlash;
    }
  }

  strupr(Q);
}

#endif
