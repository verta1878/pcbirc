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

bool static ModifySelf = FALSE;

static void _NEAR_ LIBENTRY getfield(int TextNum, char *Field, int FieldLen, DISPLAYTYPE DispCtrl, char *Mask) {
  pcbtexttype Buf;

  getpcbtext(TextNum,&Buf);
  if (Buf.Str[0] == 0)
    return;

  inputfield(Field,TextNum,FieldLen-1,DispCtrl|FIELDLEN|GUIDE|NEWLINE|LFAFTER,NOHELP,Mask);
}


/********************************************************************
*
*  Function:  checkpassword()
*
*  Desc    :  Checks the password passed to it to see if it meets the
*             Password PSA criteria (of the PSA is installed).  Also checks
*             to see if the new password is identical to the old password.
*
*             This function is designed to be called locally within this
*             module and purposely AVOIDS talking back to the caller.  This is
*             because the same module is also accessed through PPL as well as
*             through the various interactive password changing functions.
*
*  Returns :  1 = New Password identical to Old Password
*             0 = New Password is 100% okay
*            -1 = New Password is too short
*            -2 = New Password is part of caller's name
*            -3 = New Password has already been used and is in the Pwrd History
*/

static int _NEAR_ LIBENTRY checkpassword(UData *pud, char *Str) {
  int       NumTokens;
  unsigned  MinPwrdLen;
  char     *p;
  char      OldPwrd[sizeof(UsersData.Password)];
  char      Temp[sizeof(UsersData.Password)];
  char      Name[26];

  strcpy(OldPwrd,pud->Password);
  stripright(Str,' ');

  if (PasswordSupport) {
    MinPwrdLen = PcbData.MinPwrdLen;
    if (MinPwrdLen > 12)
      MinPwrdLen = 12;

    // make sure that the password is at least PcbData.MinPwrdLen chars long
    if (strlen(Str) < MinPwrdLen)
      return(-1);

    // make sure that the password is not a subset of the person's name
    strcpy(Name,pud->Name);
    if (strstr(Name,Str) != NULL)
      return(-2);

    // make sure that no subset of the person's name is part of the password
    for (NumTokens = tokenize(Name); NumTokens; NumTokens--) {
      p = getnexttoken();
      if (strstr(p,Str) != NULL || strstr(Str,p) != NULL)
        return(-2);
    }

    // make sure that an exact match is not made on any previously used
    // passwords
    strcpy(Temp,Str);
    padstr(Temp,' ',12);
    if (strcmp(Temp,pud->PwrdHistory.Previous[0]) == 0 ||
        strcmp(Temp,pud->PwrdHistory.Previous[1]) == 0 ||
        strcmp(Temp,pud->PwrdHistory.Previous[2]) == 0)
      return(-3);

    // if the MinPwrdLen is greater than 4, then don't allow the first
    // MinPwrdLen-2 characters to match a previously used password
    if (MinPwrdLen > 4) {
      MinPwrdLen -= 2;
      if (memcmp(Temp,pud->PwrdHistory.Previous[0],MinPwrdLen) == 0 ||
          memcmp(Temp,pud->PwrdHistory.Previous[1],MinPwrdLen) == 0 ||
          memcmp(Temp,pud->PwrdHistory.Previous[2],MinPwrdLen) == 0)
        return(-3);
      // if the new password is not an exact match of the old password,
      // then also check the first MinPwrdLen-2 characters of the new
      // password to ensure that it is not a match
      if (strcmp(Str,OldPwrd) != 0 && memcmp(Temp,OldPwrd,MinPwrdLen) == 0)
        return(-3);
    }
  }

  // if the new password is identical to the old password then let the
  // function caller know about it
  if (strcmp(Str,OldPwrd) == 0)
    return(1);

  return(0);
}


/********************************************************************
*
*  Function:  badpassword()
*
*  Desc    :  This function checks the password (Str) to see if it is a bad
*             password and, if so, it informs the caller.
*
*             If this function is called from a routine that is trying to
*             force the caller to change passwords then when an OLD=NEW match
*             is found, it will return an error condition.  Otherwise, it will
*             accept it so that the password goes unchanged.
*
*  Returns :  TRUE if the password did not meet passing criteria.
*             FALSE if it is okay to use the new password.
*/

bool LIBENTRY badpassword(UData *pud, char *Str, bool ForceNewPassword) {
  if (PasswordSupport) {
    switch (checkpassword(pud,Str)) {
      case  1: if (ForceNewPassword) {
                 displaypcbtext(TXT_PREVIOUSLYUSEDPWRD,NEWLINE|LOGIT);
                 return(TRUE);
               }
               break;
      case -1: ascii(Status.DisplayText,PcbData.MinPwrdLen);
               displaypcbtext(TXT_PASSWORDTOOSHORT,NEWLINE);
               return(TRUE);
      case -2: displaypcbtext(TXT_NEEDUNIQUEPASSWORD,NEWLINE);
               return(TRUE);
      case -3: displaypcbtext(TXT_PREVIOUSLYUSEDPWRD,NEWLINE|LOGIT);
               return(TRUE);
    }
  }
  return(FALSE);
}


/********************************************************************
*
*  Function:  updatepassword()
*
*  Desc    :  This function assumes that the password information has already
*             been validated and simply updates the password AND the history
*             information if the Password PSA is installed.
*/

