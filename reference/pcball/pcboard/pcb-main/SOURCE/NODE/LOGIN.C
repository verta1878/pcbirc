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

#include <account.h>
#include "messages.h"
#include "event.h"

#if defined(CPU386) && defined(MULTIPORT)
#include "md5.h"
#endif


#define PWRDTIME   ((18L * ONEMINUTE) / 10)     /* 1.8 min to enter pwrd */
#define MSGTIME    ((39L * ONEMINUTE) / 10)     /* 3.9 min to enter message */

typedef struct {
  char Name[40];
} loginnametype;


// This variable contains the user record number from the time the record
// number is established, on up to the time when the user is officially logged
// into the system.  Until that point, Status.UserRecNo is set to 0 to avoid
// problems with saving a user record back out over top of potentially changed
// information.
static long TempUserRecNo;


#if defined(CPU386) && defined(MULTIPORT)
  static char SentMd5String[32];
         char ReceivedMd5String[35];
#endif


#ifdef FIDO
  #include "defines.h"
  #include "structs.h"
  #include "prototyp.h"

  void LIBENTRY checkforfidoresponse(void) {
    if (PcbData.EnableFido && Status.FoundFido) {
      Status.LoggingIn = NOTNOW;
      attempt_emsi_handshake();          /*lint !e534 */
      recycle();
    }
  }
#endif  /* ifdef FIDO */


/***************************************************************************
static char * near LIBENTRY separator(char *Str) {
  char *p;

  if ((p = strchr(Str,' ')) != NULL)
    return(p);
  return(strchr(Str,';'));
}

static char * near LIBENTRY backseparator(char *Str) {
  char *p;

  if ((p = strrchr(Str,' ')) != NULL)
    return(p);
  return(strrchr(Str,';'));
}

void LIBENTRY splitname(char *Name, char *First, char *Last) {
  char *p;

  if (Name[0] != 0) {
    if ((p = separator(Name)) != NULL) {
      *p = 0;
      strcpy(Last,p+1);
      strcpy(First,Name);
      *p = ' ';
    } else {
      strcpy(First,Name);
      Last[0] = 0;
    }
  } else {
    First[0] = 0;
    Last[0]  = 0;
  }
}
***********************************************************************/


static void _NEAR_ LIBENTRY setlogintime(void) {
  // this function may be called more than once.  The reason is that we want
  // to try to keep the variables initialized, but we also want to keep the
  // exact date and time of the login as accurate as possible.  Preferably,
  // the date/time will reflect the moment the caller typed in the complete
  // name (but before the password was entered).

  timestr1(Status.LogonTime);
  datestr(Status.LogonDate);
  Status.CtodLogonDate   = (unsigned short) ctod(Status.LogonDate);
  Status.JulianLogonDate = datetojulian(Status.LogonDate);
  Status.LogonMinute     = (unsigned short) strtominutes(Status.LogonTime);

  if (Status.LogonMinute < 5) // if caller connects BEFORE midnight, but
    if (PcbData.EventActive)  // logs in AFTER midnight, better reset
      checkforevent();        // the event counters before calculating session limit
}


#ifdef COMM
void LIBENTRY logconnectstring(void) {
  if (PcbData.LogConnectStr) {
    stripright(Asy.ConnectString,' ');
    if (Asy.ConnectString[0] != 0) {
      maxstrcpy(Status.DisplayText,Asy.ConnectString,sizeof(Status.DisplayText));
      logsystext(TXT_MODEM,SPACERIGHT);
    }
    logcalleridstring();
  }
}
#endif

static void _NEAR_ LIBENTRY checksessionandkbdtime(void) {
  if (Status.PwrdTimeAllowed == 0 && TempUserRecNo != 1) {
    Control.WatchKbdClock = FALSE;
    Control.WatchSessionClock = FALSE;
    displayfile(PcbData.ClsFile,GRAPHICS|SECURITY|LANGUAGE);
    Display.Break = FALSE;
    displaypcbtext(TXT_CLOSEDBOARD,NEWLINE|LOGIT);
    loguseroff(ALOGOFF);
    return;
  }

  /* if time available is less than 10 seconds then log them off now       */
  /* 10 seconds was chosen since that is the buffer period given by the    */
  /* addtime() function when the caller sneaks in during the suspense time */

  if (gettimer(5) <= TENSECONDS) {
    Control.WatchKbdClock = FALSE;
    Control.WatchSessionClock = FALSE;
    displaypcbtext((Status.TimeAdjustedForEvent ? TXT_ACCESSDENIEDFOREVENT : TXT_TIMELIMITEXCEEDED),NEWLINE|LOGIT);
    loguseroff(ALOGOFF);
    return;
  }
}


static void _NEAR_ LIBENTRY logtriednames(int NumNames, loginnametype *Tried) {
  int  X;
  char *p;
  pcbtexttype Buf;

  if (NumNames != 0) {
    getsystext(TXT_NAMESTRIED,&Buf);
    p = &Buf.Str[strlen(Buf.Str)];
    for (X = 0; X < NumNames; X++) {
      strcpy(p,Tried[X].Name);
      writelog(Buf.Str,SPACERIGHT);
    }
  }
}


static void _NEAR_ LIBENTRY denyaccessrefuseregister(void) {
  #ifdef COMM
    logconnectstring();
  #endif
  // OS/2 - check if need to finish
  settimer(5,0xFFFF);  /* fix it so that "time limit exceeded" isn't shown */
  displaypcbtext(TXT_DENIEDREFUSETOREG,NEWLINE|LOGIT);
  loguseroff(ALOGOFF);
}


