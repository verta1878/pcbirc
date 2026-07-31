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
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#ifndef __OS2__
    #include "model.h"
#endif

#define NAMETHRUCITY 25+24
#define NAMETHRUSYSOPCOMMENT 183

char static Today[6];

void LIBENTRY scanusersfile(void LIBENTRY (*sub)(URead *p, long ScanRecNo)) {
  unsigned  BuffSize;
  int       NumBuffers;
  URead    *UserRecs;
  URead    *p;
  URead    *Ending;
  long      Recs;
  long      Counter;

  #ifdef __OS2__
    NumBuffers = 64;
  #else
    #ifdef LDATA
      NumBuffers = ((coreleft() > 0xFFFF ? 0xF000 : (int) coreleft()) / sizeof(URead));
    #else
      NumBuffers = coreleft() / sizeof(URead);
    #endif
    if (NumBuffers > 8)
      if (NumBuffers > 16)
        if (NumBuffers > 32)
          if (NumBuffers > 48)
            if (NumBuffers > 64)
              NumBuffers = 64;
            else
              NumBuffers = 48;
          else
            NumBuffers = 32;
        else
          NumBuffers = 16;
      else
        NumBuffers = 8;
    else
      return;
  #endif

  if ((UserRecs = (URead *) checkmalloc(NumBuffers * sizeof(URead),"USER SCAN")) == NULL)
    return;

  BuffSize = NumBuffers * sizeof(URead);
  Ending = &UserRecs[NumBuffers];
  Recs = numrecs(UsersFile,sizeof(URead));
  strtoyymmdd(Today,Status.LogonDate);

  if (Status.User == SYSOP) {
    Counter = 0;
    doslseek(UsersFile,0,SEEK_SET);
  } else {
    Counter = 1;
    doslseek(UsersFile,sizeof(URead),SEEK_SET);
  }

  showactivity(ACTBEGIN);
  for (; Counter < Recs; ) {
    if (Recs-Counter < NumBuffers) {
      NumBuffers = (unsigned) (Recs-Counter);
      BuffSize = NumBuffers * sizeof(URead);
      Ending = &UserRecs[NumBuffers];
    }

    if (readcheck(UsersFile,UserRecs,BuffSize) == (unsigned) -1)
      goto goback;

    for (p = UserRecs; p < Ending; p++, Counter++) {
      if (Display.AbortPrintout)
        goto goback;
      if (PcbData.Encrypt)
        decryptusersrec(p);
      sub(p,Counter+1);
    }
  }

goback:
  showactivity(ACTEND);
  bfree(UserRecs);
  checkdisplaystatus();
}


/********************************************************************
*
*  Function:  displayname()
*
*  Desc    :  displays the record number, user name, city and password
*
*  Notes   :  the function is defined here to avoid OVERLAY problems!
*/

void LIBENTRY displayname(URead *p, long Num) {
  char Str[100];

  if (p->SecurityLevel <= Status.CurSecLevel && p->ExpSecurityLevel <= Status.CurSecLevel) {
    printdefcolor();
    sprintf(Str,"%ld: %25.25s %24.24s",Num,p->Name,p->City);
    print(Str);

    // only show the password if the user reading the list has a security
    // level equal to or greater than what the sysop has

    if (Status.CurSecLevel >= Status.SysopLevel) {
      sprintf(Str," P/W: %12.12s",p->Password);
      print(Str);
    }

    newline();
  }
}


static void _NEAR_ LIBENTRY showline(char *Str) {
  stripright(Str,' ');
  println(Str);
}


/********************************************************************
*
*  Function:  displayrec()
*
*  Desc    :  displays a complete user record on the screen
*
*  Notes   :  the function is defined here to avoid OVERLAY problems!
*/