static void _NEAR_ LIBENTRY updatepassword(UData *pud, char *Str) {
  if (PasswordSupport) {
    if (pud->PwrdHistory.LastChange != getjuliandate()) {
      pud->PwrdHistory.LastChange = getjuliandate();
      pud->PwrdHistory.TimesChanged++;
      strcpy(pud->PwrdHistory.Previous[2],pud->PwrdHistory.Previous[1]);
      strcpy(pud->PwrdHistory.Previous[1],pud->PwrdHistory.Previous[0]);
      strcpy(pud->PwrdHistory.Previous[0],pud->Password);
      if (PcbData.PwrdUpdate == 0)
        pud->PwrdHistory.ExpireDate = 0;
      else if ((Status.JulianLogonDate + PcbData.PwrdUpdate) > pud->PwrdHistory.ExpireDate)
        pud->PwrdHistory.ExpireDate = (unsigned short) (Status.JulianLogonDate + PcbData.PwrdUpdate);
      convertpwrdztopwrd(&pud->PwrdHistory,&PwrdRec);
    }
  }
  strcpy(pud->Password,Str);
}


/********************************************************************
*
*  Function:  setnewpassword()
*
*  Desc    :  This function is called by PPL and purposely avoids talking to
*             the user.  It checks the password to see if it is okay and, if
*             so, it then updates the password.  If the password does not
*             pass the criteria in checkpassword() then an error is returned
*             without updating the caller's password.
*
*  Returns :  The return values are identical to checkpassword() as follows:
*
*      1 = New Password identical to Old Password (not necessarily an error)
*      0 = New Password is 100% okay
*     -1 = New Password is too short
*     -2 = New Password is part of caller's name
*     -3 = New Password has already been used and is in the Pwrd History
*/

int LIBENTRY setnewpassword(UData *pud, char *NewPwrd) {
  int RetVal;

  if ((RetVal = checkpassword(pud,NewPwrd)) != 0)
    return(RetVal);

  updatepassword(pud,NewPwrd);

  if (pud == &UsersData) {
    convertdatatoread(&UsersData,&UsersRead);
    putuserrecord(IGNOREDIRTY);
    getuserrecord(FALSE,FALSE);
  } else {
    convertdatatoread(TempData,TempRead);
  }
  return(0);
}


/********************************************************************
*
*  Function:  getnewpassword()
*
*  Desc    :  Interactively asks the caller for a password.  Verifies that the
*             password is good.  Asks the caller to re-enter for verification.
*             And stores the new password.
*/

void LIBENTRY getnewpassword(UData *pud, bool ForceNewPassword) {
  int  NumTries;
  char Pwrd[sizeof(UsersData.Password)];
  char OldPwrd[sizeof(UsersData.Password)];
  char Str[100];

  NumTries = 0;
  strcpy(OldPwrd,pud->Password);

  while (1) {
    Str[0] = 0;
    inputfield(Str,TXT_NEWPASSWORD,sizeof(UsersRead.Password),ECHODOTS|FIELDLEN|UPCASE|GUIDE|NEWLINE|LFBEFORE|LFAFTER,NOHELP,(PcbData.DisableEdits ? mask_message : mask_alphanum));
    if (Str[0] == 0)
      if (ForceNewPassword) {
        NumTries++;
        if (NumTries > 5)
          loguseroff(ALOGOFF);
        continue;
      } else
        return;
    else {
      if (PasswordSupport && pud == &UsersData) {
        if (badpassword(pud,Str,ForceNewPassword))
          continue;
      }

      Pwrd[0] = 0;
      inputfield(Pwrd,TXT_REENTERPASSWORD,sizeof(UsersRead.Password),ECHODOTS|FIELDLEN|UPCASE|GUIDE|NEWLINE|LFAFTER,NOHELP,(PcbData.DisableEdits ? mask_message : mask_alphanum));
      padstr(Pwrd,' ',12);
      padstr(Str,' ',12);
      if (strcmp(Str,Pwrd) != 0) {
        displaypcbtext(TXT_PWRDSDONTMATCH,NEWLINE);
        continue;
      }

      if (strcmp(Str,OldPwrd) != 0)
        updatepassword(pud,Str);
      return;
    }
  }
}


/********************************************************************
*
*  Function:  checkalias()
*
*  Desc    :  Checks the alias passed to it to see if it meets the criteria
*             for a good alias ... such as not having @-codes in it, and not
*             duplicating an existing name.
*
*             This function is designed to be called locally within this
*             module and purposely AVOIDS talking back to the caller.  This is
*             because the same module is also accessed through PPL as well as
*             through the various interactive password changing functions.
*
*  Returns :  1 = New Alias identical to Old Alias
*             0 = New Alias is 100% okay
*            -1 = New Alias consisted of all @-codes, stripped down to nothing
*            -2 = New Alias was found in the TCAN file
*            -3 = New Alias duplicated an existing name or alias
*/

static int _NEAR_ LIBENTRY checkalias(UData *pud, char *Str) {
  char Temp[26];

  if (strcmp(Str,pud->Alias) == 0)
    return(1);

  if (Str[0] != 0) {
    strcpy(Temp,Str);
    removetokens(Temp);
    stripright(Temp,' ');
    if (Temp[0] == 0)
      return(-1);

    if (readtcanfile(Str))
      return(-2);

    if (strcmp(Str,pud->Name) == 0)
      return(-3);

    padstr(Str,' ',25);
    if (finduser(Str) != -1)
      return(-3);
  }
  return(0);
}


