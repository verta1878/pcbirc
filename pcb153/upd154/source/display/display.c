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

#ifdef LIB
  #include "usersys.h"
#endif

#ifdef __OS2__
  #include <semafore.hpp>
  #include <threads.h>
#else
  #define ACTDELAY       9
#endif


/********************************************************************
*
*  Function:  checkdisplaystatus()
*
*  Desc    :  resets aborted printouts
*/

void LIBENTRY checkdisplaystatus(void) {
  checkstack();
#ifndef LIB
  if (Display.NonStopDuringCmd) {
    Display.NonStopDuringCmd = FALSE;
    Display.WasNonStop = TRUE;    /* remember that it WAS in nonstop mode  */
  }
#endif
  if (Display.AbortPrintout) {
    Display.AbortPrintout = FALSE;
    Display.WasAborted    = TRUE;    /* remember that it WAS aborted */
  }
  Display.AutoMore = FALSE;  /* in case it was on turn it back off */
}


/********************************************************************
*
*  Function:  startdisplay()
*
*  Desc    :  This function is called before any 'display' of data begins.
*             It begins by resetting the 'abort' and 'wasnonstop' flags so
*             that they can be tested later on.
*
*             Then factors such as page length, force non-stop or force count,
*             or non-stop during command come in to play to set the flags
*             accordingly.
*/

void LIBENTRY startdisplay(startdisplaytype Action) {
  checkstack();
  Display.WasNonStop = FALSE;
  Display.WasAborted = FALSE;

  switch (Action) {
    case FORCECOUNTLINES: turnkbdtimeron();
                          Display.CountLines = TRUE;
                          Display.NumLinesPrinted = 0;
                          break;
    case FORCENONSTOP   : Display.CountLines = FALSE;
                          Control.WatchKbdClock = FALSE;
                          break;
    #ifndef LIB
    case NOCHANGE       : if (Display.NonStopDuringCmd) {
                            Display.CountLines = FALSE;
                            Control.WatchKbdClock = FALSE;
                          } else {
                            Display.CountLines = TRUE;
                            Display.NumLinesPrinted = 0;
                          }
                          break;
    #else
    case NOCHANGE       : Display.CountLines = TRUE;
                          Display.NumLinesPrinted = 0;
                          break;
    #endif
  }
}


#ifdef COMM
void LIBENTRY printcom(char *Str) {
  checkstack();
  if (Asy.Online == REMOTE && ! Asy.LostCarrier)
    sendstr(Str,strlen(Str));
}
#endif



/********************************************************************
*
*  Function:  printnocapture()
*
*  Desc    :  This function simply prints to the comm port and display
*             without sending it to the printer or a capture.
*/

static void _NEAR_ LIBENTRY printnocapture(char *Str) {
  checkstack();
  #ifdef COMM
    printcom(Str);
  #endif
  ansi(Str);
}


/********************************************************************
*
*  Function:  showactivity()
*
*  Desc    :  shows activity both on the screen and out the comm port
*/

static char *Forward  = ".";
static char *Backward = " ";
static int ActivityLen;

static void LIBENTRY activitycleanup(void) {
  char Str[80];

  if (ActivityLen != 0) {
    for (Str[0] = 0; ActivityLen; ActivityLen--)
      strcat(Str,Backward);
    printnocapture(Str);
  }
}


#ifdef __OS2__
enum { TH_MUSTEXIT, TH_INTHREAD, TH_EXITED };
static bool volatile ThreadState;
static int ActivityThreadId;
static CSemaphore ActSemaphore;
static int ActivitySuspended;