void LIBENTRY displayrec(URead *p, long Num) {
  int          X;
  double       Num1;
  double       Num2;
  char         Pwrd[13];
  char         NumStr1[20];
  char         NumStr2[20];
  char         Verify[26];
  char         Temp[80];
  char         Str[100];
  addresstype  Addr;
  notestype    Notes;
  passwordtype PwrdRecord;

  if (p->SecurityLevel <= Status.CurSecLevel && p->ExpSecurityLevel <= Status.CurSecLevel) {
    showactivity(ACTSUSPEND);
    newline();
    displayname(p,Num);

    yymmddtostr(Temp,p->LastDateOn);
    countrydate(Temp);
    sprintf(Str,"D: %13.13s V: %13.13s  LD: %s  LT: %5.5s  EX: %c  PR: %c",
            p->BusDataPhone,p->HomeVoicePhone,Temp,p->LastTimeOn,(p->ExpertMode == 'Y' ? YesChar : NoChar) ,p->Protocol);
    println(Str);

    sprintf(Str,"Sec: %d  E/Sec: %d  TO: %d  PL: %d  UP: %d  DN: %d  ET: %d  DL: %c",
            p->SecurityLevel,p->ExpSecurityLevel,p->NumTimesOn,p->PageLen,
            p->NumUploads,p->NumDownloads,p->ElapsedTimeOn,(p->DeleteFlag == 'Y' ? YesChar : NoChar));
    println(Str);

    yymmddtostr(Temp,p->RegExpDate);
    countrydate(Temp);

    Num1 = basdbletodouble(p->TotDnldBytes);

    // because the values used to be used only as unsigned long integers (but
    // stored as singled double precision floats), the new double might be a
    // negative value but it should be treated as positive, so convert it

    if (Num1 < 0)
      Num1 += 0xFFFFFFFFL + 1.0;

    dcomma(NumStr1,Num1);

    Num2 = basdbletodouble(p->TotUpldBytes);
    if (Num2 < 0)
      Num2 += 0xFFFFFFFFL + 1.0;

    dcomma(NumStr2,Num2);

    sprintf(Str,"E/Date: %s  D/D: %ld  T/D: %s  T/U: %s",
            Temp,
            basdbletolong(p->DailyDnldBytes),
            NumStr1,
            NumStr2);
    println(Str);

    sprintf(Str,"C1: %30.30s  C2: %30.30s",p->UserComment,p->SysopComment);
    println(Str);

    if (AliasSupport) {
      doslseek(UsersInfFile,(((p->RecNum)-1) * Hdr.TotalRecSize) + InfHeaderSize + AliasOffset,SEEK_SET);
      if (readcheck(UsersInfFile,Temp,25) == 25) {
        if (Temp[0] != 0 && Temp[0] != ' ') {
          sprintf(Str,"ALIAS: %-25.25s",Temp);
          showline(Str);
        }
      }
    }

    if (AddressSupport) {
      doslseek(UsersInfFile,(((p->RecNum)-1) * Hdr.TotalRecSize) + InfHeaderSize + AddressOffset,SEEK_SET);
      if (readcheck(UsersInfFile,&Addr,sizeof(addresstype)) == sizeof(addresstype)) {
        if (Addr.Street[0][0] != 0 && Addr.Street[0][0] != ' ') {
          sprintf(Str,"ST#%d: %-50.50s",1,Addr.Street[0]);
          showline(Str);
        }
        if (Addr.Street[1][0] != 0 && Addr.Street[1][0] != ' ') {
          sprintf(Str,"ST#%d: %-50.50s",2,Addr.Street[1]);
          showline(Str);
        }
        if (Addr.City[0] != ' ' && Addr.City[0] != 0) {
          sprintf(Temp,"CSZC: %-25.25s",Addr.City);
          stripright(Temp,' ');
          sprintf(Str,"%s, %-10.10s",Temp,Addr.State);
          stripright(Str,' ');
          memcpy(Temp,Addr.Zip,sizeof(Addr.Zip));
          Temp[sizeof(Addr.Zip)] = 0;
          strcat(Str,"  ");
          strcat(Str,Temp);
          stripright(Str,' ');
          memcpy(Temp,Addr.Country,sizeof(Addr.Country));
          Temp[sizeof(Addr.Country)] = 0;
          strcat(Str,"  ");
          strcat(Str,Temp);
          showline(Str);
        }
      }
    }

    if (NotesSupport) {
      doslseek(UsersInfFile,(((p->RecNum)-1) * Hdr.TotalRecSize) + InfHeaderSize + NotesOffset,SEEK_SET);
      if (readcheck(UsersInfFile,&Notes,sizeof(notestype)) == sizeof(notestype)) {
        for (X = 0; X < 5; X++) {
          movestr(Temp,Notes.Line[X],60);
          stripright(Temp,' ');
          if (Temp[0] != 0) {
            sprintf(Str,"NOTE: %s",Temp);
            showline(Str);
          }
        }
      }
    }

    if (VerifySupport) {
      doslseek(UsersInfFile,(((p->RecNum)-1) * Hdr.TotalRecSize) + InfHeaderSize + VerifyOffset,SEEK_SET);
      if (readcheck(UsersInfFile,Verify,sizeof(Verify)-1) == sizeof(Verify)-1) {
        Verify[sizeof(Verify)-1] = 0;
        stripright(Verify,' ');
        if (Verify[0] != 0) {
          sprintf(Str,"VERIFY: %s",Verify);
          showline(Str);
        }
      }
    }

    if (PasswordSupport) {
      doslseek(UsersInfFile,(((p->RecNum)-1) * Hdr.TotalRecSize) + InfHeaderSize + PasswordOffset,SEEK_SET);
      if (readcheck(UsersInfFile,&PwrdRecord,sizeof(PwrdRecord)) == sizeof(PwrdRecord)) {
        strcpy(Temp,juliantodate(PwrdRecord.ExpireDate));
        sprintf(Str,"PWRD E/D: %s  L/C: %s  T/C: %u",countrydate(Temp),countrydate(juliantodate(PwrdRecord.LastChange)),PwrdRecord.TimesChanged);
        showline(Str);
        Str[0] = 0;
        Pwrd[12] = 0;
        for (X = 0; X < 3; X++) {
          memcpy(Pwrd,PwrdRecord.Previous[X],12);
          stripright(Pwrd,' ');
          if (Pwrd[0] != 0) {
            if (Str[0] == 0)
              strcpy(Str,"PWRD");
            sprintf(Temp,"  #%d: %s",X+1,Pwrd);
            strcat(Str,Temp);
          }
        }
        if (Str[0] != 0)
          showline(Str);
      }
    }

    showactivity(ACTRESUME);
  } else
    showactivity(ACTSHOW);
}