/********************************************************************
*
*  Function:  updatealias()
*
*  Desc    :  This function assumes that the alias information has already
*             been validated and simply updates the alias information, both
*             in the users file and in the index files (removing old alias
*             information if necessary).
*/

static void _NEAR_ LIBENTRY updatealias(UData *pud, char *Str, long UserRecNo) {
  char Temp[sizeof(ReadAlias)+1];

  if (pud->Alias[0] != 0) {
    // user had an alias, remove it from the index now
    removeuser(pud->Alias);
  }

  if (Str[0] != 0) {
    // user has selected a new alias, set it up and add it
    // to the index files.
    maxstrcpy(Status.DisplayText,Str,26);
    maxstrcpy(pud->Alias,Str,26);
    stripright(pud->Alias,' ');
    strcpy(Temp,Str);
    padstr(Temp,' ',25);
    addusertoindex(Temp,UserRecNo);    //lint !e534
  } else {
    // answer is blank, so blank out the alias information
    Status.DisplayText[0] = UsersData.Alias[0] = 0;
    padstr(Str,' ',25);
    pud->Alias[0] = 0;
  }

  logsystext(TXT_CHANGEDALIASTO,SPACERIGHT);
  if (Status.CurConf.AllowAliases) {
    getdisplaynames();
    usernetavailable();
  }
}


/********************************************************************
*
*  Function:  int LIBENTRY setnewalias(char *Alias);
*
*  Desc    :  This function is called by PPL and purposely avoids talking to
*             the user.  It checks the alias to see if it is okay and, if
*             so, it then updates the alias information both in the users file
*             and in the index files, including removing the old alias.  If
*             the new alias does not pass the criteria in checkalias() then
*             an error is returned without updating the caller's alias.
*
*  Returns :  The return values are identical to checkalias() as follows:
*
*             1 = New Alias identical to Old Alias
*             0 = New Alias is 100% okay
*            -1 = New Alias consisted of all @-codes, stripped down to nothing
*            -2 = New Alias was found in the TCAN file
*            -3 = New Alias duplicated an existing name or alias
*/

int LIBENTRY setnewalias(UData *pud, char *NewAlias, long UserRecNo) {
  int  RetVal;
  char Str[26];

  if (AliasSupport) {
    maxstrcpy(Str,NewAlias,sizeof(Str));
    stripright(Str,' ');

    if ((RetVal = checkalias(pud,Str)) != 0)
      return(RetVal);

    updatealias(pud,Str,UserRecNo);
  }
  return(0);
}


/********************************************************************
*
*  Function:  getalias()
*
*  Desc    :  Interactively asks the caller for an alias.  Verifies that the
*             alias is good. Warns caller if duplicate or bad alias.
*             And stores the new password if it is good.
*/

void LIBENTRY getalias(UData *pud, long UserRecNo) {
  char Answer[26];

  if (AliasSupport && Status.AllowAlias) {
    if (pud->Alias[0] == 0 || PcbData.AllowAliasChange) {
      memset(Answer,0,26);
      while (1) {
        strcpy(Answer,pud->Alias);
        getfield(TXT_GETALIASNAME,Answer,25+1,HIGHASCII|UPCASE,mask_alphanum);
        Answer[25] = 0;
        stripright(Answer,' ');
        stripleft(Answer,' ');
        stripall(Answer,'\xFF');

        switch (checkalias(pud,Answer)) {
          case -1: // drop to bottom of loop and log attempt.
                   break;
          case -2: stripright(Answer,' ');
                   strcpy(Status.DisplayText,Answer);
                   displaypcbtext(TXT_INVALIDENTRY,NEWLINE|LFBEFORE);
                   break;
          case -3: displaypcbtext(TXT_USEREXISTS,NEWLINE);
                   break;
          case  0: updatealias(pud,Answer,UserRecNo);
                   return;
          case  1: // no change was made, just exit out now
                   return;
        }

        stripright(Answer,' ');
        strcpy(Status.DisplayText,Answer);
        logsystext(TXT_ATTEMPTEDALIAS,SPACERIGHT);
        // loop back around and try again
      }
    }
  }
}


static void _NEAR_ LIBENTRY askcommonquestions(UData *pud, bool AskAll) {
  displaytype Ctrl;

  getnewpassword(pud,FALSE);

  if (AskAll) {
    Ctrl = (MixedCase ? HIGHASCII : HIGHASCII|UPCASE);
    getfield(TXT_CITYSTATE         ,pud->City          ,sizeof(pud->City)          ,Ctrl            ,mask_alphanum);
    getfield(TXT_BUSDATAPHONE      ,pud->BusDataPhone  ,sizeof(pud->BusDataPhone)  ,HIGHASCII|UPCASE,(PcbData.DisableEdits ? mask_alphanum : (ModifySelf ? mask_phone1 : mask_phone2)));
    getfield(TXT_HOMEVOICEPHONE    ,pud->HomeVoicePhone,sizeof(pud->HomeVoicePhone),HIGHASCII|UPCASE,(PcbData.DisableEdits ? mask_alphanum : (ModifySelf ? mask_phone1 : mask_phone2)));
    getfield(TXT_COMMENTFIELDPROMPT,pud->UserComment   ,sizeof(pud->UserComment)   ,Ctrl            ,mask_alphanum);
  }
}