#pragma argsused
static void THREADFUNC activitythread(void * Ignore) {
  bool   Adding;
  char  *p;

  setdefaultpriority();

  // create the semaphore the first time thru, but since we'll never close it,
  // don't bother creating it again by checking to see if it is opened.
  if (! ActSemaphore.opened()) {
    if (! ActSemaphore.createunique("ACT")) {
      ThreadState = TH_EXITED;
      return;
    }
  }

  ActivityLen = 0;
  Adding = TRUE;

  while (1) {
    ActSemaphore.waitforevent(HALFSECOND);
    ActSemaphore.reset();
    if (ThreadState == TH_MUSTEXIT)
      break;

    if (ActivitySuspended) {
      activitycleanup();
      Adding = TRUE;
    } else {
      if (Adding) {
        p = Forward;
        if (++ActivityLen >= 6)
          Adding = FALSE;
      } else {
        p = Backward;
        if (--ActivityLen <= 0) {
          Adding = TRUE;
          ActivityLen = 0;
        }
      }
      printnocapture(p);
    }
  }

  activitycleanup();
  ThreadState = TH_EXITED;
}

static void LIBENTRY createactivitythread(void) {
  ThreadState = TH_INTHREAD;
  ActivityThreadId = startthread(activitythread,4*1024,NULL);
}

static void LIBENTRY destroyactivitythread(void) {
  if (ThreadState == TH_INTHREAD) {
    ActivitySuspended = TRUE;
    ThreadState = TH_MUSTEXIT;
    ActSemaphore.postevent();
    waitthread(ActivityThreadId);
    ActivitySuspended = 0;
  }
}
#endif  /* ifdef __OS2__ */


void LIBENTRY showactivity(acttype Type) {
  bool static  KbdTimerEnabled;
  #ifndef __OS2__
  bool static  Adding;
  char        *p;
  #endif

  checkstack();
  #ifdef COMM
    turnonxmit();               /* in case we printing, turn on xmit */
  #endif

  switch (Type) {
    case ACTSHOW :
                   #ifndef __OS2__  // the OS/2 uses the activity thread
                     #ifdef PCBCOMM
                       if (Status.TerseMode) {
                         /* don't display dots in terse mode, just get out */
                         checkstatus();
                         return;
                       }
                     #endif
                     if (! timerexpired(4))
                       return;
                     if (Adding) {
                       p = Forward;
                       if (++ActivityLen >= 6)
                         Adding = FALSE;
                     } else {
                       p = Backward;
                       if (--ActivityLen <= 0) {
                         Adding = TRUE;
                         ActivityLen = 0;
                       }
                     }
                     settimer(4,ACTDELAY);  /* show activity every 1/2 second */
                   #endif
                   break;
    case ACTRESUME:
                   #ifdef __OS2__
                     if (ActivitySuspended > 0) {
                       /* decrement one suspension */
                       ActivitySuspended--;
                     }
                     return;
                   #else
                     /* under DOS, there's no suspension because it's not a */
                     /* thread, so just fall thru to ACTBEGIN */
                   #endif
    case ACTBEGIN:
                   #ifdef __OS2__
                     ActivitySuspended = 0;
                     if (ThreadState != TH_INTHREAD)
                       createactivitythread();
                   #else
                     ActivityLen = 0;
                     Adding = TRUE;
                     settimer(4,ONESECOND); /* wait 1 second before beginning */
                   #endif
                   KbdTimerEnabled = Control.WatchKbdClock;
                   Control.WatchKbdClock = FALSE;
                   return;
    case ACTSUSPEND:
                   #ifdef __OS2__
                     /* record that we were active so that we can resume later */
                     if (ActivitySuspended != 0 || ThreadState == TH_INTHREAD) {
                       ActivitySuspended++;
                       if (ActivityLen > 0) {
                         ActSemaphore.postevent();
                         while (ActivityLen > 0)
                           giveup();
                       }
                     }
                     return;
                   #else
                     /* under DOS, there's no suspension because it's not a */
                     /* thread, so just fall thru to ACTEND */
                   #endif

    case ACTEND  :
                   #ifdef __OS2__
                     destroyactivitythread();
                   #else
                     activitycleanup();
                   #endif
                   if (KbdTimerEnabled)
                     turnkbdtimeron();
                   return;
    default      : return;
  }

  #ifndef __OS2__
    printnocapture(p);
  #endif
  checkstatus();
}


