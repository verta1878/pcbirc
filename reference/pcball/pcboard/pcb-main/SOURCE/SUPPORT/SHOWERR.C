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

#ifdef __OS2__
  #include <semafore.hpp>
#endif

/**********************************/
/* NOTE:  this is a replacement   */
/* module for the SHOWERR.C that  */
/* is normally included in the    */
/* DOS library.  It is customized */
/* here for PCBoard's usage.      */
/**********************************/


#ifndef LIB
unsigned MaxRetryCount = 0xFFFF;
char DosErrorLevel     = 0;
#endif


static void _NEAR_ LIBENTRY displaytext(int PcbTextNum, DISPLAYTYPE DispCtrl) {
  #ifndef __OS2__
    int  SaveError;
    char SaveClass;
    char SaveLocus;
    char SaveAction;

    SaveError  = ExtendedError;
    SaveClass  = ExtendedClass;   /* save these because displaypcbtext() might */
    SaveLocus  = ExtendedLocus;   /* access the disk and reset them!           */
    SaveAction = ExtendedAction;
  #endif

  displaypcbtext(PcbTextNum,DispCtrl);

  #ifndef __OS2__
    ExtendedError  = SaveError;
    ExtendedClass  = SaveClass;   /* now restore them */
    ExtendedLocus  = SaveLocus;
    ExtendedAction = SaveAction;
  #endif
}


/********************************************************************
*
*  Function:  showerror()
*
*  Desc    :  This function is called whenever a DOS error occurs when trying
*             to open or write to a file.
*
*             It sends the appropriate message to the user and/or the callers
*             log.
*
*             If Timed==FALSE then it gives the message and then quits.
*
*             If Timed==TRUE and it's a network version - then it delays 2
*             seconds between each try.
*/

#pragma argsused
static int LIBENTRY showerror(char *path, char *str, bool Timed DOS2ERROR) {
  bool static InHere;
  int         Save;
  char        Temp[256];

  #ifdef __OS2__
    int ExtendedError  = Os2Error->ExtendedError;
    int ExtendedAction = Os2Error->ExtendedAction;
    int ExtendedClass  = Os2Error->ExtendedClass;
    int ExtendedLocus  = Os2Error->ExtendedLocus;
  #endif

/*
  if (Timed) {
    print(".");
    #ifdef __OS2__
      CTimeoutSemaphore Wait;
      Wait.createunique("SHWERR");
      Wait.settimer(ONESECOND);
    while (! Wait.timerexpired(QUARTERSECOND)) {
    #else
      settimer(3,ONESECOND);
    while (! timerexpired(3)) {
      giveup();
    #endif
      checkstatus();
      if (Display.AbortPrintout) {
        settimer(4,-1);                /* this will cause an abort */
        break;
      }
    }
    newline();
  }
*/

  Save = ExtendedError;
  switch (Save) {
    case  2: if (! InHere) {
               InHere = TRUE;
               sprintf(Temp,"(%s) is missing!",path);
               if (Asy.Online != OFFLINE && Status.UserRecNo == 1) {
                 bell();
                 printcolor(PCB_RED);
                 newline();
                 println(Temp);
               }
               writelog(Temp,SPACERIGHT);
               InHere = FALSE;
             }
             return(0);
    case 39: if (! InHere) {
               InHere = TRUE;
               sprintf(Temp,"(%d:%d:%d:%d) Error %s File %s (%s)",Save,ExtendedClass,ExtendedLocus,ExtendedAction,str,path,doserror(NULL,ExtendedError));
               if (Asy.Online != OFFLINE)
                 displaytext(TXT_DISKFULL,NEWLINE|LOGIT|LFBEFORE);
               writelog(Temp,SPACERIGHT);
               InHere = FALSE;
             }
             return(0);
  }

  if (! InHere) {
    sprintf(Temp,"(%d:%d:%d:%d) Error %s File %s (%s)",Save,ExtendedClass,ExtendedLocus,ExtendedAction,str,path,doserror(NULL,ExtendedError));
    InHere = TRUE;
    if (Asy.Online != OFFLINE && (Status.UserRecNo == 1 || Save > 2)) {
      newline();
      println(Temp);
    }
    writelog(Temp,SPACERIGHT);
    InHere = FALSE;
  }

  #ifndef LIB
    if (Save == 4) {
      int X;
      InHere = TRUE;
      sprintf(Temp,"Last Handle = %d",MaxHandle);
      writelog(Temp,SPACERIGHT);
      for (X = 0; X < MAXHANDLES; X++) {
        if (OpenFileNames[X][0] != 0) {
          sprintf(Temp,"Handle %d: [%s]",X,OpenFileNames[X]);
          writelog(Temp,SPACERIGHT);
        }
      }
      InHere = FALSE;
      PcbData.ExitToDos = TRUE;
      Status.ErrorLevel = EXIT_RECYCLE;
/* can't make pcboard.sys because we're out of file handles! */
/*    makepcboardsys();                                      */
      recycle();
    }
  #endif
  return(0);
}


/********************************************************************
*
*  Function:  retrycount()
*
*  Desc    :
*
*  Returns :
*/

unsigned LIBENTRY retrycount(unsigned Count, char *Path, char *Str DOS2ERROR) {
  #ifdef __OS2__
    int ExtendedAction = Os2Error->ExtendedAction;
  #else
    if (ExtendedError == 83) {
      ExtendedError  = Int24Error + 19;  /* errors 19-31 correspond to INT24 errors */
      ExtendedAction = Int24Flags;
    }
  #endif

  if (ExtendedAction > 2) {
    /* show the error and then get out of here */
    showerror(Path,Str,FALSE POS2ERROR2);          /*lint !e534 */
    return(0xFFFF);
  }

  tickdelay(QUARTERSECOND);             /* wait 1/4 second for every retry */

  Count++;
  if (Count < 12)
    return(Count);

  if (Count == 12) {           /* do this after the first several retries  */
    showerror(Path,Str,TRUE POS2ERROR2);  /*lint !e534 */
    displaytext(TXT_NETWORKDELAY,DEFAULTS);     /* show the delay message  */
    /* calculate and set the network timeout */
    #ifdef __OS2__
      settimer(4,PcbData.NetTimeout*ONESECOND);
    #else
      settimer(4,((long)PcbData.NetTimeout*182)/10);
    #endif
  } else {
    tickdelay(ONESECOND);      /* now wait a FULL second between retries */

    print(".");
    checkstatus();
    if (Display.AbortPrintout) {
      settimer(4,-1);                /* this will cause an abort */
      checkdisplaystatus();
      newline();
      return(0xFFFF);
    }

    if (timerexpired(4)) {     /* check to see if we've run out of time  */
      newline();
      return(0xFFFF);          /* return 0xFFFF if time ran out */
    }
  }

  return(Count);
}