static int _NEAR_ LIBENTRY askquestions(UData *pud, char _FAR_ *Conf, long _FAR_ *MsgPtr, bool AskName, long UserRecNo) {
  int         Len;
  char        Level;
  pcbtexttype Buf;
  char        Str[100];

  newline();
  Level = UsersData.SecurityLevel;  /* the level of the user doing the upgade */

  if (AskName) {
    Str[0] = 0;
    inputfield(Str,TXT_USERSFIRSTNAME,sizeof(pud->Name)-1,FIELDLEN|UPCASE|HIGHASCII|NEWLINE|LFAFTER,NOHELP,mask_alphanum);
    if (Str[0] == 0)
      return(-1);

    if (strchr(Str,' ') == NULL) {
      Len = strlen(Str);
      Str[Len++] = ' ';
      Str[Len]   = 0;
      inputfield(&Str[Len],TXT_USERSLASTNAME,(sizeof(pud->Name)-1)-Len,FIELDLEN|UPCASE|HIGHASCII|NEWLINE|LFAFTER,NOHELP,mask_alphanum);
    }
    if (finduser(Str) != -1) {
      displaypcbtext(TXT_USEREXISTS,NEWLINE|LFAFTER);
      return(-1);
    }
    strcpy(pud->Name,Str);
  }

  askcommonquestions(pud,TRUE);
  getfield(TXT_SPECIALCOMMENT,pud->SysopComment,sizeof(pud->SysopComment),HIGHASCII,mask_alphanum);

  ascii(Str,pud->SecurityLevel);    getfield(TXT_ENTERUSERSECLEVEL,Str,3+1,DEFAULTS,mask_numbers); pud->SecurityLevel    = (char) atoi(Str);
  ascii(Str,pud->ExpSecurityLevel); getfield(TXT_EXPSECLEVEL      ,Str,3+1,DEFAULTS,mask_numbers); pud->ExpSecurityLevel = (char) atoi(Str);

  if (! Status.OnlineUpdate) {
    if (pud->SecurityLevel > Level)
      pud->SecurityLevel = Level;
    if (pud->ExpSecurityLevel > Level)
      pud->ExpSecurityLevel = Level;
  }

  if (PcbData.SubscriptMode) {
    strcpy(Str,juliantodate(pud->RegExpDate));
    stripall(Str,'-');
    countrydate2(Str);
    getfield(TXT_EXPDATE,Str,6+1,DEFAULTS,mask_numbers);
    if (atoi(Str) == 0)
      memset(Str,'0',6);
    uncountrydate2(Str);
    pud->RegExpDate = datetojulian(Str);
  }

  getalias(pud,UserRecNo);

  getpcbtext(TXT_SELECTCONFS,&Buf);
  if (Buf.Str[0] != 0) {
    Str[0] = NoChar;
    Str[1] = 0;
    inputfield(Str,TXT_SELECTCONFS,1,YESNO|NEWLINE|UPCASE|FIELDLEN,NOHELP,mask_yesno);
    if (Str[0] == YesChar)
      chooseconferences(0,Conf,MsgPtr,ALLCONF);
  }

  return(0);
}

#ifdef OLDMETHOD
static long _NEAR_ LIBENTRY adduser(void) {
  unsigned    X;
  long        UserRecNo;
  char        yymmdd[6];
  pcbconftype Conf;

  strtoyymmdd(yymmdd,Status.LogonDate);
  memset(TempRead,' ',sizeof(URead));
  memset(&TempInfRec,0,sizeof(rectype));

  TempRead->SecurityLevel    = PcbData.UserLevels[SEC_REG];
  TempRead->ExpSecurityLevel = PcbData.UserLevels[SEC_REG];
  TempRead->ExpertMode       = 'N';
  TempRead->Protocol         = 'N';
  TempRead->DeleteFlag       = 'N';
  TempRead->PageLen          = 23;
  TempRead->NumTimesOn       = 0;
  TempRead->NumUploads       = 0;
  TempRead->NumDownloads     = 0;
  TempRead->ElapsedTimeOn    = 0;
  TempRead->LastConference   = 0;

  memcpy(TempRead->LastTimeOn,"00:00",sizeof(TempRead->LastTimeOn));
  memcpy(TempRead->LastDateOn,yymmdd,sizeof(TempRead->LastDateOn));
  memcpy(TempRead->DateLastDirRead,yymmdd,sizeof(TempRead->DateLastDirRead));

  if (PcbData.SubscriptMode)
    strtoyymmdd(TempRead->RegExpDate,juliantodate(Status.JulianLogonDate+PcbData.SubscriptionDays));
  else
    memset(TempRead->RegExpDate,'0',sizeof(TempRead->RegExpDate));

  longtobasdble(TempRead->DailyDnldBytes,0);
  longtobasdble(TempRead->TotDnldBytes  ,0);
  longtobasdble(TempRead->TotUpldBytes  ,0);

  fmemset(TempMsgPtr,0,MsgPtrLen);
  fmemset(TempReg,0,TOTCONFREGFLAGS);

  /* auto-register the caller in all PUBLIC conferences that have NO */
  /* SECURITY requirement unless the sysop sets AutoRegConf to FALSE */

  if (PcbData.AutoRegConf) {
    for (X = 0; X < PcbData.NumAreas; X++) {
      getconfrecord(X,&Conf);
      if (Conf.PublicConf && Conf.ReqSecLevel <= 0) {
        setbit(&TempReg[REG],X);
        setbit(&TempReg[EXP],X);
        setbit(&TempReg[USR],X);
        setbit(&TempReg[CUR],X);
      }
    }
  }

  convertreadtodata(TempRead,TempData);
  if (askquestions(TempData,TempReg,TempMsgPtr,TRUE,0) == -1)
    return(-1);

  convertdatatoread(TempData,TempRead);
  UserRecNo = adduserrecord(TempRead,&TempInfRec,TempReg,TempMsgPtr);

  // Up above the call to getalias() added the alias name to the index.
  // However, since the user was new, it added the alias with a record number
  // of 0.  This will not work correctly, so we must now REMOVE the alias
  // from the index file and then put it back in with the proper record
  // number.  Kind of a kludge, but previously we were not preventing
  // complete logins for users who answered only some of the questions.
  // Now if not all of the questions are answered (carrier is lost or the
  // user is forced off) it will not leave partial records in the users file.

  if (UserRecNo != 0 && TempData->Alias[0] != 0) {
    removeuser(TempData->Alias);
    moveback(ReadAlias,TempData->Alias,25);
    addusertoindex(ReadAlias,UserRecNo);
  }

  return(UserRecNo);
}
#endif


