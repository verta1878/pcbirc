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

#include "newscr.h"
#include "menu.h"

static cmdtype *Cmds;
static int  NumCmds;


void LIBENTRY freecmds(void) {
  if (Cmds != NULL) {
    bfree(Cmds);
    Cmds = NULL;
  }
  NumCmds = 0;
  Status.CmdLst[0] = 0;
}


static void _NEAR_ LIBENTRY scanformorecmds(void) {
  int       X;
  int       File;
  unsigned  AddCmds;
  unsigned  AddSize;
  unsigned  NewSize;
  unsigned  CurSize;
  cmdtype  *New;
  char      FileName[66];

top:
  for (X = 0; X < NumCmds; X++) {
    if (Cmds[X].Name[0] == 0 &&
        Status.CurSecLevel >= Cmds[X].SecLevel &&
        Cmds[X].File[0] == '%') {

      checkforalternatelist(FileName,&Cmds[X].File[1],sizeof(cmdtype)); //lint !e534

      if (FileName[0] != 0 &&
          (File = dosopencheck(FileName,OPEN_READ|OPEN_DENYNONE)) != -1) {

        AddSize = (int) (doslseek(File,0,SEEK_END));
        if ((AddCmds = AddSize / sizeof(cmdtype)) != 0) {

          CurSize = NumCmds * sizeof(cmdtype);
          NewSize = AddSize + CurSize - sizeof(cmdtype);
          if ((New = (cmdtype *) bmalloc(NewSize)) != NULL) {

            memcpy(New,Cmds,CurSize);
            if (AddCmds > 1 && X < (NumCmds-1))
              memmove(&New[X+AddCmds],&New[X+1],(NumCmds-X-1)*sizeof(cmdtype));

            doslseek(File,0,SEEK_SET);
            if (readcheck(File,&New[X],AddSize) != (unsigned) -1) {
              NumCmds += AddCmds - 1;
              dosclose(File);
              bfree(Cmds);
              Cmds = New;
              goto top;   // start over in case there are new %INCLUDE files
            } else {
              dosclose(File);
              bfree(New);
            }
          }
        }
      }
    }
  }
}


void LIBENTRY loadcmds(void) {
  int       File;
  unsigned  Size;
  char     *p;
  char      FileName[66];

  if (NoCmdLst)     // if NoCmdLst is set then disable all CMD.LST entries
    return;

  if (PcbData.CmdLst[0] != 0) {           // is there a default CMD.LST file?
    if (Status.CurConf.CmdLst[0] != 0)    //    yes, is there a conf override?
      p = Status.CurConf.CmdLst;          //        yes, load conf CMD.LST
    else
      p = PcbData.CmdLst;                 //        no, load default CMD.LST
  } else
    p = Status.CurConf.CmdLst;            // this will let NO CMD.LST load or
                                          // clear a previous conf CMD.LST

  if (strcmp(p,Status.CmdLst) == 0)
    return;

  freecmds();

  if (*p == 0)
    return;

  checkforalternatelist(FileName,p,sizeof(cmdtype));  //lint !e534
  if (FileName[0] == 0 || (File = dosopencheck(FileName,OPEN_READ|OPEN_DENYNONE)) == -1)
    return;

  Size = (int) (doslseek(File,0,SEEK_END));
  if ((NumCmds = Size / sizeof(cmdtype)) != 0) {
    if ((Cmds = (cmdtype *) bmalloc(Size)) != NULL) {
      doslseek(File,0,SEEK_SET);
      readcheck(File,Cmds,Size);  //lint !e534
    } else
      NumCmds = 0;
  }

  scanformorecmds();

  dosclose(File);
  strcpy(Status.CmdLst,p);
}


