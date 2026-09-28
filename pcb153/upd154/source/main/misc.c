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


#include <malloc.h>

#include "project.h"
#pragma hdrstop

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #define coreleft _memavl
  #include <borland.h>
#endif

#ifdef LIB
  #include "usersys.h"
#endif


#ifdef __cplusplus
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

extern struct ffblk DTA;  /* declared in exist.c */


short LIBENTRY minutesleft(void) {
  #ifdef __OS2__
    return((short) (gettimer(5) / ONEMINUTE));
  #else
    return((short) (((gettimer(5)*10L)/182L)/60L));  /* don't add 1 - we want to see "0 minutes left" */
  #endif
}


short LIBENTRY minutesonline(void) {
  #ifdef __OS2__
    return((short) -(gettimer(6) / ONEMINUTE));
  #else
    return((short) -(((gettimer(6)*10L)/182L)/60L));
  #endif
}


void LIBENTRY addtime(long Seconds, timereasontype Reason) {
  long ElapsedTime;
  long Time;
  long Num;
  long LogonTime;
  long EventTime;
  long SuspendTime;
  long SecondsToEvent;
  int  Minutes;

  if (Reason != NEWTIME && Status.TimeAdjustedForEvent && Seconds > 0)
    return;

  if (Seconds > 0)                            /* are the seconds positive?  */
    Minutes = (int) ((Seconds + 30) / 60);    /*   yes, round UP to minute  */
  else
    Minutes = (int) ((Seconds - 30) / 60);    /*   no, round DOWN to minute */

  switch (Reason) {
    case NEWTIME : if (Seconds == 0) {
                     settimer(5,0);
                     return;
                   }

                   /* Time      = current time                        */
                   /* LogonTime = time caller logged in               */
                   /* Seconds   = time requested to be added          */
                   /* TimeAdded = Seconds - (Time - LogonTime)        */

                   Time = exacttime();
                   LogonTime = Status.LogonMinute * 60L;
                   if (Time < LogonTime)
                     Time += 24L * 60L * 60L;
                   ElapsedTime = Time-LogonTime;

                   #ifndef LIB
                     /* Insure...  only the REMAINDER of the time limit */
                     /* is added on the clock, this function takes care */
                     /* of watching previous time used                  */
                     Num = Seconds - ElapsedTime;
                     #ifdef __OS2__
                       settimer(5,Num * ONESECOND);
                     #else
                       settimer(5,(Num * 182L) / 10);
                     #endif
                   #else
                     /* The library gets its time from pcboard.sys which  */
                     /* has already been calculated by PCBoard - so we'll */
                     /* just use the number straight out of the file      */
                     /* without regard for previous time used             */
                     Num = Seconds;
                     #ifdef __OS2__
                       settimer(5,Num * ONESECOND);
                     #else
                       settimer(5,(Num * 182L) / 10);
                     #endif
                   #endif
                   #ifdef __OS2__
                     settimer(6,-(ElapsedTime * ONESECOND)); /* reset our timer */
                   #else
                     settimer(6,-(ElapsedTime*182) / 10); /* reset our timer */
                   #endif
                   break;
    case CONFTIME: if (Minutes <= Status.ConfAddTime)
                     return;
                   Minutes -= Status.ConfAddTime;
                   Seconds -= Status.ConfAddTime * 60L;
                   Status.ConfAddTime += (short) Minutes;
                   #ifdef __OS2__
                     settimer(5,gettimer(5)+(Seconds * ONESECOND));
                   #else
                     settimer(5,gettimer(5)+(Seconds * 182) / 10);
                   #endif
                   break;
    case CREDTIME: Status.CreditMinutes += (short) Minutes;
                   #ifdef __OS2__
                     settimer(5,gettimer(5)+(Seconds * ONESECOND));
                   #else
                     settimer(5,gettimer(5)+(Seconds * 182) / 10);
                   #endif
                   break;
  }


  #ifdef __OS2__
    Num = (gettimer(5) / ONESECOND);
  #else
    Num = (gettimer(5)*10L)/182L;
  #endif

  if (Status.EventMinute != 0) {
    Time = exacttime();
    SuspendTime = Status.SuspendMinute * 60L;
    EventTime   = Status.EndEventMinute * 60L;

    if (Time >= SuspendTime && Time <= EventTime) {
      Status.TimeAdjustedForEvent = TRUE;
      /* bare minimum give them 10 seconds to try to find out that the    */
      /* event is about to run...  but if there is a "suspense" time then */
      /* give them half of that plus 10 seconds to logon and log back off */
      /*                                                                  */
      /* however, if the caller is returning from a door then don't give  */
      /* the caller any extra time, put 0 time on the clock               */
      if (Reason == NEWTIME)
        #ifdef __OS2__
          settimer(5,((long) (PcbData.EventSuspend/2) * ONEMINUTE) + TENSECONDS);
        #else
          settimer(5,((long) PcbData.EventSuspend * 60 * 91) / 10 + 182);
        #endif
      else
        settimer(5,0);
      return;
    }

    if (SuspendTime < Time)
      SuspendTime += (24L * 60L * 60L);
    SecondsToEvent = SuspendTime - Time;
    if (SecondsToEvent >= 0 && SecondsToEvent < Num) {
      /* if they log on within 2 minutes prior to an event and there is a  */
      /* "suspense" time period then divide the suspense time period in    */
      /* half and add it to the time limit to give the caller more time to */
      /* find out that the event is about to run before getting dumped     */
      if (SecondsToEvent < 120 && PcbData.EventSuspend > 1)
        SecondsToEvent += ((PcbData.EventSuspend * 60) / 2);
      settimer(5,(SecondsToEvent * ONESECOND));
      Status.TimeAdjustedForEvent = TRUE;
    }
  }
}