/********************************************************************
*
*  Function:  bell()
*
*  Desc    :  sends an ASCII 7 out the comm port and if the Alarm Bell is
*             toggled on it will sound the local speaker as well.
*/

void LIBENTRY bell(void) {
  checkstack();
  #ifdef COMM
    printcom("\a");
  #endif
  if (Status.Alarm)
    mysound(1000,15);
}


/********************************************************************
*
*  Function:  sendtoprinter()
*
*  Desc    :  checks the status of the printer and only sends the string to
*             the printer if it is ready for input
*
*  Returns :  -1 if the printer is not ready or an error occurs, otherwise 0
*/

int LIBENTRY sendtoprinter(char *Str, int StrLen) {
  #ifndef __OS2__
    char       PrnStatus;
  #else
    os2errtype Os2Error;
  #endif

  checkstack();

  #ifndef __OS2__
    PrnStatus = printerready(PcbData.ParallelPortNum);
    /* if ((PrnStatus & PRNSELECTED) == 0) */
    /*  return(-1); */

    if ((PrnStatus & PRNPAPEROUT) != 0)
      return(-1);

    if ((PrnStatus & PRNREADY) == 0) {
      settimer(4,TENSECONDS);
      while (1) {
        if ((printerready(PcbData.ParallelPortNum) & PRNREADY) != 0)
          break;
        if (timerexpired(4))
          return(-1);
        giveup();
        giveup();
      }
    }
  #endif

  if (doswrite(prn,Str,StrLen POS2ERROR) == -1)
    return(-1);
  return(0);
}


/********************************************************************
*
*  Function:  print()
*
*  Desc    :  prints to both the screen (thru ANSI) and to the comm port if
*             a caller is online
*
*  NOTE    :  if the printer is toggled on this routine sends data to the
*             printer as well
*/

void LIBENTRY print(char *Str) {
  checkstack();
  if (Str[0] != 0 && ! Display.AbortPrintout) {
    if (Display.ShowOnScreen) {
      #ifdef COMM
        printcom(Str);
      #endif
      ansi(Str);
      #ifndef LIB
        if (Status.Printer)
          if (sendtoprinter(Str,strlen(Str)) == -1)
            toggleprinter();  /* turn it off */
      #endif
    }
    #ifndef LIB

      if (Status.Capture) {
        if (! Display.ShowOnScreen) {
          char *p;

          /* Watch how many characters are on a line when printing to a */
          /* capture file so that the @pos@ macro will work properly.   */
          /* This is only important if the display is turned off so     */
          /* because, otherwise, @pos@ uses the ANSI emulator cursor    */
          /* position to record track the position on the screen.       */
          Display.NumCharsPrinted += (char) strlen(Str);
          for (p = Str; p != NULL; ) {
            if ((p = strstr(p,"\r\n")) != NULL) {
              p += 2;
              Display.NumCharsPrinted = (char) strlen(p);
            }
          }
        }

        /* if an error occurs when writing the capture file then turn the */
        /* capture off rather then continually going thru the same error  */
        if (dosfputs(Str,&Status.CapFile) == -1) {
          Status.Capture = FALSE;
          /* in case of a message capture - make the disk full message show */
          /* up on the screen so that the caller knows why it aborted early */
          if (! Display.ShowOnScreen) {
            Display.ShowOnScreen = TRUE;
            displaypcbtext(TXT_DISKFULL,NEWLINE|LFBEFORE);
            Display.ShowOnScreen = FALSE;
          }
          Display.AbortPrintout = TRUE;
        }
      }
    #endif
  }
}


void LIBENTRY printnow(char *Str) {
  print(Str);
  #ifdef __OS2__
    updatelinesnow();
  #endif
}


/********************************************************************
*
*  Function:  printcls()
*
*  Desc    :  sends an ansi code to the screen and the comm port to clear
*             the screen
*
*  NOTE    :  this routine calls the low-level ansi and comm drivers directly
*             to avoid having the ansi codes printed on the printer
*
*/