static void _NEAR_ LIBENTRY addrecsub(void) {
  unsigned    X;
  pcbconftype Conf;

  memset(TempData,0,sizeof(UData));

  TempData->SecurityLevel    = PcbData.UserLevels[SEC_REG];
  TempData->ExpSecurityLevel = PcbData.DefExpiredLevel;
  TempData->Protocol         = 'N';
  TempData->PageLen          = 23;
  TempData->LastDateOn       = Status.JulianLogonDate;
  TempData->DateLastDirRead  = (unsigned short) ctod(Status.LogonDate);

  memcpy(TempData->LastTimeOn,"00:00",sizeof(TempData->LastTimeOn));

  if (PcbData.SubscriptMode)
    TempData->RegExpDate = (unsigned short) (Status.JulianLogonDate+PcbData.SubscriptionDays);

  fmemset(TempMsgPtr,0,MsgPtrLen);
  fmemset(TempReg,0,TOTCONFREGFLAGS);

  /* auto-register the caller in all PUBLIC conferences that have NO */
  /* SECURITY requirement unless the sysop sets AutoRegConf to FALSE */

  if (PcbData.AutoRegConf) {
    for (X = 0; X < PcbData.NumAreas; X++) {
      getconfrecord(X,&Conf);
      if (Conf.PublicConf && Conf.ReqSecLevel <= 0) {
        setbit(&TempReg[REG],X);
        setbit(&TempReg[EXP],X);
        setbit(&TempReg[USR],X);
        setbit(&TempReg[CUR],X);
      }
    }
  }
}


long LIBENTRY ppladduser(char *Name) {
  long RecNum;

  if ((RecNum = finduser(Name)) != -1)
    return(-1);

  addrecsub();
  maxstrcpy(TempData->Name,Name,sizeof(TempData->Name));
  convertdatatoread(TempData,TempRead);
  if ((RecNum = adduserrecord(TempRead,&TempInfRec,TempReg,TempMsgPtr)) == 0)
    return(-1);

  return(RecNum);
}


static long _NEAR_ LIBENTRY adduser(void) {
  long        UserRecNo;

  addrecsub();

  if (askquestions(TempData,TempReg,TempMsgPtr,TRUE,0) == -1)
    return(-1);

  convertdatatoread(TempData,TempRead);
  UserRecNo = adduserrecord(TempRead,&TempInfRec,TempReg,TempMsgPtr);

  // Up above the call to getalias() added the alias name to the index.
  // However, since the user was new, it added the alias with a record number
  // of 0.  This will not work correctly, so we must now REMOVE the alias
  // from the index file and then put it back in with the proper record
  // number.  Kind of a kludge, but previously we were not preventing
  // complete logins for users who answered only some of the questions.
  // Now if not all of the questions are answered (carrier is lost or the
  // user is forced off) it will not leave partial records in the users file.

  if (UserRecNo != 0 && TempData->Alias[0] != 0) {
    removeuser(TempData->Alias);
    moveback(ReadAlias,TempData->Alias,25);
    addusertoindex(ReadAlias,UserRecNo);    //lint !e534
  }

  return(UserRecNo);
}


static long _NEAR_ LIBENTRY findusername(int NumTokens, long RecNum) {
  char  *p;
  long  Num;
  char  Str[sizeof(UsersData.Name)];

  Str[0] = 0;
  if (NumTokens) {
    for (; NumTokens; NumTokens--) {
      p = getnexttoken();
      if (strlen(Str)+strlen(p)+1 < sizeof(Str)-1) {
        strcat(Str,p);
        addchar(Str,' ');
      }
    }
  } else {
    inputfield(Str,TXT_FULLUSERNAMETOFIND,sizeof(UsersRead.Name),FIELDLEN|UPCASE|HIGHASCII|NEWLINE|LFBEFORE,NOHELP,mask_alphanum);
    if (Str[0] == 0)
      return(RecNum);
  }

  if ((Num = finduser(Str)) != -1)
    return(Num);

  stripright(Str,' ');
  strcpy(Status.DisplayText,Str);
  displaypcbtext(TXT_COULDNTFINDINUSERS,NEWLINE);
  return(-1);
}


