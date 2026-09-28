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


#include "project.h"
#pragma hdrstop

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <malloc.h>
  #include <borland.h>
  #define farmalloc _fmalloc
  #define farfree   _ffree
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#ifdef LIB
  #include "usersys.h"
#endif

#ifndef LIB
  char LogDivider[LOGRECLEN];
#endif

int CallersLog = 0;


void LIBENTRY closecallerlog(void) {
  checkstack();
  if (CallersLog > 0) {
    dosclose(CallersLog);
    CallersLog = 0;
  }
}


static void _NEAR_ LIBENTRY writelogrecord(char *Buf) {
  checkstack();
  #ifndef __OS2__
    if (PcbData.Slaves) {
      int Handle;
      if ((Handle = dosdup(CallersLog)) != -1) {
        writecheck(Handle,Buf,LOGRECLEN);     /*lint !e534 */
        dosclose(Handle);
      }
    } else
  #endif
    writecheck(CallersLog,Buf,LOGRECLEN);   /*lint !e534 */

  if (Status.PrintLog && prn > 0)
    if (sendtoprinter(Buf,LOGRECLEN) == -1)
      Status.PrintLog = FALSE;  /* error, turn it off */
}


void LIBENTRY repositionlogpointer(void) {
  long NumRecs;

  checkstack();
  /* just in case someone has messed with our files - let's go to the end  */
  /* get the length of the file - then make it an even RECORD SIZE by then */
  /* seeking to the record number start location thus rounding off any     */
  /* offset that may have crept into the file (it happens!)                */

  if (CallersLog > 0) {
    NumRecs = doslseek(CallersLog,0,SEEK_END) / LOGRECLEN;
    doslseek(CallersLog,NumRecs * LOGRECLEN,SEEK_SET);
    #ifndef LIB
      if (NumRecs == 0)
        writelogrecord(LogDivider);
    #endif
  }
}


void LIBENTRY openlog(void) {
  #ifndef LIB
  int  SaveLevel;
  #endif
  char Name[66];
  char Str[80];
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  checkstack();
  if (CallersLog > 0)   /* don't reopen the file if it's already open! */
    return;

  if (PcbData.ClrFile[0] == 0)  /* don't open until PcbData has been read in */
    return;

  #if PCB_MAXNODES > 99
  {
    char *p;

    /* search from the end to find the backslash so that we know */
    /* we are looking at filenames and not subdirectory names    */
    if ((p = strrchr(PcbData.ClrFile,'\\')) == NULL)
      p = PcbData.ClrFile;

    if ((p = strstr(p,"CALLER")) != NULL)
      strcpy(p,"CLR");  /* shorten the name to avoid node number problems */
  }
  #endif

  if (PcbData.Network)
    sprintf(Name,"%s%d",PcbData.ClrFile,PcbData.NodeNum);
  else
    strcpy(Name,PcbData.ClrFile);

  #ifndef LIB
    SaveLevel = DebugLevel;
    DebugLevel = 0;
  #endif

  if ((CallersLog = dosopen(Name,OPEN_RDWR|OPEN_DENYNONE POS2ERROR)) == -1)
    if ((CallersLog = doscreate(Name,OPEN_RDWR|OPEN_DENYNONE,OPEN_NORMAL POS2ERROR)) == -1) {
      sprintf(Str,"Can't create caller log: [%s]",Name);
      errorexittodos(Str);
    }

  #ifndef LIB
    DebugLevel = SaveLevel;

    memset(LogDivider,'*',LOGRECLEN-2);
    LogDivider[LOGRECLEN-2] = 13;
    LogDivider[LOGRECLEN-1] = 10;
  #endif

  repositionlogpointer();
}


