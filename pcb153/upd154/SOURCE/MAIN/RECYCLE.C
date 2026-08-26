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

#include <setjmp.h>

#ifdef __WATCOMC__
  #include <io.h>
#endif

#ifdef __OS2__
  #include <kbd.hpp>
  #include "sem.hpp"
#else
  #include <process.h>
#endif

#include "account.h"
#include "newscr.h"
#include "menu.h"
#include "tossmisc.h"

extern jmp_buf TopOfProgram;
extern int     scrCnt;

#ifdef PCB_DEMO
  ScrType static ExitScreen[2] = {
    { 1,0,"Thanks for trying the PCBoard DEMO!",0x1A},
    {39,0,"To purchase PCBoard call 1-800-356-1686.",0x1E} };
#else
  ScrType static ExitScreen[3] = {
    {12,0,"Thank you for using",0x1A},
    {32,0,"PCBoard",0x1F},
    {40,0,"Professional BBS Software!",0x1A} };
#endif


/********************************************************************
*
*  Function:  exitfunctions()
*
*  Desc    :  Manually executes 'registered-exit-functions'
*
*  NOTES   :  This function hooks into the TURBO C 'hidden' array of exit
*             functions.  Normally the exit() function will call these - but
*             in the case where we perform an exec() function we'll call them
*             manually to be sure it gets take care of!
*
*             NOTE:  This code may be borland-specific and may not work with
*                    WATCOM.  This function is important because some
*                    memory allocations are made and then set to be freed when
*                    the exit functions are called and if we are going to do
*                    an internal recycle then they need to be freed here.
*/

#ifdef USE_ATEXIT   // no longer used!
extern int        _atexitcnt;          /* count of atexit functions */
extern atexit_t   _atexittbl[];        /* array of atexit function pointers */

static void _NEAR_ LIBENTRY exitfunctions(void) {
  /* Execute "atexit" functions */
  while (_atexitcnt--)
    (*_atexittbl[_atexitcnt])();
  _atexitcnt = 0;
}
#endif  // ifdef USE_ATEXIT


void LIBENTRY loguseroff(logtype Type) {
  // The Status.LoggingUserOff semaphore will keep us from re-running the
  // logoff questionnaire

  if (! Status.LoggingUserOff) {
    Control.WatchKbdClock = FALSE;
    Control.WatchSessionClock = FALSE;
    Status.LoggingUserOff = TRUE;
    Status.Logoff = Type;
    #ifdef PCB152
      if (Status.ActStatus != ACT_DISABLED && Type == NLOGOFF && ! Status.AutoLogoff) {
        if (fileexist(PcbData.AccountLogoff) != 255)
          displayfile(PcbData.AccountLogoff,GRAPHICS|SECURITY|LANGUAGE);
      }
    #endif

    if (Type == NLOGOFF && Status.UserRecNo != 1 && ! Status.AutoLogoff)
      logoffscript();
    printdefcolor();
    /* override the errorlevel in case we're supposed to drop to dos between each call */
    if (Status.ErrorLevel == EXIT_ERROR && (PcbData.ExitToDos || LocalOn || FrontEnd))
      Status.ErrorLevel = EXIT_GOODBYE;
  }

  // we'll call recycle anyway because it has its own semaphore
  recycle();
}


void LIBENTRY byecommand(void) {
  bool FilesFlagged;
  char Str[2];
  char *p;

  displaycmdfile("G");

  FilesFlagged = filesflagged();

  p = getnexttoken();
  if (*p == YesChar)
    loguseroff(NLOGOFF);

  if (PcbData.GuardLogoff || FilesFlagged) {
    if (FilesFlagged)
      displaypcbtext(TXT_FILESAREFLAGGED,NEWLINE|LFBEFORE|BELL);
    Str[0] = NoChar;
    Str[1] = 0;
    inputfield(Str,TXT_CONTINUELOGOFF,1,YESNO|NEWLINE|LFBEFORE|FIELDLEN|UPCASE,NOHELP,mask_yesno);
    if (Str[0] != YesChar)
      return;
  }
  Status.InFileListing = FALSE;
  loguseroff(NLOGOFF);
}