static int _NEAR_ LIBENTRY logonnewuser(void) {
  int  NumTries;
  char Answer[80];
  pcbtexttype Buf;

  Status.NewUser = TRUE;
  startdisplay(NOCHANGE);
  checkpagelen();
  getconfrecord(0,&Status.CurConf); /* force new users into Main Board area */
  PcbData.DisplayNews = 'N';        /* override the "display only if changed" for new users to force them to read the news! */
  checksessionandkbdtime();

  startdisplay(NOCHANGE);
  displayfile(PcbData.NewFile,GRAPHICS|SECURITY|LANGUAGE);
  freshline();
  Answer[0] = YesChar;
  Answer[1] = 0;
  inputfield(Answer,TXT_REGISTER,1,YESNO|FIELDLEN|UPCASE|NEWLINE,NOHELP,mask_yesno);
  if (Answer[0] == NoChar) {
    if (PcbData.UserLevels[SEC_NONREG] > 0) {
      UsersData.SecurityLevel = UsersData.ExpSecurityLevel = Status.CurSecLevel = PcbData.UserLevels[SEC_NONREG];
      convertdatatoread(&UsersData,&UsersRead);
      readpwrdfile(0);
      logsystext(TXT_REFUSEDTOREGISTER,SPACERIGHT);
      return(0);
    }
    return(-1);
  }

  if (PcbData.ClosedBoard) {
    newregscript();
  } else {
    NumTries = 0;
    while (1) {
      NumTries++;
      if (NumTries > 5)
        return(-1);

      if (inputreqfield(UsersData.Password,TXT_NEWPASSWORD,sizeof(UsersRead.Password),ECHODOTS|UPCASE,NOHELP,mask_message) == INPUTREFUSEANSWER) {
        NumTries = 10;
        continue;
      }

      if (badpassword(&UsersData,UsersData.Password,FALSE))
        continue;

      stripright(UsersData.Password,' ');  // badpassword() padded it with spaces

      if (inputreqfield(Answer,TXT_REENTERPASSWORD,sizeof(UsersRead.Password),ECHODOTS|UPCASE,NOHELP,mask_message) == INPUTREFUSEANSWER) {
        NumTries = 10;
        continue;
      }

      if (strcmp(Answer,UsersData.Password) == 0)
        break;
      else
        displaypcbtext(TXT_PWRDSDONTMATCH,NEWLINE);
    }

    if (PcbData.DisableEdits)          /* the ENTER MESSAGE won't work if we allow them online with a */
      PcbData.DisableFilter = TRUE;    /* high ascii name if the filter is turned on - so turn it off */

    getpcbtext(TXT_CITYSTATE,&Buf);
    if (Buf.Str[0] != 0) {
      displaytype Ctrl;
      Ctrl = (MixedCase ? HIGHASCII : HIGHASCII|UPCASE);
      if (inputreqfield(UsersData.City,TXT_CITYSTATE,sizeof(UsersRead.City),Ctrl,NOHELP,(PcbData.DisableEdits ? mask_message : mask_name)) == INPUTREFUSEANSWER)
        return(-1);
    } else
      UsersData.City[0] = 0;

    getpcbtext(TXT_BUSDATAPHONE,&Buf);
    if (Buf.Str[0] != 0) {
      if (inputreqfield(UsersData.BusDataPhone,TXT_BUSDATAPHONE,sizeof(UsersRead.BusDataPhone),HIGHASCII|UPCASE,NOHELP,(PcbData.DisableEdits ? mask_message : mask_phone2)) == INPUTREFUSEANSWER)
        return(-1);
    } else
      UsersData.BusDataPhone[0] = 0;

    getpcbtext(TXT_HOMEVOICEPHONE,&Buf);
    if (Buf.Str[0] != 0) {
      if (inputreqfield(UsersData.HomeVoicePhone,TXT_HOMEVOICEPHONE,sizeof(UsersRead.HomeVoicePhone),HIGHASCII|UPCASE,NOHELP,(PcbData.DisableEdits ? mask_message : mask_phone2)) == INPUTREFUSEANSWER)
        return(-1);
    } else
      UsersData.HomeVoicePhone[0] = 0;

    getpcbtext(TXT_COMMENTFIELDPROMPT,&Buf);
    if (Buf.Str[0] != 0) {
      inputfield(UsersData.UserComment,TXT_COMMENTFIELDPROMPT,sizeof(UsersRead.UserComment),FIELDLEN|UPCASE|NEWLINE|LFBEFORE|HIGHASCII,NOHELP,mask_alphanum);
    } else
      UsersData.UserComment[0] = 0;

    Answer[0] = YesChar;
    Answer[1] = 0;
    getpcbtext(TXT_CLSBETWEENMSGS,&Buf);
    if (Buf.Str[0] != 0) {
      inputfield(Answer,TXT_CLSBETWEENMSGS,1,YESNO|FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_yesno);
    }
    UsersData.PackedFlags.MsgClear = (Answer[0] == YesChar);

    UsersData.Protocol = 'N';
    if (! PcbData.SkipProtocol)
      setprotocol(0);

    newline();

    if (! PcbData.SkipAlias)
      getalias(&UsersData,TempUserRecNo); // ask caller for alias *if* it is supported

    getaddress();  // ask caller for address *if* it is supported

    if (VerifySupport) {
      Answer[0] = 0;
      getpcbtext(TXT_ENTERVERIFYTEXT,&Buf);
      if (Buf.Str[0] != 0) {
        if (inputreqfield(Answer,TXT_ENTERVERIFYTEXT,25,FIELDLEN|UPCASE|NEWLINE|LFBEFORE|HIGHASCII,NOHELP,mask_message) == INPUTREFUSEANSWER)
          return(-1);
        strcpy(UsersData.Verify,Answer);
      }
    }

    getpcbtext(TXT_USESHORTDESC,&Buf);
    if (Buf.Str[0] != 0) {
      Answer[0] = NoChar;
      Answer[1] = 0;
      inputfield(Answer,TXT_USESHORTDESC,1,YESNO|UPCASE|FIELDLEN|NEWLINE,NOHELP,mask_yesno);
      UsersData.Flags.SingleLines = (Answer[0] == YesChar);
    }

    convertdatatoread(&UsersData,&UsersRead);
    if (PcbData.UseNewAskFile)
      newregscript();

    // just in case the user time expired, while filling out the script
    // questionnaire, turn the session monitor OFF while we display the
    // "Adding to index" prompt so that we don't cause ourselves to recycle
    // prior to saving the record out to disk!

    Control.WatchSessionClock = FALSE;
    displaypcbtext(TXT_ADDTOINDEX,NEWLINE|LFBEFORE);

    if ((TempUserRecNo = adduserrecord(&UsersRead,&UsersRec,ConfReg,MsgReadPtr)) == 0) {
      loguseroff(ALOGOFF);
      return(-1);
    }
    UsersData.RecNum = UsersRead.RecNum;
    Control.WatchSessionClock = TRUE;

    // Up above the call to getalias() added the alias name to the index.
    // However, since the user was new, it added the alias with a record number
    // of 0.  This will not work correctly, so we must now REMOVE the alias
    // from the index file and then put it back in with the proper record
    // number.  Kind of a kludge, but previously we were not preventing
    // complete logins for users who answered only some of the questions.
    // Now if not all of the questions are answered (carrier is lost or the
    // user is forced off) it will not leave partial records in the users file.

    if (UsersData.Alias[0] != 0) {
      removeuser(UsersData.Alias);
      addusertoindex(ReadAlias,TempUserRecNo);  //lint !e534
    }
  }

  redisplaystatusline();
  displaypcbtext(TXT_REGINFOSAVED,NEWLINE|LFBEFORE|LOGIT);
  Status.UserRecNo = TempUserRecNo;
  return(0);
}