void LIBENTRY writelog(char *Str, padtype Pad) {
  bool Open;
  char Buffer[LOGRECLEN+1];

  checkstack();
  #ifndef LIB
    if (Status.UserRecNo == -1 && DebugLevel == 0)
      #ifdef FIDO
      if (! Status.FoundFido)
      #endif
      return;
  #endif

  Open = TRUE;
  if (CallersLog == 0) {
    Open = FALSE;
    openlog();
    if (CallersLog == 0)
      return;
  }

  switch (Pad) {
    case LEFTJUSTIFY  : memcpy(Buffer,Str,LOGTEXTLEN);
                        break;
    case SPACERIGHT : memset(Buffer,' ',6);
                        memcpy(&Buffer[6],Str,LOGTEXTLEN-6);
                        break;
  }

  Buffer[LOGTEXTLEN] = 0;
  padstr(Buffer,' ',LOGTEXTLEN);
  if (Pad == SPACERIGHT) {
    Buffer[LOGRECLEN-11] = ' ';
    timestr1(&Buffer[LOGRECLEN-10]);
  }
  Buffer[LOGRECLEN-2] = 13;
  Buffer[LOGRECLEN-1] = 10;
  writelogrecord(Buffer);
  if (! Open)
    closecallerlog();
}


void LIBENTRY writedebugrecord(char *Str) {
  bool Wide;
  int  Len;
  int  StampLen;
  char Buf[63];

  /* write a record out, adding #####s/###k to the end */

  #ifdef __OS2__
    StampLen = 7;   /* %5lds */
  #else
    StampLen = 12;  /* %5lds/%3dk */
  #endif

  checkstack();
  Len = strlen(Str);
  if (Len <= 62-StampLen-6) {       /* 62=max rec, 12=stamp, 6=indent */
    Len = 62-StampLen-6;
    Wide = FALSE;
  } else {
    Len = 62-StampLen;
    Wide = TRUE;
  }

  #ifdef __OS2__
    sprintf(Buf,"%-*.*s %5lds",Len,Len,Str,exacttime());
  #else
    sprintf(Buf,"%-*.*s %5lds/%3dk",Len,Len,Str,exacttime(),coreleft()/1024L);
  #endif

  #ifndef LIB
    int SaveLevel = DebugLevel;
    DebugLevel = 0;
  #endif

  writelog(Buf,(Wide ? LEFTJUSTIFY : SPACERIGHT));

  #ifndef LIB
    DebugLevel = SaveLevel;
  #endif
}


#ifndef LIB
void LIBENTRY writenametolog(logtype Which) {
  char Node[10];
  char Connect[10];
  char DateAndTime[20];
  char Name[26];
  char Str[100];

  checkstack();
  sprintf(DateAndTime,"%s (%5.5s) ",Status.LogonDate,Status.LogonTime);
  if (PcbData.Network)
    sprintf(Node,"(%d)",PcbData.NodeNum);
  else
    Node[0] = 0;

  if (UsersData.Alias[0] != 0 && Status.UseAlias) {
/* we don't want show the alias name in the caller log, */
/* so go get the caller's real name thru the getdisplaynames() function */
    Status.UseAlias = FALSE;
    getdisplaynames();
    maxstrcpy(Name,Status.DisplayName,sizeof(Name));
/* now put things back the way we found them! */
    Status.UseAlias = TRUE;
    getdisplaynames();
  } else
    maxstrcpy(Name,Status.DisplayName,sizeof(Name));

  switch (Which) {
    case LOGON   : sprintf(DateAndTime,"%s (%5.5s) ",Status.LogonDate,Status.LogonTime);
                   if (Asy.Online == LOCAL)
                     strcpy(Connect,"Local");
                   else {
                     lascii(Connect,Asy.CarrierSpeed);
                     if (Asy.ErrorCorrected)
                       addchar(Connect,'E');
                   }
                   sprintf(Str,"%s%s %s (%s) (%c) %s",
                           DateAndTime,
                           Node,
                           Name,
                           Connect,
                           (Control.GraphicsMode ? (Control.RipMode ? 'R' : 'G') : UseAnsi ? 'A' : 'N'),
                           usercity());
                   writelog(Str,LEFTJUSTIFY);
                   break;
    case NLOGOFF :
    case ALOGOFF : datestr(DateAndTime);
                   sprintf(&DateAndTime[8]," (%s) ",timestr2(Str));
                   sprintf(Str,"%s%s %s Off %sormally",
                           DateAndTime,
                           Node,
                           Name,
                           (Which == NLOGOFF ? "N" : "Abn"));
                   writelog(Str,LEFTJUSTIFY);
                   writelogrecord(LogDivider);
                   break;
  }
}
#endif