void LIBENTRY printcls(void) {
  checkstack();
  if (Display.ShowOnScreen && ! Display.AbortPrintout) {
    if (Display.DefaultColor > 16 && curcolor() < 16)
      printdefcolor();

    #ifdef COMM
      {
      char *p;
      if (UseAnsi)
        p = "[2J[H";
      else
        p = "";
      printcom(p);
      }
    #endif
    ansi("[2J[H");
    Display.NumLinesPrinted = 0;
  }
}


#ifndef LIB
static void _NEAR_ LIBENTRY clearunguardedarea(void) {
  int Y;

  checkstack();
  for (Y = Display.KeepLines; Y < UsersData.PageLen; Y++) {
    movecursor(0,Y);
    cleareol();
  }
  Display.NumLinesPrinted = (char) (Display.KeepLines+1);
  movecursor(0,Display.NumLinesPrinted);
}
#endif


/********************************************************************
*
*  Function:  moreprompt()
*
*  Desc    :  prints a "More Prompt" and asks the user if he wants to continue
*/

void LIBENTRY moreprompt(moretype Type) {
  #ifndef LIB
  wordtype static Options[1] = {{"FLAG"}};
  int  NumTokens;
  bool FileListing;
  #endif
  bool SaveDisableColor;
  char OldColor;
  DISPLAYTYPE Params;
  int  HelpNum;
  int  TextNum;
  int  Len;
  char *OldPointer;
  char *Mask;
  char *p;
  char Tmp[80];

  checkstack();
  if (Display.AbortPrintout || ! Display.ShowOnScreen)
    return;

  #ifdef COMM
    if (Asy.LostCarrier)
      return;
  #endif

  #ifdef PCBCOMM
    if (Status.TerseMode && Asy.Online == LOCAL)
      return;
  #endif

  if (! Display.Break)
    Type = PRESSENTER;

  savetokenpointer(&OldPointer);
  OldColor = curcolor();

  switch (Type) {
    case MOREPROMPT:
                     #ifndef LIB
                     FileListing = (bool) (Status.InFileListing && checkscreenforfilenames(1,NULL));
                     if (FileListing) {
                       TextNum = (Status.UseSingleLines ? TXT_FILESSHOWPROMPT : TXT_FILESMOREPROMPT);
                       HelpNum = HLP_MOREXFER;
                       Mask    = mask_morexfer;
                       Params  = YESNO|UPCASE|STACKED;
                       Len     = sizeof(Tmp)-1;
                     } else {
                     #endif
                       TextNum = TXT_MOREPROMPT;
                       HelpNum = HLP_MORE;
                       Mask    = mask_more;
                       Params  = YESNO|UPCASE|ERASELINE|STACKED;
                       Len     = 3;
                     #ifndef LIB
                     }
                     #endif
                     break;
    case PRESSENTER: TextNum = TXT_PRESSENTER;
                     HelpNum = NOHELP;
                     Mask    = "";
                     Params  = UPCASE|ERASELINE;
                     Len     = 0;
                     #ifndef LIB
                     FileListing = FALSE;
                     #endif
                     break;
    default        : return;
  }

  if (Display.AutoMore)
    Params |= AUTO;

  SaveDisableColor = Status.DisableColor;
  Status.DisableColor = FALSE;

top:
  Tmp[0] = 0;
  inputfield(Tmp,TextNum,Len,Params,HelpNum,Mask);
  if (Type == PRESSENTER) {
    backupcleareol(awherex());
    printcolor(OldColor);
    Status.DisableColor = SaveDisableColor;
    return;
  #ifndef LIB
  } else if (Display.ClearScrnWhenFull) {
  }
  #else
  }
  #endif

  #ifdef LIB
    tokenize(Tmp);
  #else
    NumTokens = tokenize(Tmp);
  #endif

  p = getnexttoken();

  #ifndef LIB
    if (FileListing) {
      if (NumTokens == 0 || (*(p+1) == 0 && (*p == YesChar || *p == NoChar))) {
        if (UseAnsi && UsersData.PackedFlags.MsgClear && (! UsersData.PackedFlags.ScrollMsgBody) && ! Status.Capture && ! Status.InFileListingLastMore && *p != NoChar)
          printcls();
        else
          backupcleareol(awherex());
      } else
        freshline();
    }

    if (*p == NoChar)
      Display.AbortPrintout = TRUE;
    else {
      switch (*p) {
        case 'F': if (*(p+1) == 0 || option(Options,p,1) != -1) {
                    dispatch("FLAG",PcbData.UserLevels[SEC_D],NumTokens,flagfiles);
                    goto top;
                  }
                  break;
        case 'G': if (*(p+1) == 0)
                    byecommand();
                  break;
/*      case 'A':   the 'A' is no longer used to ARCVIEW a file */
        case 'V': if (*(p+1) == 0) {
                    viewfile(NumTokens-1);
                    newline();
                    displaycmdfile("PREFILE");
                    if (Status.LastView[0] != 0)
                      goto top;
                  }
                  break;
        case 'S': if (*(p+1) == 0) {
                    showfile(NumTokens-1);
                    goto top;
                  }
                  break;
      }
    }

    if (Type == MOREPROMPT && Display.ClearScrnWhenFull && ! Display.AbortPrintout)
      clearunguardedarea();

    Status.LastView[0] = 0;

  #else
    if (*p == NoChar)
      Display.AbortPrintout = TRUE;
  #endif

  restoretokenpointer(&OldPointer);
  printcolor(OldColor);
  Status.DisableColor = SaveDisableColor;
}