#pragma warn -par
void LIBENTRY listusersinconf(URead *p, long ScanRecNo) {
  char Date[10];
  char Str[80];


  if (p->SecurityLevel == 0) {
    showactivity(ACTSHOW);
    return;
  }

  if (memcmp(p->Name,"~FIDO~",6) == 0) {
    showactivity(ACTSHOW);
    return;
  }

  /* check to see if we are searching for something and if it is NOT found */
  if (Status.SearchText[0] != 0 && ! parsersearch((char *)p,NAMETHRUCITY,Status.SearchInput,FALSE,0)) {
    /*checkstatus();*/
    showactivity(ACTSHOW);
    return;
  }

  /* we do this step last in order to save time in the event that   */
  /* we are having to go to the USERS.INF file which is much slower */
  /* than simply checking for a non-match in the text that is being */
  /* searched for...                                                */
  if (! checkuserreginconf(Status.Conference,p,Today)) {
    /*checkstatus();*/
    showactivity(ACTSHOW);
    return;
  }

  showactivity(ACTSUSPEND);
  printdefcolor();
  yymmddtostr(Date,p->LastDateOn);
  countrydate(Date);
  sprintf(Str,"%25.25s %24.24s  %8.8s  %5.5s",p->Name,p->City,Date,p->LastTimeOn);
  stripright(Str,' ');
  println(Str);
  showactivity(ACTRESUME);
}


void LIBENTRY listusers(URead *p, long ScanRecNo) {
  char Date[10];
  char Str[80];

  if (memcmp(p->Name,"~FIDO~",6) == 0) {
    showactivity(ACTSHOW);
    return;
  }

  showactivity(ACTSUSPEND);
  yymmddtostr(Date,p->LastDateOn);
  countrydate(Date);
  sprintf(Str,"%25.25s %24.24s  %8.8s  %5.5s  %4d min.",
          p->Name,
          p->City,
          Date,
          p->LastTimeOn,
          p->ElapsedTimeOn);

  println(Str);
  showactivity(ACTRESUME);
}
#pragma warn +par


void LIBENTRY viewprintusers(int NumTokens) {
  bool Temp;
  char Str[2];
  char *p;

  Temp = Status.Printer;

  if (NumTokens) {
    p = getnexttoken();
    if (*(p+1) == 0) {
      switch (*p) {
        case 'V': break;
        case 'P': Status.Printer = (bool) (prn > 0); startdisplay(FORCENONSTOP); break;
        default : return;
      }
    }
  } else {
    Str[0] = 0;
    inputfield(Str,TXT_VIEWPRINTUSERS,1,FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_pv);
    p = Str;
    switch (*p) {
      case 'V': break;
      case 'P': Status.Printer = (bool) (prn > 0); startdisplay(FORCENONSTOP); break;
      default : return;
    }
  }

  newline();
  scanusersfile(listusers);
  logsystext(TXT_USERSFILEVIEWED,SPACERIGHT);
  Status.Printer = Temp;
}