/********************************************************************
*
*  Function:  exittodos()
*
*  Desc    :  Creates the ENDPCB file then clears the screen and drops to DOS
*
*             Errorlevels used by PCBoard:
*
*              0 = normal exit at sysop request
*              1 = exit to dos for the event
*              2 = exit to dos for a door
*              3 = exit to dos for remote dos operation
*              4 = exit to dos to run pcbpack  NEVER GETS USED BECAUSE PCBOARD RUNS PACK/SM DIRECTLY
*              5 = exit to dos caused by caller saying goodbye
*              6 = exit to dos to recycle through board.bat
*              7 = exit to dos to handle incoming FAX
*              8 = exit to dos to handle missed connection
*              9 = exit to dos to handle incoming FIDO mail
*             10 = exit to dos to recycle due to having run PPL
*             99 = an error occured which dropped it to dos, board.bat will
*                  recycle the system
*/

static void _NEAR_ LIBENTRY exittodos(void) {
  int File;
  #ifdef __OS2__
    os2errtype Os2Error;
  #endif

  switch (Status.ErrorLevel) {
    default: // if not one of the errorlevels below, then create ENDPCB
             // to force the system to drop to the DOS prompt.
             if ((File = doscreate("ENDPCB",OPEN_RDWR|OPEN_DENYRDWR,OPEN_NORMAL POS2ERROR)) != -1) {
               writecheck(File,"PCBoard - Normal Exit\r\n",23); //lint !e534
               dosclose(File);
             }
             setatt(0,0,79,0,0x1A);
             #ifdef PCB_DEMO
               showscreen(ExitScreen,2);
             #else
               showscreen(ExitScreen,3);
             #endif
             // fall through!
    case  1:
    case  2:
    case  3:
    case  4:
    case  6:
    case  7:
    case  8:
    case  9:
    case 10:
    case 99: agotoxy(0,2);
             setcursor(CUR_NORMAL);
             break;
  }

  #ifdef __OS2__
    updatelinesnow();
    mydelay(25);
  #endif

  #ifdef DEBUG
    mc_endcheck();
  #endif
  exit(Status.ErrorLevel);
}