/********************************************************************
*
*  Function:  newline()
*
*  Desc    :  prints a carriage return/line feed sequence
*
*             if we are counting lines
*                then check to see if we need to display a more prompt
*             else if not counting lines
*                then make sure we're not watching the clock either!
*/

void LIBENTRY newline(void) {
  /* NOTE:  Display.NumLinesPrinted and Display.PageLen are both unsigned    */
  /* chars with a maximum value of 255.  By setting the user page length to  */
  /* 0, the Display.PageLen is set to 255, which Display.NumLinesPrinted can */
  /* never exceed, resulting in a non-stop display of data.                  */

  checkstack();
  print("\r\n");
  checkstatus();
  if (Display.AbortPrintout)
    return;

  if (Display.CountLines) {
    Display.NumLinesPrinted++;
    if (Display.NumLinesPrinted > Display.PageLen)
      moreprompt(MOREPROMPT);
    return;
  }

  /* Hmmm... this method of avoiding a keyboard timeout was good under DOS   */
  /* but under OS/2 it will create a lot of overhead!  A new way needs to be */
  /* discovered for temporarily stopping the keyboard timer.                 */

  #ifndef __OS2__  // I think this has been coded elsewhere now....
    renewkbdtimer();
  #endif
}


void LIBENTRY pagebreak(int NumLines) {
  /* NOTE:  Display.NumLinesPrinted and Display.PageLen are both unsigned    */
  /* chars with a maximum value of 255.  By setting the user page length to  */
  /* 0, the Display.PageLen is set to 255, which Display.NumLinesPrinted can */
  /* never exceed, resulting in a non-stop display of data.                  */
  checkstack();
  if (Display.CountLines && NumLines > 1)
    if ((unsigned char) (Display.NumLinesPrinted + NumLines) > Display.PageLen)
      moreprompt(MOREPROMPT);
}


/********************************************************************
*
*  Function:  freshline()
*
*  Desc    :  if we're not in column one this will print a CR/LF
*/

void LIBENTRY freshline(void) {
  checkstack();
  if (awherex() != 0)
    newline();
}


/********************************************************************
*
*  Function:  println()
*
*  Desc    :  prints a string and then sends a CR/LF sequence
*/