static void _NEAR_ LIBENTRY loguserinformation(char *Name) {
  TempUserRecNo = 0;
  Status.UserRecNo = 0;
  memset(&UsersData,0,sizeof(UsersData));
  maxstrcpy(UsersData.Name,Name,sizeof(UsersData.Name));
  maxstrcpy(Status.DisplayName,UsersData.Name,sizeof(Status.DisplayName));
  moveback(UsersRead.Name,UsersData.Name,25);
  strcpy(UsersData.City,"* NEW USER *");
  newuserdefaults();
  writeusernetstatus(LOGINTOSYSTEM,NULL);
  writenametolog(LOGON);
}


static void _NEAR_ LIBENTRY removegarbage(char *Str) {
  char *p;

  if (! PcbData.DisableEdits) {
    for (p = Str; *p != 0; ) {
      if ((*p < 'A' && strchr(" -,.'0123456789&",*p) == NULL) || *p > 'Z')
        memmove(p,p+1,strlen(p));
      else
        p++;
    }
  }
  stripall(Str,'\xFF');
  stripright(Str,' ');
  stripleft(Str,' ');
}


#ifdef COMM
static void _NEAR_ LIBENTRY checkbaudrate(int SecLevel) {
  if (Asy.CarrierSpeed < PcbData.LowBaudLimit) {
    if (PcbData.LowBaudSecOverride > 0 && (SecLevel == 0 || SecLevel >= PcbData.LowBaudSecOverride))
      return;
    lascii(Status.DisplayText,Asy.CarrierSpeed);
    if (! PcbData.AllowLowBaud) {
      // OS/2 - check if need to finish
      settimer(5,0xFFFF);  /* fix it so that "time limit exceeded" isn't shown */
      displaypcbtext(TXT_BAUDNOTSUPPORTED,NEWLINE|LOGIT);
      loguseroff(ALOGOFF);
      return;
    }
    if (! timeinrange(PcbData.AllowLowStrt,PcbData.AllowLowStop)) {
      // OS/2 - check if need to finish
      settimer(5,0xFFFF);  /* fix it so that "time limit exceeded" isn't shown */
      displaypcbtext(TXT_BAUDSUPPORTEDFROM,NEWLINE|LOGIT);
      loguseroff(ALOGOFF);
      return;
    }
  }
}
#endif


#if defined(CPU386) && defined(MULTIPORT)
static bool _NEAR_ LIBENTRY md5match(char *Buffer) {
  int   X;
  char *p;
  char  TwoDigits[3];

  TwoDigits[2] = 0;

  /* compare 16 byte Buffer to 32-byte hex representation */

  for (X = 0, p = ReceivedMd5String+1; X < 16; X++) {
    TwoDigits[0] = *p;  p++;
    TwoDigits[1] = *p;  p++;
    if (Buffer[X] != hextoint(TwoDigits))
      return(FALSE);
  }
  return(TRUE);
}
#endif