static void LIBENTRY checkformatch(URead *p, long ScanRecNo) {
  if (parsersearch((char *)p,NAMETHRUSYSOPCOMMENT,Status.SearchInput,FALSE,0) != 0) {
    showactivity(ACTSUSPEND);
    displayname(p,ScanRecNo);
    showactivity(ACTRESUME);
    return;
  }
  showactivity(ACTSHOW);
  /*checkstatus();*/
}


void LIBENTRY scanuserrecords(int NumTokens) {
  char *p;
  char Text[80];
  char Input[80];

  // The TS command might be used, the a Find User, then a Scan ... this
  // could cause the search text to be lost.  Save it here.
  memcpy(Text,Status.SearchText,80);
  memcpy(Input,Status.SearchInput,80);
  Status.SearchText[0] = 0;

  if (NumTokens) {
    for (; NumTokens; NumTokens--) {
      p = getnexttoken();
      if (strlen(Status.SearchText)+strlen(p)+1 < sizeof(Status.SearchText)-1) {
        strcat(Status.SearchText,p);
        addchar(Status.SearchText,' ');
      }
    }
  } else {
    inputfield(Status.SearchText,TXT_TEXTTOSCANFOR,sizeof(Status.SearchText)-1,UPCASE|NEWLINE|LFBEFORE|HIGHASCII,HLP_SRCH,mask_alphanum);
    if (Status.SearchText[0] == 0)
      return;
  }

  if (tokenscan(Status.SearchText,Status.SearchInput,FALSE) == -1) {
    displaypcbtext(TXT_PUNCTUATIONERROR,NEWLINE|LFBEFORE);
    return;
  }

  scanusersfile(checkformatch);
  stopsearch();
  Status.SearchText[0] = 0;

  // Restore the TS search (if needed) here
  if (Text[0] != 0) {
    memcpy(Status.SearchText ,Text ,80);
    memcpy(Status.SearchInput,Input,80);
    tokenscan(Status.SearchText,Status.SearchInput,FALSE); //lint !e534
  }
}


void LIBENTRY listuserrecords(void) {
  startdisplay(NOCHANGE);
  scanusersfile(displayrec);
}


void LIBENTRY scanforusers(int NumTokens) {
  char *p;

  if (tempuseralloc(FALSE) == -1)
    return;

  Status.SearchText[0] = 0;
  for (; NumTokens; NumTokens--) {
    p = getnexttoken();
    if (strlen(p)+strlen(Status.SearchText)+1 < sizeof(Status.SearchText)-1) {
      strcat(Status.SearchText,p);
      addchar(Status.SearchText,' ');
    }
  }

  if (Status.SearchText[0] == 0)
    inputfield(Status.SearchText,TXT_USERSCAN,sizeof(Status.SearchText)-1,UPCASE|HIGHASCII|NEWLINE|LFBEFORE,HLP_SRCH,mask_alphanum);

  stripright(Status.SearchText,' ');
  if (Status.SearchText[0] != 0) {
    if (tokenscan(Status.SearchText,Status.SearchInput,FALSE) == -1) {
      displaypcbtext(TXT_PUNCTUATIONERROR,NEWLINE|LFBEFORE);
      goto end;
    }
  }

  startdisplay(NOCHANGE);
  displaypcbtext(TXT_USERSHEADER,NEWLINE|LFBEFORE|NOTBLANK);
  displaypcbtext(TXT_USERSCANLINE,NEWLINE|NOTBLANK);
  printdefcolor();
  scanusersfile(listusersinconf);

  if (Status.SearchText[0] != 0) {
    stopsearch();
    Status.SearchText[0] = 0;
  }

end:
  checkdisplaystatus();
  Status.SearchText[0] = 0;

  tempuserdealloc();
}

void LIBENTRY listconfmembers(void) {
  if (tempuseralloc(FALSE) == -1)
    return;

  Status.SearchText[0] = 0;
  newline();
  startdisplay(NOCHANGE);
  displaypcbtext(TXT_USERSHEADER,NEWLINE|LFBEFORE|NOTBLANK);
  displaypcbtext(TXT_USERSCANLINE,NEWLINE|NOTBLANK);
  printdefcolor();
  scanusersfile(listusersinconf);
  checkdisplaystatus();
  tempuserdealloc();
}