void LIBENTRY println(char *Str) {
  checkstack();
  print(Str);
  newline();
}


/********************************************************************
*
*  Function:  printcolor()
*
*  Desc    :  prints the color from the color table - if we're in graphics mode
*             AND the "current" color is not the same as the "new" color
*
*  NOTE    :  colorstr() sets the LOCAL color so a call is made only to the
*             printcom() to avoid sending color codes to the capture file or
*             to the printer
*/

void LIBENTRY printcolor(int ColorNum) {
  char Str[20];

  checkstack();
  if (Control.GraphicsMode && Display.ShowOnScreen && ! Display.AbortPrintout) {
    #ifndef LIB
      if (Display.DefaultColor > 16 && ColorNum < 16)
        ColorNum |= (Display.DefaultColor & 0xF0);
    #endif

    if (colorstr(Str,(char) ColorNum)) {
      #ifdef COMM
        printcom(Str);
      #endif
    }
  }
}


/********************************************************************
*
*  Function:  printdefcolor()
*
*  Desc    :  prints the default ansi color string if in graphics mode
*             and the "current" color is NOT the "default" color
*/

void LIBENTRY printdefcolor(void) {
  checkstack();
  printcolor(Display.DefaultColor);
}


/********************************************************************
*
*  Function:  backupdestructive()
*
*  Desc    :  Uses alternating backspace and space characters to perform a
*             destructive backup.
*
*/

void LIBENTRY backupdestructive(int NumColumns) {
  checkstack();
  for (; NumColumns; NumColumns--)
    printnocapture(" ");
}



/********************************************************************
*
*  Function:  backupcleareol()
*
*  Desc    :  Backs the cursor up "NumColumns" positions to the left.
*             In non-graphics it erases characters as it goes.  While in
*             graphics mode it repositions the cursor and erases the entire
*             line using an ANSI command.
*/

void LIBENTRY backupcleareol(int NumColumns) {
  char Temp[10];

  checkstack();
  if (UseAnsi) {
    if (NumColumns > 0) {
      sprintf(Temp,"[%dD[K",NumColumns);
      printnocapture(Temp);
    } else
      printnocapture("[K");
  } else
    backupdestructive(NumColumns);
}


/********************************************************************
*
*  Function:  backup()
*
*  Desc    :  Backs the cursor up "NumColumns" to the left
*/

void LIBENTRY backup(int NumColumns) {
  char Temp[10];

  checkstack();
  if (UseAnsi) {
    sprintf(Temp,"[%dD",NumColumns);
    printnocapture(Temp);
  } else {
    for (; NumColumns; NumColumns--)
      printnocapture("");
  }
}


/********************************************************************
*
*  Function:  forward()
*
*  Desc    :  Forwards the cursor up "NumColumns" to the right
*
*  NOTE    :  Only works in graphics mode!
*/

void LIBENTRY forward(int NumColumns) {
  char Temp[10];

  checkstack();
  sprintf(Temp,"[%dC",NumColumns);
  printnocapture(Temp);
}


void LIBENTRY cleareol(void) {
  int  X;
  char Temp[81];

  checkstack();
  if (UseAnsi)
    printnocapture("[K");
  else {
    X = 79-awherex();
    memset(Temp,' ',X);  Temp[X] = 0;
    print(Temp);
    for ( ; X; X--)
      printnocapture("");
  }
}


void LIBENTRY showmessage(int TextNum) {
  #ifndef LIB
  char OldX;
  char OldY;
  #endif
  char OldColor;

  checkstack();
  OldColor = curcolor();

  #ifndef LIB
    if (Status.FullScreen) {  /* are we in the Full Screen Editor? */
      OldX = awherex();
      OldY = wherey();
      movebottom();
      cleareol();
      displaypcbtext(TextNum,DEFAULTS);
      movecursor(OldY,OldX);
    } else {
  #endif
      freshline();
      displaypcbtext(TextNum,NEWLINE);
  #ifndef LIB
    }
  #endif
  printcolor(OldColor);
}


