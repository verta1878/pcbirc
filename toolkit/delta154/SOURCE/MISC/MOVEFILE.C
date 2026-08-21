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
  #include <io.h>
  #define _chmod chmod
#else
  #include <dir.h>
#endif

#include <io.h>
#include <dos.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>
#include "screen.h"
#include "scrnio.h"
#include "scrnio.ext"
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


int LIBENTRY movefile(char *Srce, char *Dest) {
  int  Code;

  /* see if filenames and paths are identical, if so, consider it moved */
  if (strcmp(Srce,Dest) == 0)
    return(0);

  if (Srce[0] == Dest[0] && ((Srce[1] == ':' && Dest[1] == ':') || (Srce[0] == '\\'))) {
    /* try to move the file by renaming it */
    Code = rename(Srce,Dest);

    if (Code) {
      /* the rename failed, so try copying it instead */
      Code = copyfile(Srce,Dest,FALSE);
      if (! Code) {
        /* successful copy, delete the source file */
        unlink(Srce);
      }
    } else {
      #ifndef __WATCOMC__
        /* set attributes for the destination file equal to the source           */
        /* PLUS!  Add the archive bit for EARL BONSER cuz he gets upset          */
        /* about having to do full backups instead of partial backups (10/24/94) */
        _chmod(Dest,1,_chmod(Srce,0)|_A_ARCH);
      #endif
    }
  } else {
    /* different drives, can't use rename, so copy it instead */
    Code = copyfile(Srce,Dest,FALSE);
    if (! Code) {
      /* successful copy, delete the source file */
      unlink(Srce);
    }
  }

  return(Code);
}