void LIBENTRY getaddress(void) {
  pcbtexttype Buf;

  getpcbtext(TXT_ENTERADDRESS,&Buf);
  if (AddressSupport && Buf.Str[0] != 0) {
    displaypcbtext(TXT_ENTERADDRESS,NEWLINE|LFAFTER|LFBEFORE);
    getfield(TXT_STREET1,UsersData.Address.Street[0],50+1,HIGHASCII,mask_alphanum);
    getfield(TXT_STREET2,UsersData.Address.Street[1],50+1,HIGHASCII,mask_alphanum);
    getfield(TXT_CITY   ,UsersData.Address.City     ,25+1,HIGHASCII,mask_alphanum);
    getfield(TXT_STATE  ,UsersData.Address.State    ,10+1,HIGHASCII|UPCASE,mask_alphanum);
    getfield(TXT_ZIP    ,UsersData.Address.Zip      ,10+1,HIGHASCII|UPCASE,mask_alphanum);
    getfield(TXT_COUNTRY,UsersData.Address.Country  ,15+1,HIGHASCII|UPCASE,mask_alphanum);
    convertaddrztoaddr(&UsersData.Address,&AddrRec);
  }
}


#ifdef PCB152
static void LIBENTRY getqwklimits(void) {
  char        Str[12];
  pcbtexttype Buf;

  if (QwkSupport) {
    newline();
    getpcbtext(TXT_PERSONALMSGLIMIT,&Buf);
    if (Buf.Str[0] != 0) {
      lascii(Str,UsersData.QwkConfig.MaxMsgs);
      lascii(Status.DisplayText,PcbData.MaxTotalMsgs);
      getfield(TXT_PERSONALMSGLIMIT,Str,6,DEFAULTS,mask_numbers);
      UsersData.QwkConfig.MaxMsgs = (unsigned short) atoi(Str);
    }
    getpcbtext(TXT_PERSONALCONFLIMIT,&Buf);
    if (Buf.Str[0] != 0) {
      lascii(Str,UsersData.QwkConfig.MaxMsgsPerConf);
      lascii(Status.DisplayText,PcbData.MaxConfMsgs);
      getfield(TXT_PERSONALCONFLIMIT,Str,6,DEFAULTS,mask_numbers);
      UsersData.QwkConfig.MaxMsgsPerConf = (unsigned short) atoi(Str);
    }
    getpcbtext(TXT_PERSONALQWKLIMIT,&Buf);
    if (Buf.Str[0] != 0) {
      lascii(Str,UsersData.QwkConfig.PersonalAttachLimit);
      getfield(TXT_PERSONALQWKLIMIT,Str,10,DEFAULTS,mask_numbers);
      UsersData.QwkConfig.PersonalAttachLimit = atol(Str);
    }
    getpcbtext(TXT_PUBLICQWKLIMIT,&Buf);
    if (Buf.Str[0] != 0) {
      lascii(Str,UsersData.QwkConfig.PublicAttachLimit);
      getfield(TXT_PUBLICQWKLIMIT,Str,10,DEFAULTS,mask_numbers);
      UsersData.QwkConfig.PublicAttachLimit = atol(Str);
    }
  }
}
#endif