/********************************************************************
*
*  Function:  checkforansi()
*
*  Desc    :  sends an ANSI esc sequence to ask the caller's terminal
*             emulation where the cursor is. if the caller responds then he
*             must have ANSI capabilities
*
*             "ESC[6n" asks thee the cursor is.
*             "ESC F" puts 8-bito 7-bit mode.
*/

void LIBENTRY checkforansi(void) {
  checkstack();
  #ifdef FIDO
    if (Status.FoundFido)
      return;
  #endif

  UseAnsi = (bool) (UseAnsi || Control.GraphicsMode || (Asy.Online == LOCAL));
  if (UseAnsi)
    return;

  #ifdef COMM
/*  printcom("\r F[6n\r       \r"); */
    printcom(" F   [6n    ");
  #endif
}


#ifndef LIB
void LIBENTRY checkforrip(void)  {
  #ifdef COMM
  bool  NeedToExtend;
  int   Count;
  int   Key;
  int   PCount;
  char *p;
  char  Remove[15];
  char  Buffer[128];
  #endif

  checkstack();
  #ifdef FIDO
    if (Status.FoundFido)
      return;
  #endif

  HasRip = (bool) (HasRip || Control.RipMode);
  if (HasRip) {
    UseAnsi = TRUE;
    return;
  }

  checkforansi();

  if (Asy.Online == LOCAL)
    return;

  #ifdef COMM
    if (! SkipRipDetect) {
//    printcom("\r[!\r   \r");
      printcom("[!   ");

      /* watching for RIPSCRIPxxyyvs */
      memset(Buffer,0,sizeof(Buffer));
      NeedToExtend = TRUE;

      settimer(8,ONESECOND);
      for (Count = 0, PCount = 0, p = NULL; Count < 128; ) {
        Key = commportinkey();

        #ifdef FIDO
          if (Status.FoundFido)
            return;
        #endif

        if (Key > 0) {
          Buffer[Count++] = Key;
          if (Key == 'P')
            PCount++;
          if (PCount >= 2) {
            if (p != NULL || (p = strstr(Buffer,"RIPSCRIP")) != NULL) {
              /* RIPSCRIP is in there, now we need 6 more characters     */
              /* compare total count, minus start position of RIPSCRIP   */
              /* against the number of bytes in RIPSCRIPxxyyvs (i.e. 14) */
              /* and simply accept the last 6 bytes when they are there  */
              if (Count - (int) (p-Buffer) >= 14) {
                UseAnsi = TRUE;
                HasRip  = TRUE;
                memcpy(Status.RipVersion,p + 8,6); Status.RipVersion[6] = 0;
                memcpy(Remove,p,14); Remove[14] = 0;
                substitute(Buffer,Remove,"",sizeof(Buffer));  /* remove RIPSCRIP */
                Count -= 14;
                break;
              }
            }
          }
        }

        // if we may have received a portion of the RIP sequence, then extend
        // the timer to wait for the rest of it to come in
        if (NeedToExtend && strchr(Buffer,'R') != NULL) {
          NeedToExtend = FALSE;
          settimer(8,TWOSECONDS);
        }

        if (timerexpired(8))
          break;

        giveup();
      }

      /* stuff back into the buffer any bytes we may have collected */
      if (Count != 0)
        stuffbufferstr(Buffer);
    }
  #endif
}
#endif


/********************************************************************
*
*  Function:  movecursor()
*
*  Desc    :  Uses Ansi cursor position commands to place the cursor at the
*             specified location.
*/

void LIBENTRY movecursor(int X, int Y) {
  checkstack();
  agotoxy((char) X, (char) Y);

  #ifdef COMM
  {
    char Str[20];

    if (Asy.Online == REMOTE) {
      sprintf(Str,"[%d;%dH",Y+1,X+1);
      printcom(Str);
    }
  }
  #endif
}