void LIBENTRY subtime(long Seconds, timereasontype Reason) {
  long Num;
  int  Minutes;

  if (timerexpired(5))
    return;

  if (Seconds > 0)                            /* are the seconds positive?  */
    Minutes = (int) ((Seconds + 30) / 60);    /*   yes, round UP to minute  */
  else
    Minutes = (int) ((Seconds - 30) / 60);    /*   no, round DOWN to minute */

  switch (Reason) {
    case CREDTIME: Status.CreditMinutes -= (short) Minutes;
                   break;
  }

  Num = Seconds * ONESECOND;
  settimer(5,gettimer(5) - Num);
}



#ifndef LIB
int LIBENTRY numrandrecords(char *FileName, int RecSize) {
  if (FileName[0] == 0 || fileexist(FileName) == 255)
    return(0);
  return((int) (DTA.ff_fsize / RecSize));
}
#endif


void LIBENTRY checkpagelen(void) {
  if (Asy.Online == LOCAL)
    Display.PageLen = (char) (min(UsersData.PageLen-1,Display.NumScrnLines-2));
  else
    Display.PageLen = (char) (UsersData.PageLen - 1);
  /* NOTE: Display.PageLen is an "unsigned char", so when the user sets his  */
  /* page length to 0, it sets Display.PageLen to 255.  Since the number of  */
  /* lines printed is ALSO an "unsigned char" and the check is made for the  */
  /* number to be greater than the page length, and since 255+1 = 0, the     */
  /* user's page length of 0, which is Display.PageLen=255, will never issue */
  /* a "more?" prompt.                                                       */
}


int LIBENTRY openfile(char *Name) {
  return(dosopencheck(Name,OPEN_READ|OPEN_DENYNONE));
}


#ifndef LIB
bool LIBENTRY passwordokay(char *Password, char *Input, int TextNum, int BadNum, int NumTries) {
  char Pwrd[20];

  memcpy(Pwrd,Password,12);
  Pwrd[12] = 0;
  stripright(Pwrd,' ');

  while (NumTries) {
    if (TextNum != TXT_YOURPASSWORD) {
      if (strcmp(Pwrd,Status.LastPassword) == 0 || strcmp(Pwrd,UsersData.Password) == 0)
        return(TRUE);
    }
    if (Input[0] == 0)
      inputfield(Input,TextNum,12,ECHODOTS|UPCASE|FIELDLEN|NEWLINE|HIGHASCII,NOHELP,mask_message);
    if (Input[0] == 0 && TextNum == TXT_PASSWORDTOREADMSG)
      return(FALSE);
    if (strcmp(Input,Pwrd) == 0) {
      maxstrcpy(Status.LastPassword,Pwrd,sizeof(Status.LastPassword));
      return(TRUE);
    }
    if (TextNum == TXT_YOURPASSWORD) {
      maxstrcpy(Status.DisplayText,Input,sizeof(Status.DisplayText));
      displaypcbtext(BadNum,NEWLINE);
      logsystext(TXT_PASSWORDFAILURE,SPACERIGHT);
      UsersData.Stats.NumPwrdErrors++;
    } else displaypcbtext(BadNum,NEWLINE|LOGIT);
    NumTries--;
    Input[0] = 0;
  }
  return(FALSE);
}
#endif