bool LIBENTRY runscriptwithparams(char *FileName) {
  int  NumTokens;
  char *p;
  char *OldPointer;
  char Temp[128];
  char Buf[128];

  savetokenpointer(&OldPointer);
  maxstrcpy(Buf,FileName,sizeof(Buf));
  stripright(Buf,' ');

  if ((p = strchr(Buf,' ')) != NULL) {
    *p++ = 0;
    stripleft(p,' ');
    NumTokens = tokenizestr(p);
  } else
    NumTokens = 0;

  strcpy(Temp,Buf);
  if (checkenvfileexist(Temp,sizeof(Temp)) != 255) {
    doScript(Temp,NULL,NumTokens);          //lint !e534
    restoretokenpointer(&OldPointer);
    return(TRUE);
  }

  restoretokenpointer(&OldPointer);
  return(FALSE);
}


bool LIBENTRY runcmds(char *Command, int NumTokens) {
  int     X;
  int     Len;
  int     CmdLen;
  int     WordLen;
#ifdef PCB152
  long    MinutesUsed;
  long    StartTime;
#endif
  cmdtype Cmd;
  char    Str[80];

  if (NumCmds == 0)
    return(FALSE);

  // first check for an EXACT MATCH on the PPL name versus the Command entered

  for (X = 0; X < NumCmds; X++)
    if (strcmp(Command,Cmds[X].Name) == 0)
      goto foundmatch;

  // There was not an EXACT MATCH found
  //
  // If the command entered was less than 2 characters then it can't be an
  // abbreviation so abort out now.

  CmdLen = strlen(Command)-1;
  if (CmdLen == 0)
    return(FALSE);

  // Okay, so maybe it's an abbreviation.  Go check for a match on two or more
  // characters...

  for (X = 0; X < NumCmds; X++) {
    for (Len = 0; Len <= CmdLen; Len++) {
      if (Command[Len] != Cmds[X].Name[Len])
        goto next;  // jump to outer level loop
    }
    Len++;
    WordLen = strlen(Cmds[X].Name);
    if (Len >= WordLen || (Len >= 3 && WordLen >= 3))
      goto foundmatch;

next:;
  }
  return(FALSE);

foundmatch:
  Cmd = Cmds[X];

  if (seclevelokay(Cmd.Name,Cmd.SecLevel)) {
    #ifdef PCB152
      if (Status.ActStatus != ACT_DISABLED) {
        if (insufficientcredits(Cmd.ChargePerMin + Cmd.ChargePerUse,0))
          return(TRUE);

        recordusage("CMD USAGE",Cmd.Name,Cmd.ChargePerUse,1,&UsersData.Account.DebitTPU);
        StartTime = dosgetlongtime();
      }
    #endif

    stripright(Cmd.File,' ');
    strcpy(Str,Cmd.File);  // set up a temporary variable so that we don't destroy the original
    substenvinfile(Str,sizeof(Str));

    if (strstr(Cmd.File,".PPE") != NULL) {
      writeusernetstatus(UNAVAILABLE,NULL);
      if (strchr(Cmd.File,' ') != NULL)
        runscriptwithparams(Str);      //lint !e534
      else
        doScript(Str,NULL,NumTokens);  //lint !e534
      usernetavailable();
    } else if (strstr(Str,".MNU") != NULL) {
      doMenu(Str,NumTokens);           //lint !e534
    } else if (Str[0] == '%') {
      stuffkbdwithfile(&Str[1]);
    } else {
      stuffpplbuffer(Cmd.File);
      for (X = 0; X < NumTokens; X++) {
        stuffpplbuffer(";");
        stuffpplbuffer(getnexttoken());
      }
    }

    #ifdef PCB152
      if (Status.ActStatus != ACT_DISABLED) {
        MinutesUsed = minutesused(StartTime);  //lint !e644 StartTime *is* being initialized
        if (MinutesUsed != 0) {
          recordusage("CMD USAGE MIN",Str,Cmd.ChargePerMin,MinutesUsed,&UsersData.Account.DebitTPU);
          checkaccountbalance();
        }
      }
    #endif
  }
  return(TRUE);
}
