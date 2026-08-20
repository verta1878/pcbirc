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


#ifdef __BORLANDC__
#include <dir.h>
#else
#include <direct.h>
#endif

#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dosfunc.h"
#include "misc.h"
#include "newdata.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/********************************************************************
*
*  Function: srchpath()
*
*  Desc    : Finds the environment PATH= setting and scans each of the paths
*            listed to see if a file resides in that directory.
*
*            It starts out by checking the DEFAULT directory and then searches
*            each of the paths specified by the environment PATH variable.
*
*  Returns : -1 if the file is not found
*             0 if found (with FileName modified to hold the found path\file)
*/


int LIBENTRY srchpath(char *FileName) {
  char EnvPath[512];
  char Path[512];
  char *p;

  if (fileexist(FileName) != 255)
    return(0);

  maxstrcpy(EnvPath,getenv("PATH"),sizeof(EnvPath));
  if (EnvPath[0] != 0) {
    if ((p = parsepaths(EnvPath)) != NULL) {
      do {
        strcpy(Path,p);
        addbackslash(Path,sizeof(Path));
        strcat(Path,FileName);
        if (fileexist(Path) != 255) {
          strcpy(FileName,Path);
          return(0);
        }
      } while ((p = parsepaths(NULL)) != NULL);
    }
  }
  return(-1);
}