#ifndef LIB
#define NUMTYPES 14
bool LIBENTRY asciionbinary(char *FileName) {
  char static BinaryList[NUMTYPES][5] = {".COM",".EXE",".ZIP",".ARJ",".ARC",".PAK",".DWC",".LZH",".GIF",".DBF",".WKS",".WK1",".QWK",".REP"};
  int  X;

  for (X = 0; X < NUMTYPES; X++) {
    if (strstr(FileName,BinaryList[X]) != NULL) {
      displaypcbtext(TXT_ASCIINOTAVAILONBIN,NEWLINE|LOGIT);
      return(TRUE);
    }
  }
  return(FALSE);
}


bool LIBENTRY invalidfilename(char *FileName, VERIFYTYPE Action) {
  bool  HasWildCards;
  int   FullLen;
  int   RootLen;
  int   ExtLen;
  int   MaxFullLen;
  int   MaxRootLen;
  int   MaxExtLen;
  char *p;

  if ((FullLen = strlen(FileName)) == 0)
    return(TRUE);

  if ((p = strrchr(FileName,'\\')) != NULL) {
    strcpy(FileName,p+1);         /* remove path information */
    FullLen = strlen(FileName);   /* get new file length     */
  }

  if ((p = strrchr(FileName,':')) != NULL) {
    strcpy(FileName,p+1);         /* remove drive letter */
    FullLen = strlen(FileName);   /* get new file length */
  }

  HasWildCards = (bool) (strpbrk(FileName,"*?") != NULL);

  /* note:  FILENAME*.EXT* is valid as far as DOS is concerned! */

  if (HasWildCards) {
    if (Action & ALLOWWILDCARDS) {
      if (! (Action & ALLOWSTARDOTSTAR) && strpbrk(FileName,"ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") == NULL)
        goto bad;
      MaxFullLen = 14; /* allow up to two extra wildcards, filename and ext */
      MaxRootLen = 9;  /* allow an extra wildcard at the end of name */
      MaxExtLen = 4;   /* allow an extra wildcard at the end of ext  */
    } else
      goto bad;
  } else {
    MaxFullLen = 12;
    MaxRootLen = 8;
    MaxExtLen = 3;
  }

  if (FullLen > MaxFullLen)
    goto bad;

  if ((p = strchr(FileName,'.')) == NULL) {
    RootLen = FullLen;
    ExtLen  = 0;
  } else {
    RootLen = (int) (p - FileName);
    ExtLen  = FullLen - RootLen - 1;
    /* check for an extra wildcard at the end of the extension */
    if (ExtLen == 4 && p[4] != '*' && p[4] != '?')
      goto bad;
  }

  if (RootLen > MaxRootLen || ExtLen > MaxExtLen)
    goto bad;

  /* check for an extra wildcard at the end of the filename */
  if (RootLen == 9 && FileName[8] != '*' && FileName[8] != '?')
    goto bad;

  if (dosreservedword(FileName))
    goto bad;

  if (Action & ALLOWTEXTONLY) {
    if (asciionbinary(FileName))
      return(TRUE);
  }
  return(FALSE);

bad:
  if (Action & INFORM) {
    if (FullLen > 40)
      FileName[40] = 0;
    maxstrcpy(Status.DisplayText,FileName,sizeof(Status.DisplayText));
    displaypcbtext(TXT_INVALIDFILENAME,NEWLINE|LOGIT|BELL);
  }
  return(TRUE);
}
#endif


/********************************************************************
*
*  Function:  checkmalloc()
*
*  Desc    :  Allocates memory - reports in the callers log if the allocation
*             fails due to lack of memory.
*
*  Returns :  A pointer to the memory allocated or NULL if it fails
*/

void *LIBENTRY checkmalloc(unsigned Size, char *Str) {
  void *p;

  #if defined(BETA) && defined(__OS2__)
    if (heapcheck() == _HEAPCORRUPT) {
      char HeapStr[80];
      sprintf(HeapStr,"ERROR: Heap Corrupted before %s",Str);
      writelog(HeapStr,SPACERIGHT);
    }
  #endif

  if ((p = bmalloc(Size)) == NULL) {
    #ifdef __OS2__
      sprintf(Status.DisplayText,"%s (%u)",Str,Size);
    #else
      sprintf(Status.DisplayText,"%s (%u/%ld)",Str,Size,coreleft());
    #endif
    displaypcbtext(TXT_INSUFFICIENTMEMORY,NEWLINE|LOGIT);
    if (Status.SysopFlag == ' ') {
      Status.SysopFlag = (Status.SysopFlag == 'R' ? ' ' : 'R');
      makepcboardsys();
    }
    return(NULL);
  }
  return(p);
}