void LIBENTRY updateuserrecord(void) {
  bool           UnRegisteredUser;
  unsigned short Time;
  unsigned short MinutesOnline;
  char           Str[10];

  UnRegisteredUser = (bool) (Status.UserRecNo == 0 && UsersData.Name[0] != 0);

  if (Status.LatestDate > UsersData.DateLastDirRead)
    UsersData.DateLastDirRead = Status.LatestDate;

  bool StayOnline = Status.Logoff == REMOTEDOS || Status.Logoff == DOOR || Status.Logoff == FILEVIEWSHELL;
  bool LoggingOff = ! StayOnline;

  if (LoggingOff) {
    /* DO NOT CALCULATE THIS STUFF IF THEY ARE DROPPING TO A DOOR PROGRAM!!! */
    Time = (unsigned short) currentminute();

    if (Status.LoggingIn) {
      /* AVOID DEDUCTING HIS TIME IF HE WAS NOT FULLY LOGGED IN!!! */
      /* go ahead and fix up the time calculation if we crossed    */
      /* midnight though - but without changing UsersData values   */
      if (Time < (unsigned) Status.LogonMinute)
        Time += (unsigned short) 1440;
    } else {
      /* NOTE: if we went past midnight then "Time" will be < than "LogonMinute" */
      /* and since we don't want to charge yesterdays time on today's bill we'll */
      /* just set it equal to the current "Time" minus any credits               */
      if (Time < (unsigned) Status.LogonMinute) {
        UsersData.ElapsedTimeOn = Time;
        if (Status.CreditMinutes > 0)
          UsersData.ElapsedTimeOn -= Status.CreditMinutes;
        Time += (unsigned short) 1440;
      } else {
        UsersData.ElapsedTimeOn += (short) ((Time - Status.LogonMinute) - Status.CreditMinutes);
      }
      if (UsersData.ElapsedTimeOn < 0)
        UsersData.ElapsedTimeOn = 0;
    }

    MinutesOnline = (unsigned short) (Time - Status.LogonMinute);

    #ifdef PCB152
      if (Status.LoggingIn == NOTNOW) {
        chargetimeinconf();
        chargetimeonline(Time,MinutesOnline);
        // we've charged them, now disable the charges so that they don't
        // appear to be doubled when displayed down below
        // The reason they appeared to be doubled is that the calculatebalance()
        // function takes the time charges into account and subtracts them
        // from the balance which we have *just* finished doing up above.
        AccountRates.ChargeForTime = 0;
        AccountRates.ChargeForPeakTime = 0;
      }
    #endif

    memcpy(UsersData.LastTimeOn,timestr2(Str),5);
    UsersData.LastTimeOn[5] = 0;
    UsersData.LastDateOn    = getjuliandate();
    if (Status.JulianLogonDate < UsersData.LastDateOn)
      UsersData.DailyDnldBytes = 0;  /* reset after crossing midnight */

  }

  UsersData.LastConference = Status.Conference;

  // if the user record was found, and the caller is NOT in the middle of
  // logging in, then update his user record.
  //
  // Why might the status be LOGINNOW?  Because, if the caller logs in and
  // doesn't know his password, he will be kicked off the system.  We want
  // to show him in the caller log and do everything normal - but we do NOT
  // want to record any charges in his user record otherwise it will mess up
  // things like the accounting and also Andy Keeves' CMS.

  if (Status.UserRecNo != 0 && Status.LoggingIn != LOGINNOW) {
    convertdatatoread(&UsersData,&UsersRead);
    putuserrecord(CHECKDIRTY);
  }

  if (StayOnline) {
    if (Status.MakeUserSys)
      makeusersys();
    if (Status.Logoff == DOOR)
      writeusernetstatus(INADOOR,Status.DoorName);
    else if (Status.Logoff == REMOTEDOS)
      writeusernetstatus(OUTINDOS,NULL);
  } else {
    if (Status.UserRecNo != 0 || UnRegisteredUser) {
      if (Status.LoggingIn == NOTNOW) {
        #ifdef PCB152
          if (Status.ActStatus != ACT_DISABLED) {
            calculatebalance();
            displaypcbtext(TXT_CREDITSUSED,NEWLINE|LFBEFORE|LOGIT);
            if (Status.ActStatus == ACT_ENFORCE)
              displaypcbtext(TXT_CREDITSLEFT,NEWLINE|LOGIT);
          }
        #endif
        #ifdef PCBSTATS
          updatestats(0,0,0,LOGOFF);
        #endif
      }
      ascii(Status.DisplayText,MinutesOnline);  //lint !e644 MinutesOnline *is* being initialized
      displaypcbtext(TXT_MINUTESUSED,NEWLINE|LFBEFORE|LOGIT);
      if (! Asy.LostCarrier) {
        if (Status.Logoff == NLOGOFF) {
          displaypcbtext(TXT_THANKSFORCALLING,LFBEFORE|NEWLINE);
          printcolor(7);  /* reset the color after logging off */
        }
      }
      Asy.Online = OFFLINE;
      writenametolog(Status.Logoff);
    }
  }
}


/********************************************************************
*
*  Function:  recycle()
*
*  Desc    :  If a user was online this function takes care of updating his
*             user record.
*
*  NOTES   :  A non-registered user (user who is not in the users file) will
*             have Status.UserRecNo set to ZERO.  If no user was online or
*             the user hung up without his name being found in the users file
*             then Status.UserRecNo is set to 0xFFFF.
*/

