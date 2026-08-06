#include "pcbsm_externs.h"
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
#include <alloc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
extern int ExitKeyFlag[];
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

// NOTE:   DosBuffer means that the DOSFILE functions are buffering the stream
//         BigBuffer means that a big pool of memory is used to buffer the records

void pascal processusersfile(long Recs, bool ProcessSysop, bool Printer, bool UserInfo, bool DosBuffer, int pascal (*sub)(URead *p)) {
  bool     BigBuffer;
  bool     Aborted;
  unsigned NumBuffers;
  unsigned BuffSize;
  long     Counter;
  URead    *UserRecs;
  URead    *p;
  URead    *Ending;

  BigBuffer = FALSE;
  if (! DosBuffer) {
    // don't try to allocate more than 32K
    NumBuffers  = (coreleft() > 0x8000U ? 0x8000U : (int) coreleft());
    // get a number of buffers that will hold an even number of user records
    NumBuffers /= sizeof(URead);
    // round down to a multiple of 16
    if (NumBuffers > 16)
      NumBuffers &= 0xFFF0;

    if (NumBuffers != 0 && (UserRecs = (URead *) malloc(NumBuffers * sizeof(URead))) != NULL)
      BigBuffer = TRUE;
  }

  Aborted = FALSE;
  if (createtemp(PREALLOC,! BigBuffer,UserInfo) == -1) {
    Aborted = TRUE;
    goto exit;
  }

  if (DosBuffer) {
    if (dossetbuf(&DosUsersFile,16384) == -1) { // try to get a larger buffer
      Aborted = TRUE;
      goto exit;
    }
    if (dossetbuf(&DosTempFile,16384) == -1) {  // try to get a larger buffer
      Aborted = TRUE;
      goto exit;
    }
  }

  if (ProcessSysop == FALSE)
    if (copysysoprecord(DosBuffer,DosBuffer) == -1) {
      Aborted = TRUE;
      goto goback;
    } else {
      Recs--;
    }

  if (BigBuffer) {
    BuffSize = NumBuffers * sizeof(URead);  //lint !e644 NumBuffers initialized if PreBuffered
    Ending = &UserRecs[NumBuffers];         //lint !e644 NumBuffers initialized if PreBuffered

    for (Counter = 0; Counter < Recs; Counter += NumBuffers) {
      if (Recs-Counter < NumBuffers) {
        NumBuffers = (int) (Recs-Counter);
        BuffSize = NumBuffers * sizeof(URead);
        Ending = &UserRecs[NumBuffers];
      }

      if (readcheck(UsersFile,UserRecs,BuffSize) == (unsigned) -1) {
        Aborted = TRUE;
        goto goback;
      }

      for (p = UserRecs; p < Ending; p++) {
        #ifndef DEMO
          decryptusersrec(p);
        #endif

        if (sub(p) == -1) {
          Aborted = TRUE;
          goto goback;
        }

        #ifndef DEMO
          encryptusersrec(p);
        #endif
      }

      if (writecheck(TempFile,UserRecs,BuffSize) == (unsigned) -1 || userabort()) {
        Aborted = TRUE;
        goto goback;
      }
    }
  } else {
    for (Counter = 0; Counter < Recs; Counter++) {
      if (dosfread(&UsersRead,sizeof(URead),&DosUsersFile) == -1) {
        Aborted = TRUE;
        goto goback;
      }

      if (UserInfo) {
        if (readusersinffile() == -1) {
          Aborted = TRUE;
          goto goback;
        }
        convertreadtoconfandmsgptrs(&UsersRead,ConfReg);
      }

      #ifndef DEMO
        decryptusersrec(&UsersRead);
      #endif

      if (sub(&UsersRead) == -1) {
        Aborted = TRUE;
        goto goback;
      }

      #ifndef DEMO
        encryptusersrec(&UsersRead);
      #endif

      if (UserInfo) {
        convertconfandmsgptrstoread(&UsersRead,ConfReg);
        if (writeusersinffile(FALSE) == -1) {
          Aborted = TRUE;
          goto goback;
        }
      }

      if (dosfwrite(&UsersRead,sizeof(URead),&DosTempFile) == -1) {
        Aborted = TRUE;
        goto goback;
      }

      if (userabort()) {
        Aborted = TRUE;
        goto goback;
      }
    }
  }

goback:
  if (BigBuffer)
    dosclose(TempFile);
  else
    dosfclose(&DosTempFile);

  if (UserInfo)
    dosfclose(&DosUsersInfFile);

  if (Aborted) {
    unlink(TempFileName);
    if (UserInfo)
      unlink(TempInfFileName);
  } else if (Printer) {
    dosfputs("\r\n",&prn);   //lint !e534
    dosflush(&prn);
  }

exit:
  if (BigBuffer)
    free(UserRecs);          //lint !e644  UserRecs initialized when PreBuffered

  closeusersfile(Aborted,DosBuffer,UserInfo);
  KeyFlags = NOTHING;
}
