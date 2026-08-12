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
#include <sys/stat.h>
#include <alloc.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include "pcbdiag.h"

#ifdef DEBUG
#include <memcheck.h>
#endif


void scanusersfile(int (*sub)(long Counter, URead *p)) {
  long     Recs;
  long     Counter;
  long     Counter2;
  URead    *UserRecs;
  URead    *p;
  URead    *Ending;
  int      NumBuffers;
  unsigned BuffSize;

  Recs = numrecs(UsersFile,sizeof(URead));
  lseek(UsersFile,0,0);
/*  lseek(TempFile,0,0); */

  NumBuffers = ((coreleft() > 64000U ? 64000U : (int) coreleft()) / sizeof(URead));
  if (NumBuffers > 16)
    NumBuffers &= 0xFFF0;

  if (NumBuffers > 0) {
    if ((UserRecs = (URead *) malloc(NumBuffers * sizeof(URead))) == NULL)
      NumBuffers = 0;
  } else {
    UserRecs = NULL;
    NumBuffers = 0;
  }

  if (NumBuffers) {
    BuffSize = NumBuffers * sizeof(URead);
    Ending = &UserRecs[NumBuffers];

    for (Counter = 0, Counter2 = 1; Counter < Recs; Counter += NumBuffers) {
      if (Recs-Counter < NumBuffers) {
        NumBuffers = (int) (Recs-Counter);
        BuffSize = NumBuffers * sizeof(URead);
        Ending = &UserRecs[NumBuffers];
      }

      if (readcheck(UsersFile,UserRecs,BuffSize) != BuffSize)
        goto goback;

      for (p = UserRecs; p < Ending; p++, Counter2++)
        if (sub(Counter2,p) == -1)
          goto goback;
    }
  } else {
    for (Counter = 0; Counter < Recs; Counter++) {
      if (readcheck(UsersFile,&UsersRead,sizeof(URead)) == (unsigned) -1)
        goto goback;
      sub(Counter,&UsersRead);
    }
  }

goback:
  if (NumBuffers)
    free(UserRecs);

/*  KeyFlags = NOTHING; */
}