#pragma warn -par
void LIBENTRY modifyself(int NumTokens) {
  char Answer[26];
  pcbtexttype Buf;

  ModifySelf = TRUE;
  displaypcbtext(TXT_ENTERNOCHANGE,NEWLINE|LFBEFORE);
  UsersData.LastConference = Status.Conference;  /* remember where we are! */
  askcommonquestions(&UsersData,(bool) (! PcbData.AllowPwrdOnly));

  #ifdef PCBCOMM2
    if (Status.TerseMode)     /* if we're in terse mode then the Msg Clear */
      goto skip;              /* FSE, and Conference settings don't matter */
  #endif

  Answer[0] = (UsersData.PackedFlags.MsgClear ? YesChar : NoChar);
  Answer[1] = 0;
  getfield(TXT_CLSBETWEENMSGS,Answer,1+1,YESNO|UPCASE,mask_yesno);
  UsersData.PackedFlags.MsgClear = (Answer[0] == YesChar);

  Answer[0] = (UsersData.PackedFlags.ScrollMsgBody ? YesChar : NoChar);
  Answer[1] = 0;
  getfield(TXT_SCROLLMSGBODY,Answer,1+1,YESNO|UPCASE,mask_yesno);
  UsersData.PackedFlags.ScrollMsgBody = (Answer[0] == YesChar);

  Answer[0] = (UsersData.PackedFlags.ShortHeader ? NoChar : YesChar);
  Answer[1] = 0;
  getfield(TXT_USEBIGHEADERS,Answer,1+1,YESNO|UPCASE,mask_yesno);
  UsersData.PackedFlags.ShortHeader = (Answer[0] != YesChar);

  Answer[0] = (UsersData.PackedFlags.DontAskFSE ? (UsersData.PackedFlags.FSEDefault ? YesChar : NoChar) : 'A');
  Answer[1] = 0;
  inputfield(Answer,TXT_SETFSEDEFAULT,1,YESNO|NEWLINE|LFAFTER|UPCASE|FIELDLEN|GUIDE,NOHELP,mask_any);
  if (Answer[0] == 'A')
    UsersData.PackedFlags.DontAskFSE = FALSE;
  else {
    UsersData.PackedFlags.DontAskFSE = TRUE;
    UsersData.PackedFlags.FSEDefault = (Answer[0] == YesChar);
  }

  Answer[0] = (UsersData.PackedFlags.WideEditor ? YesChar : NoChar);
  Answer[1] = 0;
  getfield(TXT_DEFAULTWIDEMSGS,Answer,1+1,YESNO|UPCASE,mask_yesno);
  UsersData.PackedFlags.WideEditor = (Answer[0] == YesChar);

  if (! PcbData.AllowPwrdOnly)
    getalias(&UsersData,Status.UserRecNo);

  Answer[0] = (UsersData.Flags.SingleLines ? YesChar : NoChar);
  Answer[1] = 0;
  getfield(TXT_USESHORTDESC,Answer,2,YESNO|UPCASE,mask_yesno);
  UsersData.Flags.SingleLines = (Answer[0] == YesChar);

  if (Status.CurSecLevel >= PcbData.UserLevels[SEC_J]) {
    getpcbtext(TXT_SELECTCONFS,&Buf);
    if (Buf.Str[0] != 0) {
      Answer[0] = NoChar;
      Answer[1] = 0;
      inputfield(Answer,TXT_SELECTCONFS,1,YESNO|NEWLINE|UPCASE|FIELDLEN|GUIDE,NOHELP,mask_yesno);
      if (Answer[0] == YesChar)
        chooseconferences(0,ConfReg,MsgReadPtr,SCANONLY);
    }
  }

  if (! PcbData.AllowPwrdOnly)
    getaddress();

  getqwklimits();

#ifdef PCBCOMM2
skip:
#endif

  if (Status.UserRecNo != 0) {
    displaypcbtext(TXT_USERRECORDUPDATED,NEWLINE|LOGIT|LFBEFORE);
    UsersData.PackedFlags.Dirty = TRUE;   /* mark the record as dirty */
    convertdatatoread(&UsersData,&UsersRead);
    putuserrecord(IGNOREDIRTY);
    Status.BackFromDoor = TRUE;   /* this will cause processuserrecord() to */
    getuserrecord(FALSE,FALSE);   /* remember the conference and also to    */
    Status.BackFromDoor = FALSE;  /* remember any bytes downloaded so _FAR_ */
  }
  checkaccountbalance();
  redisplaystatusline();
  ModifySelf = FALSE;
}
#pragma warn +par


void LIBENTRY onlineupdate(void) {
  int Save;

  Save = Status.CurSecLevel;
  Status.OnlineUpdate = TRUE;
  displaypcbtext(TXT_ONLINEUPGRADE,NEWLINE|LFBEFORE);
  UsersData.LastConference = Status.Conference;  /* remember where we are! */
  askquestions(&UsersData,ConfReg,MsgReadPtr,FALSE,Status.UserRecNo);  //lint !e534

  /* remove any additional security he might have had before processing */
  Status.TempAddSec = 0;
  Status.ConfAddSec = 0;

  displaypcbtext(TXT_ONLINEUPGRADEDONE,NEWLINE|LOGIT);
  UsersData.PackedFlags.Dirty = TRUE;   /* mark the record as dirty */
  convertdatatoread(&UsersData,&UsersRead);

  if (Status.NewUser && Status.UserRecNo == 0)
    adduserrecord(&UsersRead,&UsersRec,ConfReg,MsgReadPtr);   //lint !e534
  else
    putuserrecord(IGNOREDIRTY);

  getuserrecord(FALSE,FALSE);
  redisplaystatusline();
  Status.OnlineUpdate = FALSE;

  if (Save != Status.CurSecLevel) {
    Status.CmdLst[0] = 0;
    loadcmds();
  }
}



