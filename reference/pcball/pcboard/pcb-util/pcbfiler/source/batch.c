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
#include <stdlib.h>
#include <string.h>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "rules.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

bool BatchMode;
char CommandLine[128];

#define abool   int

typedef struct {
  int TerseMode:1;
  int Reserved1:1;
  int Reserved2:1;
  int Reserved3:1;
  int Reserved4:1;
  int Reserved5:1;
  int Reserved6:1;
  int Reserved7:1;
} resbyte;

typedef struct {
  abool    Screen;
  abool    PrintLog;
  abool    PageBell;
  abool    Alarm;
  char     SysopFlag;
  abool    ErrorCorrected;
  char     GraphicsMode;
  char     UserNetStatus;
  char     ModemSpeed[5];         /* rate to open com port  */
  char     CarrierSpeed[5];       /* actual speed of caller */
  unsigned UserRecNo;
  char     FirstName[15];
  char     Password[12];
  unsigned LogonMinute;
  int      TimeUsed;
  char     LogonTime[5];
  int      PwrdTimeAllowed;
  int      MaxKBytesAllowed;
  char     Conference;
  char     ConfJoined[5];
  char     ConfScanned[5];
  int      ConfAddTime;
  int      CreditMinutes;
  char     MultiLangExt[4];
  char     Name[25];
  int      MinutesLeft;
  char     NodeNum;
  char     EventTime[5];
  abool    EventActive;
  abool    EventSlide;
  bassngl  MemorizeNum;
  char     ComPortNumber;
  char     PackFlag;
  resbyte  Reserve;
  bool     UseAnsi;
  char     LastEventDate[8]; /* date last event was run */
  int      LastEventMinute;  /* time of last event */
  bool     RemoteDOS;
  bool     EventUpComing;
  bool     StopUploads;
  unsigned Conference2;
} systype;


#ifndef DEMO
static unsigned near pascal readsys(void) {
  systype  Sys;
  int      File;
  int      BytesRead;

  if ((File = dosopencheck("PCBOARD.SYS",OPEN_READ|OPEN_DENYNONE)) == -1)
    return(0xFFFF);

  BytesRead = readcheck(File,&Sys,sizeof(Sys));
  dosclose(File);

  if (BytesRead == sizeof(Sys))
    return(Sys.Conference2);
  else if (BytesRead == 128)
    return(Sys.Conference);
  return(0xFFFF);
}
#endif


static int near pascal intparam(char *Param) {
  if ((Param = strchr(Param,':')) != NULL)
    return(atoi(++Param));
  return(0);
}


void pascal checkforretry(int argc, char *argv[]) {
  for (argc--; argc > 0; argc--) {
    if (argv[argc][0] == '/') {
      strupr(argv[argc]);
      if (findfour("RETR",&argv[argc][1]) == 1)
        MaxRetryCount = intparam(&argv[argc][1])+3;
    }
  }
}


void pascal batch(char *Str) {
  char *p;                    /* 1   2   3   4   5   6   7   8   9*/
  static char BatchCommands[] = "SORTLISTSCANPROCCONFINDEMOVEAUTOPROT";
  #ifndef DEMO
  unsigned BoardNumber;
  unsigned DirNumber;
  bool     ScanDlPath;
  #endif

  if (Str[0] == '/') {
    Work.NonStop = TRUE;
    strupr(Str);
    strcpy(CommandLine,&Str[1]);
    p = parsepaths(CommandLine);
    switch(findfour(BatchCommands,p)) {
      case 0:                      break;
      case 1: process();           break;
      case 2: makefileslist();     break;
      case 3: scandupe();          break;
      #ifndef DEMO
      case 4: if ((p = strchr(Str,':')) == NULL)
                return;
              BoardNumber = atoi(++p);
              if ((p = strchr(p,':')) == NULL)
                return;
              DirNumber = atoi(++p);
              if ((p = strchr(p,':')) != NULL)
                ScanDlPath = (*(p+1) == 'A');
              else
                ScanDlPath = FALSE;
              processdirfile(BoardNumber,DirNumber,ScanDlPath,NULL);
              break;
      case 5: if ((p = strchr(Str,':')) == NULL)
                return;
              p++;
              if (strcmp(p,"PCBOARD.SYS") == 0)
                MenuSelection = readsys() - 1;
              else
                MenuSelection = atoi(p) - 1;
              BatchMode = FALSE;    /* we're not really in batch mode here! */
              confgetdirtoedit();
              break;
      case 6: makeallindexfiles();
              break;
      case 7: automove();
              break;
      case 8: if ((p = strpbrk(Str,":;")) == NULL)
                return;
              BoardNumber = atoi(++p);
              if ((p = strpbrk(p,":;")) == NULL)
                return;
              DirNumber = atoi(++p);
              if ((p = strpbrk(p,":;")) == NULL)
                return;
              processdirfile(BoardNumber,DirNumber,FALSE,++p);
              break;
      case 9: Protected = TRUE;
              break;
      #endif
    }
  }
}
