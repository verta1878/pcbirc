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


#include <stdio.h>
#include <string.h>
#include <alloc.h>
#include <misc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
extern int ExitKeyFlag[];
#include <pcb.h>
#include <dosfunc.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif



/********************************************************************
*
*  Function: dopack()
*
*  Desc    : Performs the actual PACK operation.
*
*  Returns : -1 if the operation was aborted otherwise 0
*/

#ifdef __WATCOMC__
extern hdrtype Header;
extern apptype AppHeader;
extern addresstypez Address;
extern addresstype OldAddress;
#endif
int pascal dopack(void) {
  bool      Deleted;
  bool      BadInfRec;
  bool      GetInfRec;
  unsigned  X;
  char     *p;
  long      Counter;
  long      NewNum;
  long      Total;
  long      Bytes;
  long      NumInfRecs;
  char      Temp[9];
  DOSFILE   NewFile;
  char      Reason[51];
  char      Divider[90];
  rectype   Rec;

  setcursor(CUR_BLANK);
  boxcls(3,9,75,20,Colors[OUTBOX],SINGLE);

  p = NULL;
  memset(&NewFile,0,sizeof(DOSFILE));
  memset(&DosTempFile,0,sizeof(DOSFILE));
  memset(&DosUsersInfFile,0,sizeof(DOSFILE));

  // NOTE:  DosUsersFile is already open and locked

  if (createtemp(NONPREALLOC,BUFFERED,FALSE) == -1)
    goto abort;

  if ((p = (char *) mallochk(INFBUFSIZE)) == NULL)
    goto abort;

  if (openusersinffile(PcbData.InfFile) == -1)
    goto abort;

  if (dosfopen(TempInfFileName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&NewFile) == -1)
    goto abort;

  dossetbuf(&DosTempFile,16384);
  dossetbuf(&NewFile,16384);
  dossetbuf(&DosUsersFile,16384);

  NeedToReindex = FALSE;
  Total         = numrecs(UsersFile,sizeof(URead));
  Bytes         = Header.TotalRecSize - sizeof(rectype);
  NumInfRecs    = (dosfseek(&DosUsersInfFile,0,SEEK_END) - InfHeaderSize) / Header.TotalRecSize;
  GetInfRec     = (AliasSupport || VerifySupport || AddressSupport || NotesSupport);
  dosrewind(&DosUsersInfFile);

  if (copyinfbytes(&NewFile,&DosUsersInfFile,p,sizeof(hdrtype)) == -1)
    goto abort;

  for (X = 0; X < Header.NumOfApps; X++) {
    if (copyinfbytes(&NewFile,&DosUsersInfFile,p,sizeof(apptype)) == -1)
      goto abort;
  }

  if (Rem.Printer) {
    if (printheading(TRUE) == -1)
      goto abort;
    memset(Divider,'-',78);
    Divider[78] = '\r';
    Divider[79] = '\n';
    Divider[80] = 0;
  }

  if (Rem.NumDays != 9999) {
    datestr(Temp);
    Rem.Since = datetojulian(Temp) - Rem.NumDays;
  }

  dosrewind(&DosUsersFile);
  dosrewind(&DosTempFile);
  for (Counter = NewNum = 1; Counter <= Total; Counter++) {
    if (readpackinfo() == -1)
      goto abort;

    clsbox(5,19,29,19,Colors[QUESTION]);
    fastprint(5,19,UsersData.Name,Colors[QUESTION]);

    Deleted   = FALSE;
    BadInfRec = FALSE;
    if (Rem.Deleted && (UsersData.DeleteFlag || UsersData.SecurityLevel == 0)) {
      strcpy(Reason,"Deleted or Locked Out");
      Deleted = TRUE;
    } else if (memcmp(UsersData.Name,"~FIDO~",6) != 0) {
      if (UsersData.LastDateOn < Rem.Since) {
        strcpy(Reason,"Last On ");
        strcat(Reason,juliantodate(UsersData.LastDateOn));
        Deleted = TRUE;
      } else if (UsersData.RegExpDate != 0 && UsersData.RegExpDate < Rem.RegExp) {
        strcpy(Reason,"Expired ");
        strcat(Reason,juliantodate(UsersData.RegExpDate));
        Deleted = TRUE;
      }
    }

    if (Deleted) {
      if ((UsersRead.SecurityLevel >= Rem.KeepSec) && (UsersData.DeleteFlag != TRUE) || Counter == 1) {
        /* Kept because of security level or because he's the SYSOP */
        Deleted = FALSE;
      } else {
        if (UsersRead.SecurityLevel == 0 && Rem.KeepLockOut && ! UsersData.DeleteFlag) {
          /* Kept because user is Locked Out */
          Deleted = FALSE;
        } else {
          fastprint(30,19,"Deleted:",Colors[HEADING]);
          fastprint(39,19,Reason,Colors[STATUS]);
          scrollup(5,10,74,19,Colors[OUTBOX]);
        }
      }
    }

    if (Deleted) {
      NeedToReindex = TRUE;
      if (Rem.Printer) {
        if (GetInfRec)
          if (readusersinffile() == -1)
            goto abort;
        convertreadtodata();
        if (dosfputs(Divider,&prn) == -1)
          goto abort;
        LineNum++;
        if (printrec(TRUE) == -1)
          goto abort;
      }
    } else {
      fastprint(30,19,"Copied.",Colors[STATUS]);
      if (UsersRead.RecNum > NumInfRecs) {
        fastprint(38,19,"(bad num - fixing users.inf record)",Colors[HEADING]);
        scrollup(5,10,74,19,Colors[OUTBOX]);
        BadInfRec = TRUE;
      } else {
        if (UsersRead.RecNum >= 1) {
          dosfseek(&DosUsersInfFile,((UsersRead.RecNum-1) * Header.TotalRecSize) + InfHeaderSize,SEEK_SET);
          if (dosfread(&Rec,sizeof(rectype),&DosUsersInfFile) == -1)
            goto abort;
          if (memcmp(Rec.Name,UsersRead.Name,sizeof(UsersRead.Name)) != 0) {
            fastprint(38,19,"(mismatch - fixing users.inf record)",Colors[HEADING]);
            scrollup(5,10,74,19,Colors[OUTBOX]);
            BadInfRec = TRUE;
          }
        } else {
          fastprint(38,19,"(bad num - fixing users.inf record)",Colors[HEADING]);
          scrollup(5,10,74,19,Colors[OUTBOX]);
          BadInfRec = TRUE;
        }
      }

      UsersRead.RecNum = NewNum++;

      encryptusersrec(&UsersRead);

      if (dosfwrite((char *)&UsersRead,sizeof(URead),&DosTempFile) == -1)
        goto abort;

      if (BadInfRec) {
        memset(&Rec,0,sizeof(rectype));
        memcpy(Rec.Name,UsersRead.Name,sizeof(UsersRead.Name));
        if (dosfwrite(&Rec,sizeof(rectype),&NewFile) == -1)
          goto abort;
        if (Bytes != 0 && writezeroes(&NewFile,p,Bytes) == -1)
          goto abort;
      } else {
        if (dosfwrite(&Rec,sizeof(rectype),&NewFile) == -1)
          goto abort;
        if (Bytes != 0 && copyinfbytes(&NewFile,&DosUsersInfFile,p,Bytes) == -1)
          goto abort;
      }
    }
    if (userabort())
      goto abort;
  }

  if (Rem.Printer)
    if (dosfputs("\r\n",&prn) == -1)
      goto abort;
    else if (dosflush(&prn) == -1)
      goto abort;

  free(p);
  p = NULL;
  if (dosfclose(&NewFile) == -1)            /* close USERS.NEW */
    goto abort;
  if (dosfclose(&DosTempFile) == -1)        /* close USERS.TMP */
    goto abort;
  if (dosfclose(&DosUsersFile) == -1)       /* close USERS     */
    goto abort;
  if (dosfclose(&DosUsersInfFile) == -1)    /* close USERS.INF */
    goto abort;
  unlink(BackFileName);                     /* remove USERS.BAK           */
  unlink(BackInfFileName);                  /* remove USERS.IBK           */
  rename(PcbData.UsrFile,BackFileName );    /* rename USERS     USERS.BAK */  //lint !e534
  rename(TempFileName   ,PcbData.UsrFile);  /* rename USERS.TMP USERS     */  //lint !e534
  rename(PcbData.InfFile,BackInfFileName);  /* rename USERS.INF USERS.IBK */  //lint !e534
  rename(TempInfFileName,PcbData.InfFile);  /* rename USERS.NEW USERS.INF */  //lint !e534
  return(0);

abort:
  if (p != NULL)
    free(p);
  NeedToReindex = FALSE;
  dosfclose(&NewFile);
  dosfclose(&DosTempFile);
  dosfclose(&DosUsersFile);
  dosfclose(&DosUsersInfFile);
  unlink(TempInfFileName);
  unlink(TempFileName);
  return(-1);
}