void LIBENTRY recycle(void) {
  bool static    Recycling;
  bool           UnRegisteredUser;
  bool           WasExport;
  #ifndef __OS2__
  char           Path[66];
  #endif
  char           PcbDataFile[66];
  #ifdef COMM
  onlinetype     WasOnline;
  pcbtexttype    Buffer;
  char           Str[100];
  #endif

  if (Recycling)  /* don't let it go recursive! */
    return;

  Recycling = TRUE;
  Control.WatchKbdClock = FALSE;
  Control.WatchSessionClock = FALSE;

  /* make sure the activity has ended (especially under OS/2) */
  showactivity(ACTEND);

  #if defined(COMM)
    WasOnline = Asy.Online;
  #endif

  UnRegisteredUser = (bool) (Status.UserRecNo == 0 && UsersData.Name[0] != 0);

  if (Status.UserRecNo == -1)  /* getuserrecord() returns -1 so     */
    Status.UserRecNo = 0;      /* let's change it to 0 for invalid  */

  if (Status.ReadCapture && (
      Status.KbdTimedOut
      #ifdef COMM
      || Asy.LostCarrier
      #endif
      )) {
    restorepointers();
  }

  // make sure that we get out of chat in case we got here due to a keyboard
  // or session timeout
  if (Status.UserRecNo > 0)
    exitchat();

  #ifdef COMM
    if (Asy.LostCarrier) {
      if (! Asy.IgnoreCDLoss) {
        getpcbtext(TXT_CARRIERLOST,&Buffer);
        ansi("\r\n");
        xlatetext(Str,Buffer.Str);
        ansi(Str);
        if (PcbData.MonitorModem) {
          if (Status.UserRecNo == 0 && UsersData.City[0] != '*') {
            logconnectstring();
            logsystext(TXT_CARRIERLOST,SPACERIGHTAT);
            writelog(LogDivider,LEFTJUSTIFY);
          } else {
            logsystext(TXT_CARRIERLOST,SPACERIGHTAT);
          }
        } else {
          if (Status.UserRecNo != 0 && UsersData.City[0] != '*')
            logsystext(TXT_CARRIERLOST,SPACERIGHTAT);
        }
      }
      runcdlostbat();
    }
  #endif

  if (Status.UserRecNo != 0 || UnRegisteredUser)
    updateuserrecord();
  else {
    // detect when a user never completes the login process, but got as far
    // as entering his name before losing carrier and write it to the log
    if (Status.LoggingIn && Status.Logoff == ALOGOFF && UsersData.Name[0] != 0)
      writenametolog(Status.Logoff);
  }

  if (DebugLevel >= 6)
    writedebugrecord("Recycle 1");

  #ifdef __OS2__
    updatelinesnow();
  #endif

  if (Status.TaggedFileNames != NULL) {     /* free up tagged name memory */
    bfree(Status.TaggedFileNames);
    Status.TaggedFileNames = NULL;
    Status.NumTaggedFiles = 0;
  }

  WasExport = Status.CapFileExport;
  if (Status.CapFile.handle != 0)      /* close capture file */
    closecapturefile();

  if (Status.CapFileName[0] != 0) {    /* remove capture file */
    /* If the sysop hit ALT-F to capture a file (Export) then we don't */
    /* want to lose his capture file if he forgot to ALT-F to turn it  */
    /* off.  Otherwise, if the system was NOT exporting a file, then   */
    /* go ahead and delete the file.                                   */
    if (! WasExport)
      unlink(Status.CapFileName);
    Status.CapFileName[0] = 0;
  }

  #ifdef COMM
    if (Asy.ComPortNumber != 0 && ! LocalOn) {  /* in /LOCALON mode the modem is not opened so skip - same is not true of local user logon */
      /* if we're not EXITING to DOS then see what special modem processing */
      /* needs to be done during the recycle process                        */
      if (Status.Logoff == NLOGOFF || Status.Logoff == ALOGOFF) {
        /* Check to see if we need to reset the modem or take it off hook   */
        if (PcbData.ResetModem || PcbData.OffHook) {
          initcommwindow(HIDE);
          closemodem(TRUE);                               /* hang the guy up if he's still there */
          if (PcbData.OffHook && WasOnline == REMOTE)     /* if we're going to go back OFF HOOK then let's */
            tickdelay(ONESECOND);                         /* wait long enough for the phone co. to hang up */
//        reopenport();                                   /* reopen the modem now */
          openmodem(HIDE);
          /* skip the modem reset/offhook if the phone is ringing */
          if (! Status.Ringing) {
            if (PcbData.ResetModem) {
              if (Asy.Online != OFFLINE)                  /* display only if no longer online */
                displaypcbtext(TXT_RESETMODEM,NEWLINE|LFBEFORE);
              resetmodem(TRUE,TRUE);   //lint !e534
            }
            if (PcbData.OffHook) {
              if (Asy.Online != OFFLINE)                  /* display only if no longer online */
                displaypcbtext(TXT_GOINGOFFHOOK,NEWLINE|LFBEFORE);
              modemoffhook();
              PcbData.LeaveDtrUp = TRUE;
            }
          }
        }
      }

      /* Check to see if the DTR signal should be left on or not....     */
      /* Leave it on if...  we are going to PCBPACK, going to REMOTE DOS */
      /* logging a user off and taking the phone off hook during recycle */
      /* or dropping to DOS with DTR up otherwise drop it.               */

      if (Status.PackFlag != ' ' ||
          Status.Logoff == DOOR ||
          Status.Logoff == REMOTEDOS ||
          #ifndef __OS2__
            Status.Logoff == RUNSM ||
            Status.Logoff == RUNFILER ||
            Status.Logoff == RUNSETUP ||
          #endif
          Status.Logoff == RUNFAX ||
          ModemOffHook ||
          (PcbData.ExitToDos && PcbData.LeaveDtrUp && ! PcbData.Packet))  /* don't leave DTR up if Packet Switch Network - DTR "off" tells PAD we're down */
        closemodem(FALSE);
      else
        closemodem(TRUE);
    }
  #endif

  if (Status.Logoff == DOOR || Status.Logoff == REMOTEDOS) {
    removefinishedfilesfromlist();
    deallocatefilelist(NumFiles < 1 ? THROWAWAY : KEEP);
  } else {
    finishqwkpacket(FALSE);        //lint !e534 close up the files
    removeqwkfiles(TRUE);          // and remove them
    deallocatefilelist(THROWAWAY); // free up FileList memory
  }

  #ifdef DEBUGSTACK
    {
      char StackStr[80];
      if (StackCheck > _stklen)  // if StackCheck is GREATER than the original
        StackCheck = 0;          // stack length, then we've overflowed!
      sprintf(StackStr,"Lowest Free Stack Space: %u",StackCheck);
      writedebugrecord(StackStr);
    }
  #endif

  #ifdef __OS2__
    // need to destroy usernetthread before the usernet file is closed
    destroyusernetthread();
    destroyheartbeatthread();
  #endif

  if (DebugLevel >= 6)
    writedebugrecord("Recycle 2");

  memfreeallscreens();       // free up any saved screen buffers
  deallocatefiledesc();      // free up FileDesc memory
  closeautostuff();          // close auto-kbd-stuff file
  cleanupmsgmemory();        // free up message base memory
  closecallerlog();          // close the callers log

  if (DebugLevel >= 6)
    writedebugrecord("Recycle 3");

  closeusersinffile();       // close the USERS.INF file
  closeusersfile();          // close the USERS file
  tempuserdealloc();         // deallocate temporary user memory
  stopsearch();              // free memory from parsersearch
  cleanupScript();           // free up script memory
  clearMenu();               // free up menu memory
  freecmds();                // free up CMD.LST memory
  resetxfercounters(TRUE);   // reset the transfer counters for the next login

  if (DebugLevel >= 6)
    writedebugrecord("Recycle 4");

  cls();                     // clear screen before running the
  agotoxy(0,0);              // $$logoff.bat file; users complain

  if (Status.Logoff == NLOGOFF || Status.Logoff == ALOGOFF)
    checkforlogoffbat();              /* check for process after logoff  */

  makepcboardsys();

  #ifdef FIDO
    de_init_users();         /* deallocate fido user memory       */
  #endif

  usersdealloc();            /* deallocate user memory            */
  closepcbtext();            /* close pcbtext file if it was open */
  unlink("PCBDOOR.TXT");     /* remove it if it exists            */

  // just in case the usernet record has been updated with a setting such
  // as "drop to dos", let's scan the usernet file one last time just before
  // we close it.
  scanusernet();             //lint !e534
  closeusernet();            /* close the usernet.dat file        */

  closedosfopenfiles();      /* close any open buffered files     */
  closedosopenfiles();       /* close any open non-buffered files */
  clearkbdbuffer();          /* make sure there's nothing left    */

  if (DebugLevel >= 6)
    writedebugrecord("Recycle 5");

  /* if /LOCALON or a front end is used then we want to exit to DOS, but,  */
  /* if it's a door then we don't want to mess up the test for a "shelled" */
  /* door which takes place below by accidently turning ExitToDos on.      */
  if (Status.Logoff != DOOR && (LocalOn || FrontEnd))
    PcbData.ExitToDos = TRUE;

  /* If we will be exiting to DOS for any reason, then we need to make sure  */
  /* that the display buffer is freed before exiting.  So check to see if it */
  /* is toggled off, and if we're exiting to DOS, and if so toggle it on.    */

  if ((! Control.Screen) &&
      (PcbData.ExitToDos ||
       Status.PackFlag >= 254 ||
       Status.Logoff >= REMOTEDOS))
    turndisplayon(FALSE);

  bfreeall();                /* deallocate anything that's left   */
  fbfreeall();               /* deallocate anything that's left   */

  #ifndef __OS2__
    setcbrk(CtrlBreak);
  #endif

  #ifdef USE_ATEXIT  // no longer used!
    exitfunctions();
  #else
    closecnames();
    uninstallhandlers();
  #endif

  turnoffscrollback();

  #ifdef KBD3
    restoreint3();
  #endif

  Recycling = FALSE;

  cls();
  agotoxy(0,0);

  #ifdef __OS2__
    updatelinesnow();
    kbdunregistersemaphore(WaitKeySem);
    WaitKeySem.close();
  #endif

  if (strpbrk(DatFile,":\\") != NULL)
    sprintf(PcbDataFile,"/FILE:%s",DatFile);
  else
    PcbDataFile[0] = 0;

  switch (Status.PackFlag) {
    case 254:
    case 255:
              #ifdef __OS2__
                performshell(PackPgm,PackCmd,SHELLVIACOMMAND,0,TRUE,FALSE,-1);    //lint !e534
              #else
                execl(PackPgm,PackPgm,PackCmd,PcbDataFile,NULL); //lint !e534
              #endif
              break;
  }

  switch (Status.Logoff) {
    #ifndef __OS2__
    case RUNSETUP: strcpy(Path,"PCBSETUP.EXE");
                   srchpath(Path);                               //lint !e534
                   execl(Path,Path,PcbDataFile,"/RELOAD:",NULL); //lint !e534
                   break;
    case RUNFILER: strcpy(Path,"PCBFILER.EXE");
                   srchpath(Path);                               //lint !e534
                   execl(Path,Path,PcbDataFile,"/RELOAD:",NULL); //lint !e534
                   break;
    case RUNSM   : strcpy(Path,"PCBSM.EXE");
                   srchpath(Path);                               //lint !e534
                   execl(Path,Path,PcbDataFile,"/RELOAD:",NULL); //lint !e534
                   break;
    #endif  // ifndef __OS2__

    case DOOR    : if (! PcbData.ExitToDos) {
                     runshelldoor();
                     #ifdef __BORLANDC__
                       _heapmin();  //lint !e534 release any unused areas of the heap
                     #endif
                     longjmp(TopOfProgram,1);  /* go re-load now */
                   }
                   break;
  }

  #ifdef PCB152
    if (scrCnt != 0 && ! PcbData.ExitToDos) {
      PcbData.ExitToDos = TRUE;
      Status.ErrorLevel = EXIT_PPLRECYCLE;
    }
  #endif

/*
  {
    struct heapinfo hi;
    int Total = 0;
    int Num = 0;
    hi.ptr = NULL;
    while (heapwalk(&hi) == _HEAPOK) {
      if (hi.in_use) {
        printf("%lX - %d (%X)\n",hi.ptr,hi.size,hi.size);
        Total += hi.size;
        Num++;
      }
    }
    printf("Num = %d, Total = %d\n",Num,Total);
    mydelay(300);
  }
*/

  #ifdef __BORLANDC__
    if (heapcheck() != _HEAPOK) {
      Status.ErrorLevel = EXIT_RECYCLE;
      PcbData.ExitToDos = TRUE;
    }
  #endif

  if (PcbData.ExitToDos) {
    #ifdef __OS2__
      kbdstopthread();
    #endif
    deregisterdosopencb();
    deregisterdosdupcb();
    deregisterdoscreatecb();
    deregisterdosreadcb();
    deregisterdoswritecb();
    deregisterdosclosecb();
    exittodos();
  }

  #ifdef __BORLANDC__
    _heapmin();  //lint !e534 release any unused areas of the heap
  #endif
  longjmp(TopOfProgram,1);
}