#ifndef LIB
bool LIBENTRY seclevelokay(char *Cmd, int SecLvlReq) {
  if (Status.CurSecLevel < SecLvlReq) {
    bell();
    freshline();
    maxstrcpy(Status.DisplayText,Cmd,sizeof(Status.DisplayText));
    displaypcbtext(TXT_MENUSELUNAVAIL,NEWLINE|LFAFTER|LOGIT);
    Status.NumSecViolations++;
    UsersData.Stats.NumSecViol++;
    if (Status.NumSecViolations < 10)
      return(FALSE);
/*  UsersData.SecurityLevel = UsersData.ExpSecurityLevel = 0; */
    displaypcbtext(TXT_SECURITYVIOL,NEWLINE|LFBEFORE|LOGIT);
/*  displaypcbtext(TXT_AUTOMATICLOCKOUT,NEWLINE|LOGIT); */
    displaypcbtext(TXT_AUTODISCONNECTNOW,NEWLINE|LFBEFORE);
    loguseroff(ALOGOFF);
    return(FALSE);
  }
  return(TRUE);
}


void LIBENTRY dispatch(char *Cmd, int SecLvlReq, int NumTokens, void LIBENTRY (*func)(int NumTokens)) {
  if (seclevelokay(Cmd,SecLvlReq)) {
    displaycmdfile(Cmd);
    startdisplay(NOCHANGE);
    func(NumTokens-1);
    checkdisplaystatus();
  }
}


int LIBENTRY option(wordtype *Options, char *Str, int NumOptions) {
  int      Num;
  int      Len;
  int      CmdLen;
  int      WordLen;
  wordtype *p;

  CmdLen = strlen(Str)-1;
  for (Num = 0, p = Options; Num < NumOptions; Num++, p++) {
    for (Len = 0; Len <= CmdLen; Len++) {
      if (Str[Len] != p->Word[Len])
        goto next;
    }
    Len++;
    WordLen = strlen(p->Word);
    if (Len >= WordLen || (Len >= 3 && WordLen >= 3))
      return(Num);
next:;
  }
  return(-1);
}


/********************************************************************
*
*  Function:  addtext()
*
*  Desc    :  This function is used to roll-up command line parameters that
*             have been unrolled by the tokenizer.  It adds each parameter
*             together with a space in between each as long as it all fits
*             within the MaxChars defined.
*
*  Returns :  Str with Text appended to it.
*/

void LIBENTRY addtext(char *Str, char *Text, int MaxChars) {
  int Len;

  Len = strlen(Str);
  if (Len + (int) strlen(Text) + 1 >= MaxChars)
    return;

  if (Len != 0)
    Str[Len++] = ' ';

  strcpy(&Str[Len],Text);
}


/********************************************************************
*
*  Function:  previousconfmatchfound()
*
*  Desc    :  scans all of the conferences which the caller has already JOINED
*             during this session for a matching filename in the field that is
*             specified by the address of the string passed.
*
*             Example:  If the caller has already view the Main Board NEWS file
*             and he joins a conference that shares the same NEWS file then you
*             do not want to show it to him again.  To avoid this you pass the
*             offset to the NEWS file to this function which then checks thru
*             all of the conferences in which the caller has already joined.
*
*  Returns :  TRUE if the string pointed to by offset matches the same field
*             in another conference which the caller has already joined.
*/

bool LIBENTRY previousconfmatchfound(char *Str) {
  unsigned     X;
  int          Offset;
  char        *p;
  char         File1[80];
  char         File2[80];
  pcbconftype  Conf;

  Offset = (int) (Str - (char *) &Status.CurConf);

  p = (char *) &Conf;
  p += Offset;

  fullyqualifiedname(File1,Str,sizeof(File1));

  for (X = 0; X < PcbData.NumAreas; X++) {
    if (X == Status.Conference)
      continue;
    if (! isset(&ConfFlags[JOINED],X))
      continue;
    getconfrecord(X,&Conf);
    fullyqualifiedname(File2,p,sizeof(File2));
    if (strcmp(File1,File2) == 0)
      return(TRUE);
  }
  return(FALSE);
}

#if defined(PCBCOMM) && defined(COMM)
void LIBENTRY tersemodechecksum(char *Str, int Len) {
  char *p;
  char Sum;

  printcom(Str);

  for (Sum = 0, p = &Str[1]; p <= &Str[Len]; p++)
    Sum += *p;

  sendbyte(Sum);
}
#endif
#endif
