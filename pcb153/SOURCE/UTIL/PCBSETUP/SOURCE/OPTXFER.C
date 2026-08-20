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


#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <pcb.h>
#include <misc.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

static char UploadCredit[5];
static char ByteCredit[5];
static char SLOWMASK[] = { 5, ' ', '-', 0,'A','Z'};

int checkbufsize(int Before) {
  if (Before)
    return(0);

  if (PcbData.UploadBufSize >= 4 && PcbData.UploadBufSize <= 64)
    return(0);

  beep();
  showhelp(ERRBUFSIZE);
  return(-1);
}


/********************************************************************
*
*  Function: initfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vBOOL    ,FILEOPTIONS+0,YESNO   , 3, 5, 1,Questions[FLDXFERS+ 0],&PcbData.NoBatchUp        ,NOCLEARFLD,NULL);
  addquest(P, 1,vBOOL    ,FILEOPTIONS+1,YESNO   , 3, 6, 1,Questions[FLDXFERS+ 1],&PcbData.PromoteBatch     ,NOCLEARFLD,NULL);
  addquest(P, 2,vSTR     ,FILEOPTIONS+2,ALLNUM  , 3, 7, 4,Questions[FLDXFERS+ 2],UploadCredit              ,NOCLEARFLD,NULL);
  addquest(P, 3,vSTR     ,FILEOPTIONS+3,ALLNUM  , 3, 8, 4,Questions[FLDXFERS+ 3],ByteCredit                ,NOCLEARFLD,NULL);
  addquest(P, 4,vBOOL    ,FILEOPTIONS+4,YESNO   , 3, 9, 1,Questions[FLDXFERS+ 4],&PcbData.UploadBy         ,NOCLEARFLD,NULL);
  addquest(P, 5,vBOOL    ,FILEOPTIONS+5,YESNO   , 3,10, 1,Questions[FLDXFERS+ 5],&PcbData.TestUploads      ,NOCLEARFLD,NULL);
  addquest(P, 6,vBOOL    ,FILEOPTIONS+6,YESNO   , 3,11, 1,Questions[FLDXFERS+ 6],&PcbData.DisableDriveCheck,CLEAR     ,NULL);
  addquest(P, 7,vBYTE    ,FILEOPTIONS+7,ALLNUM  , 3,12, 2,Questions[FLDXFERS+ 7],&PcbData.UploadBufSize    ,CLEAR     ,checkbufsize);
  addquest(P, 8,vUPSTR   ,FILEOPTIONS+8,SLOWMASK, 3,13,26,Questions[FLDXFERS+ 8],PcbData.SlowDrives        ,NOCLEARFLD,NULL);
  addquest(P, 9,vUPSTR   ,FILEOPTIONS+9,ALLFILE , 3,14,30,Questions[FLDXFERS+ 9],&PcbData.SlowDriveBat     ,CLEAR     ,checkfiles8);

  addquest(P,10,vUNSIGNED,FILEOPTIONS+10,ALLNUM , 3,16, 5,Questions[FLDXFERS+10],&PcbData.StopFreeSpace    ,CLEAR     ,NULL);
}


void pascal transferoptions(void) {
  FldType *Fields;

  if ((Fields = (FldType *) mallochk(NUMXFERS * sizeof(FldType))) == NULL)
    return;

  decimal(UploadCredit,PcbData.UploadCredit);
  decimal(ByteCredit  ,PcbData.ByteCredit);

  initfields(Fields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  fastprint(54,14,"K-bytes",Colors[QUESTION]);
  readscrn(Fields,NUMXFERS-1,0,"Configuration Options","File Transfers",1,NOCLEARFLD);

  PcbData.UploadCredit = (int) timesten(UploadCredit);
  PcbData.ByteCredit = (int) timesten(ByteCredit);

  freescrn(Fields,NUMXFERS-1);
}