static void _NEAR_ LIBENTRY getloginname(void) {
  int  NumTries;
  int  NumNames;
  int  NumTokens;
  char Password[13];
  char Str[40];
  char LastStr[40];
  char Answer[40];
  char Name[80];
  loginnametype Tried[5];

  TempUserRecNo = 0;
  NumNames = 0;
  NumTries = 0;

retry:
  Name[0]     = 0;
  Password[0] = 0;
  LastStr[0]  = 0;
  NumTries++;

  if (NumTries > 4) {
    if (Name[0] != 0 || NumNames != 0) {
      loguserinformation(Name);
      logtriednames(NumNames,Tried);
      Status.UserRecNo = 0;            //
      UsersData.Name[0] = ' ';         //  force recycle() to close out the log
      UsersData.Name[1] = 0;           //
    }
    denyaccessrefuseregister();
    loguseroff(ALOGOFF);
    return;
  }

  Answer[0] = 0;
  inputfield(Answer,TXT_YOURFIRSTNAME,39,UPCASE|STACKED|NEWLINE|HIGHASCII,NOHELP,mask_alphanum);

  #ifdef FIDO
    if (strstr(Answer,"~FIDO~") != NULL) {
      strcpy(Tried[NumNames++].Name,Name);
      goto retry;
    }
    checkforfidoresponse();
  #endif

  if (Answer[0] != 0) {
    NumTokens = tokenize(Answer);
    maxstrcpy(Name,getnexttoken(),sizeof(Name));
    removegarbage(Name);
    if (Name[0] == 0)
      goto retry;
    Name[23] = 0;   /* guarantee that FName is not too long */
    NumTokens--;
    if (NumTokens > 0) {
      while (1) {
        strcpy(LastStr,getnexttoken());
        if (NumTokens == 1)
          break;
        NumTokens--;
        strcpy(Str,LastStr);
        removegarbage(Str);
        if (Str[0] != 0) {
          addchar(Name,' ');
          strcat(Name,Str);
        }
      }
    } else {
      Answer[0] = 0;
      inputfield(Answer,TXT_YOURLASTNAME,24-strlen(Name),UPCASE|STACKED|NEWLINE|HIGHASCII,NOHELP,mask_alphanum);
      removegarbage(Answer);
      if (Answer[0] != 0) {
        addchar(Name,' ');
        strcat(Name,Answer);
      }
    }
  }

  Name[25] = 0;
  stripright(Name,' ');
  if (Name[0] == 0)
    goto retry;

  #ifdef FIDO
    if (PcbData.EnableFido && ! Status.AllowHumanCallers) {
      newline();
      println("FIDO MAIL HOUR IN PROGRESS - NO HUMAN CALLERS ALLOWED");
      loguserinformation(Name);
      writelog("FIDO MAIL HOUR IN PROGRESS - NO HUMAN CALLERS ALLOWED",SPACERIGHT);
      loguseroff(ALOGOFF);
      return;
    }
  #endif

  if (strcmp(Name,"SYSOP") == 0) {
    loguserinformation(Name);
    strcpy(Status.DisplayName,Name);
    displaypcbtext(TXT_UNAUTHORIZEDNAME,NEWLINE|LOGIT);
    loguseroff(ALOGOFF);
    return;
  }

  // Is the Name plus the LastStr a valid user name?
  // first copy LastStr to Answer and remove garbage from it
  // if it still isn't blank then check to see if it makes up a user name

  strcpy(Answer,LastStr);
  removegarbage(Answer);
  if (Answer[0] != 0) {
    sprintf(Answer,"%s %s",Name,LastStr);
    Answer[25] = 0;          // make sure the name fits within 25 characters
    removegarbage(Answer);
    setlogintime();
    if ((TempUserRecNo = finduser(Answer)) != -1) {
      /* yes, get rid of LASTSTR and go log the user on */
      stripright(Answer,' ');  /* finduser() will have padded spaces on the end! */
      strcpy(Name,Answer);
      LastStr[0] = 0;
      Status.UserRecNo = TempUserRecNo; // temporarily assign it so getuserrecord() can retrieve the record
      getuserrecord(FALSE,FALSE);
      Status.UserRecNo = 0;  // clear it back out until officially logged in
      goto found;
    }
  }

  // Name+LastStr was NOT a valid user name.  Two possibilities now exist:
  // 1) LastStr is the password for the user called "Name"
  // 2) Or the user "Name" does not yet exist on the system (i.e. newuser)

  setlogintime();
  if ((TempUserRecNo = finduser(Name)) != -1) {
    // we found a user ...  Name is either a single name or something more
    // that.   If the name was entered on separate lines then LastStr is
    // empty so Name is could be a First and Last name.
    //
    // If Name is a single name then two possibilities exist:
    //
    // 1) We found the right record
    // 2) Or we found the WRONG record - example:  Typing "BOB JAMES" to login
    //    would pull up a record for BOB.  Now...  do we:  a) assume that
    //    "JAMES" (ie LastStr) is the PASSWORD for the BOB that we found? or
    //    b) assume that BOB JAMES is a NEW USER trying to log in?
    //
    // Let's establish a procedure whereby, if "Name" is found, AND if "Name"
    // is a SINGLE WORD, AND if "LastStr" does not match the password....
    // then, it must be a different user so drop through.  Otherwise, if
    // everything DOES match up then proceed with the login.

    settimer(8,gettimer(5));  // save the current session count down timer
    Status.UserRecNo = TempUserRecNo; // temporarily assign it so getuserrecord() can retrieve the record
    getuserrecord(FALSE,FALSE);
    Status.UserRecNo = 0;  // clear it back out until officially logged in
    stripright(Name,' ');

    // Based on the above procedure, it would be possible to log into the
    // system using a name such as "DAVID TERRY _" and at this point, the
    // third parameter is considered the password.  But because it is
    // "garbage" it will be removed and the caller could then log into the
    // system as a NEW USER and would be known as "DAVID TERRY" even though
    // he would have a completely different record.  To fix this, we will
    // check to see if there is "nothing left" after removing garbage from
    // the password.  If that is the case, then we will consider the password
    // to be blank.  Otherwise, it is valid to have garbage characters in
    // the password --- we just won't allow the password to be made up
    // entirely of garbage characters!

    strcpy(Answer,LastStr);
    removegarbage(Answer);
    if (Answer[0] == 0)
      LastStr[0] = 0;

    // now check to see if there is a password left

    if (LastStr[0] == 0 || strcmp(UsersData.Password,LastStr) == 0)
      goto found;

    settimer(5,gettimer(8));  // restore the current session count down timer
    Status.UserRecNo = -1;    // because getuserrecord() may have found a user
    redisplaystatusline();    // who had used up all of his time already
    Status.User = USER;       // also, in case of a name match on the sysop, revert back to user status
  }

  stripright(Name,' '); /* finduser() will have padded spaces on the end! */

  /* re-build the entire logon string to show to the caller when not found */
  sprintf(Answer,"%s %s",Name,LastStr);
  Answer[sizeof(UsersData.Name)] = 0;
  removegarbage(Answer);
  strcpy(Name,Answer);

  if (! PcbData.AllowOneName) {
    if (strchr(Name,' ') == NULL) {
      displaypcbtext(TXT_REQUIRETWONAMES,NEWLINE|LFBEFORE|LFAFTER);
      strcpy(Tried[NumNames++].Name,Name);
      goto retry;
    }
  }

  // user name not found
  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));
  displaypcbtext(TXT_NOTINUSERSFILE,NEWLINE);

  Answer[0] = 'C';
  Answer[1] = 0;
  inputfield(Answer,TXT_REENTERNAME,1,FIELDLEN|UPCASE|NEWLINE|LFAFTER,NOHELP,mask_cr);

  if (Answer[0] == 'R') {
    strcpy(Tried[NumNames++].Name,Name);
    goto retry;
  }

  /* If they type 'C' or press Enter then continue down */
  loguserinformation(Name);
  logtriednames(NumNames,Tried);

  if (PcbData.ClosedBoard && fileexist(PcbData.RegFile) == 255) {
    #ifdef COMM
      logconnectstring();
    #endif
    Control.WatchKbdClock = FALSE;
    Control.WatchSessionClock = FALSE;
    displayfile(PcbData.ClsFile,GRAPHICS|SECURITY|LANGUAGE);
    Display.Break = FALSE;
    displaypcbtext(TXT_CLOSEDBOARD,NEWLINE|LOGIT);
    loguseroff(ALOGOFF);
    return;
  }

  if (readtcanfile(Name)) {
    #ifdef COMM
      logconnectstring();
    #endif
    displaypcbtext(TXT_REALNAMESONLY,NEWLINE|LOGIT);
    loguseroff(ALOGOFF);
    return;
  }

  #ifdef PCB_DEMO
    if (numrecs(UsersFile,sizeof(UsersRead)) >= 10) {
      displaypcbtext(TXT_SYSTEMUNAVAILABLE,NEWLINE|LOGIT);
      loguseroff(ALOGOFF);
      return;
    }
  #endif

  #ifdef COMM
    checkbaudrate(Status.CurSecLevel);
  #endif

  if (logonnewuser() == -1)
    denyaccessrefuseregister();

  // fully logged in now, set status to normal
  // then assign the user record number
  // then re-get the user record in case it was changed (by another node)
  // in between when we first found it and now
  Status.LoggingIn = NOTNOW;
  Status.UserRecNo = TempUserRecNo;
  getuserrecord(FALSE,FALSE);