void LIBENTRY usermaint(int NumTokens) {
  bool     SkipIt;
  bool     PrintIt;
  bool     Temp;
  int      Direction;
  char     *p;
  long     MaxRec;
  long     RecNum;
  long     OldRecNum;
  char     Str[128];

  if (tempuseralloc(TRUE) == -1)
    return;

  MaxRec  = numrecs(UsersFile,sizeof(UsersRead));
  PrintIt = FALSE;
  SkipIt  = FALSE;
  RecNum  = 0;

  Direction = 1;  /* default to forward direction */

  while (1) {
    if (NumTokens == 0) {
      if (SkipIt || RecNum == 0)
        SkipIt = FALSE;
      else {
        if (RecNum == 1 && Status.UserRecNo != 1)
          RecNum = 2;
        Temp = Status.Printer;
        if (PrintIt && prn > 0)
          Status.Printer = TRUE;
        if (gettemprecord(RecNum) == -1)
          goto exit;
        if (TempRead->SecurityLevel <= UsersData.SecurityLevel)
          displayrec(TempRead,RecNum);
        PrintIt = FALSE;
        Status.Printer = Temp;
      }

      Str[0] = 0;
      inputfield(Str,(UsersData.ExpertMode ? TXT_USERMODEXPERT : TXT_USERMODNONEXPERT),6,UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_usrmaint);
      NumTokens = tokenize(Str);
      p = Str;
    } else
      p = getnexttoken();

    if (*p >= '0' && *p <= '9') {
      RecNum = (unsigned) atol(p);
      if (strchr(p,'+') != NULL)
        Direction = 1;
      else if (strchr(p,'-') != NULL)
        Direction = -1;
      if (RecNum > MaxRec) {
        MaxRec = numrecs(UsersFile,sizeof(UsersRead));
        if (RecNum > MaxRec)
          RecNum = MaxRec;
      }
    }

    #ifdef PCB_DEMO
      MaxRec = 10;
      if (RecNum >= 10)
        RecNum = 10;
    #endif

    switch (*p) {
      case '-':
      case '+': Direction = (*p == '+' ? 1 : -1);
                /* fall thru */
      case  0 : RecNum += Direction;
                if (RecNum > MaxRec || RecNum < 0)
                  goto exit;
                break;
      case 'A': OldRecNum = RecNum;
                #ifdef PCB_DEMO
                  if (numrecs(UsersFile,sizeof(UsersRead)) >= 10) {
                    newline();
                    printcolor(PCB_RED);
                    println("The users file is full.  You'll need the non-demo version to");
                    println("expand the users file any further.");
                    break;
                  }
                #endif
                if ((RecNum = adduser()) == -1)
                  RecNum = OldRecNum;
                else
                  MaxRec = numrecs(UsersFile,sizeof(UsersRead));
                break;
      case 'C': if (RecNum == 0 || TempRead->SecurityLevel > UsersData.SecurityLevel)
                  break;
                if (RecNum == Status.UserRecNo) {
                  UsersData.LastConference = Status.Conference;  /* remember where we are! */
                  askquestions(&UsersData,ConfReg,MsgReadPtr,FALSE,RecNum);    //lint !e534
                  UsersData.PackedFlags.Dirty = TRUE;   /* mark the record as dirty */
                  convertdatatoread(&UsersData,&UsersRead);
                  putuserrecord(IGNOREDIRTY);
                  getuserrecord(FALSE,FALSE);
                } else {
                  askquestions(TempData,TempReg,TempMsgPtr,FALSE,RecNum);  //lint !e534
                  TempData->PackedFlags.Dirty = TRUE;   /* mark the record as dirty */
                  convertdatatoread(TempData,TempRead);
                  puttemprecord(RecNum,IGNOREDIRTY);
                }
                break;
      case 'D': if (RecNum > 1) {
                  Str[0] = NoChar;
                  Str[1] = 0;
                  inputfield(Str,TXT_DELETERECORD,1,YESNO|UPCASE|NEWLINE|LFBEFORE|FIELDLEN,NOHELP,mask_yesno);
                  if (Str[0] == YesChar) {
                    TempRead->DeleteFlag       = 'Y';
                    TempRead->SecurityLevel    = 0;
                    TempRead->ExpSecurityLevel = 0;
                    TempRead->PackedFlags.Dirty = TRUE;   /* mark the record as dirty */
                    puttemprecord(RecNum,IGNOREDIRTY);
                  }
                }
                break;
      case 'F': OldRecNum = RecNum;
                if ((RecNum = findusername(NumTokens-1,RecNum)) == -1) {
                  RecNum = OldRecNum;
                  SkipIt = TRUE;
                }
                break;
      case 'L': listuserrecords();
                break;
      case 'P': if (! Status.Printer && prn > 0)
                  PrintIt = TRUE;
                break;
      case 'Q': goto exit;
      case 'S': scanuserrecords(NumTokens-1);
                SkipIt = TRUE;
                break;
      case 'U': if (RecNum > 1) {
                  TempRead->DeleteFlag = 'N';
                  TempRead->PackedFlags.Dirty = TRUE;   /* mark the record as dirty */
                  puttemprecord(RecNum,IGNOREDIRTY);
                }
                break;
      case 255: /* this one is special and gets called from MSGREAD.C */
                Str[0] = 0;
                for (NumTokens--; NumTokens; NumTokens--) {
                  p = getnexttoken();
                  strcat(Str,p);
                  addchar(Str,' ');
                }
                if ((RecNum = finduser(Str)) == -1)
                  goto exit;
                if (RecNum == 1 && Status.UserRecNo != 1)  /* don't let non-sysops look at the sysop record */
                  goto exit;
                break;
    }
    NumTokens = 0;
  }

exit:
  tempuserdealloc();
}


#ifdef PCBCOMM2
void LIBENTRY tersemodeuserinfo(void) {
  char Buf[90];

  sprintf(Buf,"[%-24.24s%-13.13s%-13.13s%-30.30s%c%c%c]",
          UsersData.City,
          UsersData.BusDataPhone,
          UsersData.HomeVoicePhone,
          UsersData.UserComment,
          (UsersData.PackedFlags.MsgClear ? 'Y' : 'N'),
          (UsersData.PackedFlags.DontAskFSE ? (UsersData.PackedFlags.FSEDefault ? 'Y' : 'N') : 'A'),
          (PcbData.AllowPwrdOnly ? 'N' : 'Y'));

  tersemodechecksum(Buf,83);
}
#endif
