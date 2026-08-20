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
  #include <malloc.h>
#else
  #include <alloc.h>
#endif

#include <io.h>
#include <stdio.h>
#include <string.h>
#include "dosfunc.h"
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

int LIBENTRY appendfile(char *Srce, char *Dest, bool CheckForEOF) {
  int      S;
  int      D;
  int      Error;
  unsigned Memory;
  unsigned BytesRead;
  char     *Buffer;
  char     *Eof;

  Error = 0;
  if (strcmp(Srce,Dest) == 0) /* source = destination!  ignore copy request */
    goto end1;

  if ((S = dosopencheck(Srce,OPEN_READ|OPEN_DENYNONE)) == -1)
    return(-1);

  if (CheckForEOF)
    D = dosappendcheck(Dest,OPEN_RDWR|OPEN_DENYRDWR);
  else {
    D = dosopencheck(Dest,OPEN_WRIT|OPEN_DENYRDWR);
    doslseek(D,0,SEEK_END);
  }

  if (D == -1) {
    dosclose(S);
    return(-1);
  }


  #ifdef __OS2__
    Memory = 65536;
  #else
    if (coreleft() > 0xFFFF)        /* if we're in COMPACT model then adjust */
      Memory = 0xFFFF;              /* size to fit in 64k buffer             */
    else
      Memory = (int) coreleft();

    Memory &= 0xFC00;               /* round memory size to multiple of 2048 */
  #endif

  if ((Buffer = (char *) malloc(Memory)) == NULL) {
    Error = COPYFILE_ERRMEMALLOC;
    goto end2;
  }

  BytesRead = Memory;
  while (BytesRead == Memory) {
    if ((BytesRead = readcheck(S,Buffer,Memory)) == 0xffff) {
      Error = COPYFILE_ERRREADSRCE;
      goto end3;
    }
    if (CheckForEOF)
      if ((Eof = strnchr(Buffer,26,BytesRead)) != NULL)
        BytesRead = (unsigned) (Eof - Buffer);
    if (writecheck(D,Buffer,BytesRead) == (unsigned) -1) {
      Error = COPYFILE_ERRWRITDEST;
      goto end3;
    }
  }


end3:
  free(Buffer);
end2:
  dosclose(D);
  dosclose(S);
end1:

  if (Error == COPYFILE_ERRREADSRCE || Error == COPYFILE_ERRWRITDEST)
    unlink(Dest);
  return(Error);
}