//UsersData.NumTimesOn++;
  return;

found:
  // check for a bad index file - we might have picked up the wrong record!
  if (strcmp(Name,UsersData.Name) != 0 && strcmp(Name,UsersData.Alias) != 0) {
    freshline();
    printcolor(PCB_RED);
    println("USERS/PCBNDX out of synch");
    writelog("USERS/PCBNDX out of synch",SPACERIGHT);
    goto retry;
  }


  // This line tells PCBoard to use the alias name only if that is what the
  // caller used at login.  This prevents a password failure prompt from
  // displaying the wrong name to the caller (causing a breach of security).
  Status.UseAlias = (bool) (strcmp(Name,UsersData.Alias) == 0);
  getdisplaynames();

  #if defined(CPU386) && defined(MULTIPORT)
    if (Asy.Online == REMOTE && LastStr[0] == 0 && ReceivedMd5String[0] != 0) {
      char Buffer[80];
      char Md5Digest[16];

      memset(Md5Digest,0,sizeof(Md5Digest));
      strcpy(Buffer,SentMd5String);
      strcat(Buffer,UsersData.Password);
      md5String(Buffer,strlen(Buffer),Md5Digest);
      if (md5match(Md5Digest))
        strcpy(LastStr,UsersData.Password);
      else
        ReceivedMd5String[0] = 0;
    } else
      ReceivedMd5String[0] = 0;
  #endif

  maxstrcpy(Password,LastStr,sizeof(Password));
  if (PcbData.ConfirmCaller && strcmp(Password,UsersData.Password) != 0) {
    settimer(8,gettimer(5));  /* save the current session count down timer */
    settimer(5,PWRDTIME);     /* fix it so that "time limit exceeded" isn't shown */
    newline();
    println(Name);

    if (AddressSupport && UsersData.Address.City[0] != 0 && UsersData.Address.State[0] != 0) {
      pcbtexttype Buf;
      getpcbtext(TXT_CITYSTATE,&Buf);
      if (UsersData.City[0] == 0 || Buf.Str[0] == 0) {
        print(UsersData.Address.City); print(", "); println(UsersData.Address.State);
      } else
        println(UsersData.City);
    } else
      println(UsersData.City);

    Answer[0] = YesChar;
    Answer[1] = 0;
    inputfield(Answer,TXT_ISTHISCORRECT,1,YESNO|FIELDLEN|UPCASE|NEWLINE|LFBEFORE|LFAFTER|AUTO,NOHELP,mask_yesno);
    if (Answer[0] == NoChar) {
      displaypcbtext(TXT_CHANGENAMES,NEWLINE|LFAFTER);
      strcpy(Tried[NumNames++].Name,Name);
      // having retrieved the user information, the Time Adjusted for Event
      // status may have been set.  Yet, if the time limit for the person that
      // really logs on doesn't exceed the event time, the time adjusted
      // message should not be shown, so turn it off now.
      Status.TimeAdjustedForEvent = FALSE;
      goto retry;
    }
    settimer(5,gettimer(8));  /* restore the session count down timer */
  }

  writeusernetstatus(LOGINTOSYSTEM,NULL);
  writenametolog(LOGON);
  logtriednames(NumNames,Tried);

  #if defined(CPU386) && defined(MULTIPORT)
    if (Asy.Online == REMOTE && ReceivedMd5String[0] != 0) {
      char Md5LoginStr[80];
      sprintf(Md5LoginStr,"MD5LOGIN=%s",ReceivedMd5String);
      writelog(Md5LoginStr,SPACERIGHT);
    }
  #endif

  if (UsersData.SecurityLevel == 0 || Status.CurSecLevel == 0) {
    #ifdef COMM
      logconnectstring();
    #endif
    Control.WatchKbdClock = FALSE;
    Control.WatchSessionClock = FALSE;
    displaypcbtext(TXT_LOCKEDOUT,NEWLINE|LFBEFORE|LOGIT);
    loguseroff(ALOGOFF);
    return;
  }

  #ifdef COMM
    checkbaudrate(Status.CurSecLevel);
  #endif

  checksessionandkbdtime();

  /* if the caller doesn't have a password then finish logging him in */

  if (UsersData.Password[0] == 0) {
    // fully logged in now, set status to normal
    // then assign the user record number
    // then re-get the user record in case it was changed (by another node)
    // in between when we first found it and now
    Status.LoggingIn = NOTNOW;
    Status.UserRecNo = TempUserRecNo;
    getuserrecord(FALSE,FALSE);
    UsersData.NumTimesOn++;
    return;
  }

  settimer(8,gettimer(5));   /* save the current session count down timer */
  if (gettimer(8) > PWRDTIME)
    settimer(5,PWRDTIME);      /* limit time to enter password */

  /* if we already have a password check it - otherwise get one */

  if (passwordokay(UsersData.Password,Password,TXT_YOURPASSWORD,TXT_WRONGPWRDENTERED,4)) {
    settimer(5,gettimer(8));   // restore the session count down timer
    // fully logged in now, set status to normal
    // then assign the user record number
    // then re-get the user record in case it was changed (by another node)
    // in between when we first found it and now
    Status.LoggingIn = NOTNOW;
    Status.UserRecNo = TempUserRecNo;
    getuserrecord(FALSE,FALSE);
    UsersData.NumTimesOn++;
    return;
  }

  #ifdef COMM
    logconnectstring();
  #endif

  if (PcbData.AllowPwrdComment) {
    displaypcbtext(TXT_WRONGPWRD,NEWLINE|LFBEFORE);
    if (gettimer(8) > MSGTIME)
      settimer(5,MSGTIME);       /* limit time to enter message */
    entercomment(-2);     /* see if user wants to leave the sysop a comment */
  }

  displaypcbtext(TXT_DENIEDWRONGPWRD,NEWLINE|LFBEFORE|LOGIT);
  loguseroff(ALOGOFF);
}


