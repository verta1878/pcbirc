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


#include <io.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>
/* #include <screen.h> */
/* #include <scrnio.h> */
/* #include <scrnio.ext> */
#include <dosfunc.h>
#include "misc.h"

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <malloc.h>
#else
  #include <alloc.h>
#endif

#ifdef DEBUG
  #include <memcheck.h>
#endif

int LIBENTRY copyfile(char *Srce, char *Dest, bool CheckForEOF) {
  int      S;
  int      D;
  int      Error;
  unsigned Memory;
  unsigned BytesRead;
  char     *Buffer;
  char     *Eof;
  #if ! (defined(_MSC_VER) || defined(__WATCOMC__))
    struct ftime TimeDate;
  #endif


  Error = 0;
  if (strcmp(Srce,Dest) == 0) /* source = destination!  ignore copy request */
    goto end1;

  if ((S = dosopencheck(Srce,OPEN_READ|OPEN_DENYWRIT)) == -1)
    return(COPYFILE_ERROPENSRCE);

  #if ! (defined(_MSC_VER) || defined(__WATCOMC__))
    getftime(S,&TimeDate);
  #endif

  unlink(Dest);
  if ((D = doscreatecheck(Dest,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL)) == -1) {
    dosclose(S);
    return(COPYFILE_ERROPENDEST);
  }

  #ifdef __OS2__
    Memory = 32768;
  #else
    if (coreleft() > 0x8000)        /* Don't let it try to use more than a   */
      Memory = 0x8000;              /* 32K buffer (8000 hex bytes)           */
    else
      Memory = (int) coreleft();

    if (Memory > 2048) {            /* if memory is greater than 2048 bytes  */
      Memory /= 2048;               /* then round it to a multiple of 2048   */
      Memory *= 2048;               /* for more efficient copying            */
    }
  #endif

  while (1) {
    if ((Buffer = (char *) malloc(Memory)) != NULL) /* if memory alloc successful */
      break;                                        /* then proceed with copy     */

    if (Memory > 256)
      Memory >>= 1;
    else {
      Error = COPYFILE_ERRMEMALLOC;
      goto end2;
    }
  }

  BytesRead = Memory;
  while (BytesRead == Memory) {
    if ((BytesRead = readcheck(S,Buffer,Memory)) == 0xffff) {
      Error = COPYFILE_ERRREADSRCE;
      goto end3;
    }
    if (CheckForEOF)
      if ((Eof = strnchr(Buffer,26,BytesRead)) != NULL)
        BytesRead = (int) (Eof - Buffer);
    if (writecheck(D,Buffer,BytesRead) == (unsigned) -1) {
      Error = COPYFILE_ERRWRITDEST;
      goto end3;
    }
  }


end3:
  free(Buffer);

end2:
  #if ! (defined(_MSC_VER) || defined(__WATCOMC__))
    setftime(D,&TimeDate);
  #endif

  dosclose(D);
  dosclose(S);

  /* set attributes for the destination file equal to the source           */
  /* PLUS!  Add the archive bit for EARL BONSER cuz he gets upset          */
  /* about having to do full backups instead of partial backups (10/24/94) */
  #if ! (defined(_MSC_VER) || defined(__WATCOMC__))
    _chmod(Dest,1,_chmod(Srce,0)|FA_ARCH);
  #endif

end1:
  if (Error == COPYFILE_ERRREADSRCE || Error == COPYFILE_ERRWRITDEST)
    unlink(Dest);
  return(Error);
}