static void _NEAR_ LIBENTRY writelangused(void) {
  pcbtexttype Buf;

  if (PcbData.MultiLingual) {
    getpcbtext(TXT_LANGACTIVE,&Buf);
    writelog(Buf.Str,SPACERIGHT);
  }
}



void LIBENTRY login(void) {
  bool QuickLogon;
  bool NonStop;
  #ifdef COMM
  char Answer[15];
  #endif
  #ifndef PCB_DEMO
  #ifdef COMM
  char *Mask;
  #endif
  #endif
  char Str[81];

  writeusernetstatus(LOGINTOSYSTEM,NULL);

  #ifndef __OS2__
    settimer(7,Control.NetTimer);
  #endif

  settimer(5,PWRDTIME);
  Control.WatchSessionClock = TRUE;
  turnkbdtimeron();

  cls();         /* clear the ENTIRE screen */
  agotoxy(0,0);
  if (Control.Screen)
    setcursor(CUR_NORMAL);

  setlogintime();
  Display.CountLines = FALSE;
  Status.UserRecNo   = -1;

  QuickLogon = FALSE;
  NonStop    = FALSE;

  Status.StatLine = BLANKLINE;
  if (Asy.Online == LOCAL && Status.User == SYSOP) {
    asetcolor(Display.DefaultColor);
    printcls();
    Status.UserRecNo     = 1;
    Control.GraphicsMode = PcbData.Graphics;
    Control.RipMode      = FALSE;
    UseAnsi              = TRUE;
    if (PcbData.MultiLingual)
      getdefaultlanguage();
    getuserrecord(FALSE,FALSE);
    UsersData.NumTimesOn++;
    Status.CallerNumber = addcaller();
    writenametolog(LOGON);
    startdisplay(NOCHANGE);
    writelangused();

    #ifdef PCB152
      if (Status.ActStatus != ACT_DISABLED) {
        checkaccountbalance();
        UsersData.Account.StartThisSession = Status.Balance;
      }
    #endif

    checksessionandkbdtime();
    setkbdtimer(PcbData.KbdTimeout);  // reset normal keyboard timer value

    // have to do this before it writes out to pcboard.sys or the pcboard.sys
    // file will show us as being unavailable when we *are* available
    usernetavailable();
    makepcboardsys();
  } else {
    UseAnsi = FALSE;
    Control.GraphicsMode = FALSE;
    Control.RipMode      = FALSE;

    if (PcbData.MultiLingual)
      getdefaultlanguage();

    #ifdef COMM
      checkbaudrate(0);
    #endif

    addtime(5 * 60L,NEWTIME);              /* 5 minutes online time */
    Status.PwrdTimeAllowed = 5;            /* update PWRD for closed board checking */
    setkbdtimer(1);                        // 1 minute keyboard timeout
    renewkbdtimer();

    Status.LoggingIn = LOGINNOW; /* used to indicate not fully logged in */

    #ifdef FIDO
      if (PcbData.EnableFido) {
        Display.Break = FALSE; // FrontDoor sends CTRL-K which flushes the buffer, disable CTRL-K checking
        Status.LoggingIn |= WATCHFORFIDO;
      }
    #endif

    #ifdef COMM
    {
      char Temp[9];

      strcpy(Temp,Status.LogonDate);
      countrydate(Temp);
      sprintf(Str,"CONNECT %ld / %s (%s)",Asy.ConnectSpeed,Temp,Status.LogonTime);
      bell();
      asetcolor(Display.DefaultColor);

      #if defined(CPU386) && defined(MULTIPORT)
        if (Asy.Online == REMOTE) {
          int Len;

          Status.LoggingIn |= WATCHFORMD5;
          ReceivedMd5String[0] = 0;
          sendbyte('\r');
          sprintf(SentMd5String,"<PCB#%d@%s*%s>",PcbData.NodeNum,Status.LogonDate,Status.LogonTime);
          Len = strlen(SentMd5String);
          sendstr(SentMd5String,Len);
          backupdestructive(Len);
//        sendbyte('\r');
        }
      #endif

      if (Display.DefaultColor & 0xF0)
        printcls();
      if (Asy.ConnectString[0] != 0 && strstr(Str,Asy.ConnectString) == NULL)
        println(Asy.ConnectString);
      println(Str);
      if (ISDN)
        Asy.ErrorCorrected = TRUE;
      if (Asy.ErrorCorrected)
        displaypcbtext(TXT_ERRORCORRECTING,NEWLINE);
      newline();
      #ifdef FIDO
        // QUERYFIDO (ask for remote fido to acknowledge its presence)
        if (Asy.Online == REMOTE && PcbData.EnableFido) {
          printcom("**EMSI_REQA77E");
          backupdestructive(14);
        }
      #endif
      println(PcbData.BoardName);
      print(Status.Version);
      print(Status.NodeStr);
      newline();
      checkforrip();
      newline();
    }
    #else
      checkforrip();
    #endif

    if (NoPPE) {
      asetcolor(0x0C);
      ansi("\r\nNOTE TO SYSOP: PPEs are disabled via SET PCB=/NOPPE\r\n");
      asetcolor(Display.DefaultColor);
    }

    checksessionandkbdtime();

    #ifdef FIDO
      checkforfidoresponse();
    #endif

    if (PcbData.MultiLingual) {
      Status.Language = 0;  // force the caller to pick a language
      getlanguage(0);
      newline();
      #ifdef FIDO
        checkforfidoresponse();
      #endif
    }

    if (PcbData.NonGraphics) {
      Control.GraphicsMode = FALSE;
      Control.RipMode      = FALSE;
      setkbdtimer(PcbData.KbdTimeout);  // reset normal keyboard timer value
      turnkbdtimeron();
      checkforansi();
    } else {
      #ifdef COMM
        Answer[0] = 0;
        Mask = mask_graphics;
        #ifdef FIDO
          if (PcbData.EnableFido)
            Mask = mask_alphanum;
        #endif
        inputfield(Answer,TXT_WANTGRAPHICS,sizeof(Answer)-1,YESNO|UPCASE|STACKED|NEWLINE,NOHELP,Mask);

        Control.RipMode      = FALSE;
        Control.GraphicsMode = PcbData.DefaultGraphics;

        #ifdef FIDO
          checkforfidoresponse();
        #endif

        if (substitute(Answer,"NS","",sizeof(Answer)))
          if (! PcbData.DisableQuick)
            NonStop = TRUE;

        if (substitute(Answer,"Q","",sizeof(Answer)))
          QuickLogon = TRUE;

        if (strchr(Answer,NoChar) != NULL)
          Control.GraphicsMode = FALSE;
        else if (strchr(Answer,YesChar) != NULL) {
          UseAnsi = TRUE;
          Control.GraphicsMode = TRUE;
        } else if (strchr(Answer,'R') != NULL) {
          UseAnsi = TRUE;
          HasRip  = TRUE;
          Control.GraphicsMode = TRUE;
          Control.RipMode = TRUE;
        }

        checkforansi();

        if (Control.GraphicsMode) {
          asetcolor(0);               /* force it to start from scratch! */
          if (Display.DefaultColor & 0xF0)
            printcls();
        }

        setkbdtimer(PcbData.KbdTimeout);  // reset normal keyboard timer value
        turnkbdtimeron();

        if (Asy.Online == REMOTE && Asy.DataBits == 7) {
          Control.GraphicsMode = FALSE;
          if (! PcbData.Allow7E1) {
            displaypcbtext(TXT_7E1NOTAVAILABLE,NEWLINE|LFBEFORE);
            loguseroff(ALOGOFF);
            return;
          }
          if (DebugLevel >= 1)
            reportcom(Asy.ModemSpeed);
          setport(bauddivisor(Asy.ModemSpeed),Asy.DataBits);
          displaypcbtext(TXT_7E1INEFFECT,NEWLINE);
        }
      #else
        Control.RipMode = FALSE;
        Control.GraphicsMode = (bool) (Scrn_Mode == VID_COLOR);
        setkbdtimer(PcbData.KbdTimeout);  // reset normal keyboard timer value
        turnkbdtimeron();
      #endif

    }

    if (! QuickLogon)
      displayfile(PcbData.WlcFile,GRAPHICS|LANGUAGE|WELCOME);
    else
      newline();

    getloginname();
    writelangused();

    Status.UseAlias = (bool) (UsersData.Alias[0] != 0);

    #ifdef FIDO
      if (PcbData.EnableFido)
        Display.Break = TRUE;     // disabled up above
    #endif

    if (! Status.MultipleLogins) {
      if (foundinusernet(UsersData.Name,UsersData.Alias)) {
        Status.UserRecNo = 0;  // change back to 0 to avoid writing out user data
        displaypcbtext(TXT_NAMEALREADYINUSE,NEWLINE|LFBEFORE|LOGIT);
        loguseroff(ALOGOFF);
        return;
      }

      if (PasswordSupport && UsersData.PwrdHistory.ExpireDate != 0) {
        if (Status.JulianLogonDate >= UsersData.PwrdHistory.ExpireDate) {
          displaypcbtext(TXT_PASSWORDEXPIRED,NEWLINE|LFBEFORE);
          getnewpassword(&UsersData,TRUE);
        } else if (Status.JulianLogonDate >= UsersData.PwrdHistory.ExpireDate-PcbData.PwrdWarn) {
          ascii(Status.DisplayText,UsersData.PwrdHistory.ExpireDate - Status.JulianLogonDate);
          displaypcbtext(TXT_PASSWORDWILLEXP,NEWLINE|LFBEFORE|BELL);
        }
      }
    }

    Status.CallerNumber = addcaller();

    #ifdef PCB152
      if (Status.ActStatus != ACT_DISABLED) {
        checkaccountbalance();
        UsersData.Account.StartThisSession = Status.Balance;
      }
    #endif

    if (StatsSupport) {
      if (UsersData.Stats.FirstDateOn == 0)
        UsersData.Stats.FirstDateOn = Status.JulianLogonDate;
      #ifdef COMM
        if (Asy.Online == REMOTE) {
          switch((unsigned) (Asy.CarrierSpeed/10)) {
            case   30: UsersData.Stats.Num300++;    break;
            case  120: UsersData.Stats.Num1200++;   break;
            case  240: UsersData.Stats.Num2400++;   break;
            case  480:
            case  720:
            case  960: UsersData.Stats.Num9600++;   break;
            default  : if (Asy.CarrierSpeed > 9600)
                          UsersData.Stats.Num14400++;
                       break;
          }
        }
      #endif
    }

    // have to do this before it writes out to pcboard.sys or the pcboard.sys
    // file will show us as being unavailable when we *are* available
    usernetavailable();

    makepcboardsys();
    logonscript();
  }

  #ifdef COMM
    logconnectstring();
  #endif

  if (PcbData.LogCallerNumber) {
    if (Asy.Online == REMOTE || ! PcbData.ExcludeLocals) {
      comma(Status.DisplayText,Status.CallerNumber);
      logsystext(TXT_CALLERNUM,SPACERIGHT);
    }
  }

  if (PcbData.LogSecLevel) {
    if (Asy.Online == REMOTE || ! PcbData.ExcludeLocals) {
      ascii(Status.DisplayText,Status.CurSecLevel);
      logsystext(TXT_CALLERSEC,SPACERIGHT);
    }
  }

  if (Asy.Online == LOCAL)
    turnonscrollback();

  checkforlogonbat();  /* check for pre-processing to perform */

  startdisplay(NOCHANGE);
  if (PcbData.SubscriptMode && UsersData.RegExpDate != 0) {
    if (UsersData.RegExpDate < Status.JulianLogonDate)
      displayfile(PcbData.ExpFile,GRAPHICS|SECURITY|LANGUAGE);
    else if (UsersData.RegExpDate < Status.JulianLogonDate+PcbData.SubscriptWarning)
      displayfile(PcbData.WrnFile,GRAPHICS|SECURITY|LANGUAGE);
  }

  #ifdef PCB152
    if (Status.ActStatus != ACT_DISABLED) {
      if (Asy.Online == LOCAL)
        strcpy(Str,"LOCAL");
      else
        lascii(Str,Asy.CarrierSpeed);

      recordusage("LOGON",Str,AccountRates.ChargeForCall,1,&UsersData.Account.DebitCall);

      if (fileexist(PcbData.AccountInfo) != 255)
        displayfile(PcbData.AccountInfo,GRAPHICS|SECURITY|LANGUAGE);
      if (Status.ActStatus == ACT_ENFORCE) {
        if (Status.Balance <= AccountRates.WarnLevel && fileexist(PcbData.AccountWarn) != 255)
          displayfile(PcbData.AccountWarn,GRAPHICS|SECURITY|LANGUAGE);
      }
    }
  #endif

  strcpy(Str,PcbData.SecLoc);
  ascii(&Str[strlen(Str)],Status.CurSecLevel);
  if (fileexist(Str) != 255)
    displayfile(Str,GRAPHICS|LANGUAGE);

  if (Status.TimeAdjustedForEvent) {
    displaypcbtext(TXT_TIMEADJUSTED,NEWLINE|LFBEFORE);
    // the INTRO screen might wipe the time adjusted prompt off the screen
    // before the caller can see it.
    moreprompt(PRESSENTER);
  }

  scanmailflags();

  #ifdef PCB152
    Status.ConfJoinTime = dosgetlongtime();
  #endif

  Status.Conference = 0;
  showconfjoined(UsersData.LastConference,&Status.CurConf,FALSE,(bool) (! NonStop));
  processjoin((bool) (! NonStop),QuickLogon,TRUE);

  if (Status.AllowAlias && UsersData.Alias[0] != 0 &&
      Status.CurConf.AllowAliases && Status.UseAlias) {
    strcpy(Status.DisplayText,Status.DisplayName);
    logsystext(TXT_CHANGEDNAMETO,SPACERIGHT);
  }

  #ifndef PCB_DEMO
    logindoor();       /* execute the login door if there is one */
  #endif
  makepcboardsys();
}
