#include "pcbsm_externs.h"
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


#include <io.h>
#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
extern int ExitKeyFlag[];
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <help.h>
#include <country.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define IGNORE     0
#define NOTEXPIRED 1
#define EXPIRESOON 2
#define EXPIRED    3

#define NUMPRINTFIELDS  4

typedef struct {
  long Start;
  long Stop;
  bool Exp;
  char Style[2];
} ListType;

#ifdef __WATCOMC__
extern hdrtype Header;
extern apptype AppHeader;
extern addresstypez Address;
extern addresstype OldAddress;
extern passwordtypez Password;
extern notestypez Notes;
#endif
static ListType List;
static int      PageNum;

char     *PrintHeadStr;
int      LineNum;
DOSFILE  prn;

static char  ATOC[3] = {0, 'A','C'};


static FldType PrintFields[NUMPRINTFIELDS] = {
  {vLONG ,ALLNUM,PRNTUSER, 3, 6, 8,CLEAR     ,"Starting Record","",&List.Start,NULL},
  {vLONG ,ALLNUM,PRNTUSER, 3, 7, 8,CLEAR     ,"Ending   Record","",&List.Stop ,NULL},
  {vBOOL ,YESNO ,PRNTUSER, 3, 8, 1,NOCLEARFLD,"Exp. Users Only","",&List.Exp  ,NULL},
  {vUPSTR,ATOC  ,PRNTUSER, 3,15, 1,NOCLEARFLD,"Printing Format","", List.Style,NULL}
};


static void closeprinter(void) {
  if (prn.handle > 0)
    dosfclose(&prn);
}

/********************************************************************
*
*  Function: openprint()
*
*  Desc    : Asks the users for the name of the device or file to send printer
*            output to.
*/

void pascal openprint(void) {
  int  Len;
  int  OpenParm;
  bool Save;
  int  Okay;

  dosfclose(&prn);
  Okay = -1;
  while (Okay == -1) {
    setcursor(CUR_NORMAL);
    showkeystatus();
    boxcls(3,16,76,22,Colors[OUTBOX],SINGLE);
    fastcenter(18,"Enter the name of a file or print device to send output to:",Colors[DISPLAY]);
    inputstr(5,20,35,"e.g. FILENAME.TXT or LPT1 or PRN",smConfig.Dev,smConfig.Dev,ALLFILE,INPUT_CAPS|INPUT_CLEAR,PRINTER);
    stripright(smConfig.Dev,' ');
    Len = strlen(smConfig.Dev);
    if (smConfig.Dev[Len-1] == ':')
      smConfig.Dev[Len-1] = 0;
    if (strcmp(smConfig.Dev,"PRN") == 0 || strncmp(smConfig.Dev,"LPT",3) == 0)
      OpenParm = OPEN_WRIT;
    else
      OpenParm = OPEN_RDWR|OPEN_APPEND|OPEN_DENYNONE;
    Okay = dosfopen(smConfig.Dev,OpenParm,&prn);
  }

  clsbox(4,20,75,20,Colors[OUTBOX]);
  Save = TRUE;
  inputnum(22,20,1,"Save printer definition to disk",&Save,vBOOL,0);
  if (Save && KeyFlags != ESC)
    writeconfig(&smConfig,ConfigName);

  atexit(closeprinter);      //lint !e534
}


/********************************************************************
*
*  Function: print()
*
*  Desc    : 1) Checks to see if the printer file or device is open
*            2) If not it opens it
*            3) Then prints to the printer
*/

static int near pascal print(char *Str) {
  int OpenParm;

  if (prn.handle == 0) {
    if (smConfig.Dev[0] == 0)
      openprint();
    else {
      strupr(smConfig.Dev);
      if (strcmp(smConfig.Dev,"PRN") == 0 || strncmp(smConfig.Dev,"LPT",3) == 0)
        OpenParm = OPEN_WRIT;
      else
        OpenParm = OPEN_WRIT|OPEN_APPEND|OPEN_DENYNONE;
      if (dosfopen(smConfig.Dev,OpenParm,&prn) == -1)
        return(-1);
    }
    atexit(closeprinter);    //lint !e534
  }
  return(dosfputs(Str,&prn));
}


/********************************************************************
*
*  Function: printheading()
*
*  Desc    : Prints a heading on the printer output page.
*/

int pascal printheading(const bool Top) {
  char Temp[100];
  char DStr[9];
  int  Len;
  int  Before;
  int  After;

  Len    = strlen(PrintHeadStr);
  Before = 31 - (Len >> 1);
  After  = 61 - Before - Len;
  datestr(DStr);
  countrydate(DStr);

  if (! Top) {
    if (print("\r\n") == -1)
      return(-1);
    PageNum++;
  } else {
    PageNum = 1;
  }
  sprintf(Temp,"%s%*s%s%*sPage:%5d\r\n\r\n\r\n",DStr,Before,"",PrintHeadStr,After,"",PageNum);
  if (print(Temp) == -1)
    return(-1);
  LineNum = 3;
  return(0);
}


/********************************************************************
*
*  Function: printline()
*
*  Desc    : Prints a line at a time and checks to see if we need to eject a
*            page and print a new header.
*/

static int pascal printline(char *Str, bool PerformHeadCheck) {
  if (print(Str) == -1)
    return(-1);

  LineNum++;
  if (PerformHeadCheck)
    if (LineNum > 58)
      return(printheading(FALSE));
  return(0);
}


/********************************************************************
*
*  Function: printrec()
*
*  Desc    : Prints the entire contents of a user record on the printer.
*/

int pascal printrec(const bool PerformHeadCheck) {
  char Tmp[512];
  char Date1[10];
  char Date2[10];
  int  X;

  // don't print deleted records
  if (memcmp(UsersRead.Name,"DELETED RECORD - IGNORE  ",25) == 0 ||
      memcmp(UsersRead.Name,"                         ",25) == 0)
    return(0);

  strcpy(Date1,countrydate(juliantodate(UsersData.DateLastDirRead)));
  strcpy(Date2,countrydate(juliantodate(UsersData.LastDateOn)));

  sprintf(Tmp,"Name: %-26sLoc: %-25sP/W: %-12s\r\n",UsersData.Name,UsersData.City,UsersData.Password);
  if (printline(Tmp,PerformHeadCheck) == -1)
    return(-1);

  sprintf(Tmp,"B/D: %-15sH/V: %-15sSec:%4d  Exp: %c  Prot: %c  Pg Len:%3d\r\n",
          UsersData.BusDataPhone , UsersData.HomeVoicePhone,
          UsersData.SecurityLevel, UsersRead.ExpertMode,
          UsersRead.Protocol     , UsersData.PageLen);
  if (printline(Tmp,PerformHeadCheck) == -1)
    return(-1);

  sprintf(Tmp,"Date Last DIR: %-10sLast On: %-9sat: %-7sMin:%4d Times On:%5d\r\n",
          Date1,
          Date2,
          UsersData.LastTimeOn,
          UsersData.ElapsedTimeOn,
          UsersData.NumTimesOn);
  if (printline(Tmp,PerformHeadCheck) == -1)
    return(-1);

  sprintf(Tmp,"Num UP:%5d  Num DN:%5d  Daily DN:%8ld  Tot UP:%8.0f Tot DN:%8.0f\r\n",
          UsersData.NumUploads,
          UsersData.NumDownloads,
          UsersData.DailyDnldBytes,
          UsersData.TotUpldBytes,
          UsersData.TotDnldBytes);
  if (printline(Tmp,PerformHeadCheck) == -1)
    return(-1);

  sprintf(Tmp,"Last in Conf: %2d  Exp Date: %-9sExp Sec:%4d Deleted: %c\r\n",
          UsersRead.LastConference,
          countrydate(juliantodate(UsersData.RegExpDate)),
          UsersData.ExpSecurityLevel,
          UsersRead.DeleteFlag);
  if (printline(Tmp,PerformHeadCheck) == -1)
    return(-1);

  sprintf(Tmp,"Cmnt 1: %-33sCmnt 2: %-30s\r\n",UsersData.UserComment,UsersData.SysopComment);
  if (printline(Tmp,PerformHeadCheck) == -1)
    return(-1);

  if (AliasSupport || VerifySupport || AddressSupport || NotesSupport) {
    // try to read USERS.INF information, if the read fails, do not return
    // a -1 to abort the process, instead, just let the printout skip the
    // PSA information for this record
    if (readusersinffile() == -1)
      return(0);

    convertreadtodata();

    if (AliasSupport) {
      sprintf(Tmp,"Alias: %s\r\n",Alias);
      if (printline(Tmp,PerformHeadCheck) == -1)
        return(-1);
    }

    if (VerifySupport) {
      sprintf(Tmp,"Verification Information: %s\r\n",Verify);
      if (printline(Tmp,PerformHeadCheck) == -1)
        return(-1);
    }

    if (AddressSupport) {
      stripright(Address.Street[0],' ');
      stripright(Address.Street[1],' ');
      stripright(Address.City,' ');
      stripright(Address.State,' ');
      stripright(Address.Zip,' ');
      stripright(Address.Country,' ');
      sprintf(Tmp,"Address Line 1        : %s\r\n",Address.Street[0]);
      if (printline(Tmp,PerformHeadCheck) == -1)
        return(-1);
      sprintf(Tmp,"Address Line 2        : %s\r\n",Address.Street[1]);
      if (printline(Tmp,PerformHeadCheck) == -1)
        return(-1);
      sprintf(Tmp,"City/State/Zip/Country: %s, %s %s %s\r\n",Address.City, Address.State,Address.Zip,Address.Country);
      if (printline(Tmp,PerformHeadCheck) == -1)
        return(-1);
    }

    if (NotesSupport) {
      for (X = 0; X < 5; X++) {
        stripright(Notes.Line[X],' ');
        if (Notes.Line[X][0] != 0) {
          sprintf(Tmp,"Notes %d: %s\r\n",X+1,Notes.Line[X]);
          if (printline(Tmp,PerformHeadCheck) == -1)
            return(-1);
        }
      }
    }
  }
  return(0);
}


/********************************************************************
*
*  Function: printshortrec()
*
*  Desc    : Prints only a short, columnar format, listing of a user record.
*            Data printed: Name, City, Security Level, Bus. & Home Phone.
*/

static int near pascal printshortrec(const bool PerformHeadCheck) {
  char Tmp[512];

  sprintf(Tmp,"%-23.23s %-21.21s %3d %-15s%-14s\r\n",
          UsersData.Name        , UsersData.City, UsersData.SecurityLevel,
          UsersData.BusDataPhone, UsersData.HomeVoicePhone);
  if (printline(Tmp,PerformHeadCheck) == -1)
    return(-1);
  return(0);
}


/********************************************************************
*
*  Function: printexprec()
*
*  Desc    : Prints only a short, columnar format, listing of a user record.
*            Data printed: Name, Bus. & Home Phone, Security Level, Exp Date
*            and flag
*/

static int near pascal printexprec(bool PerformHeadCheck, int Status) {
  char Tmp[512];
  char Str[20];

  switch(Status) {
    case IGNORE     :
    case NOTEXPIRED : Str[0] = 0; break;
    case EXPIRESOON : sprintf(Str,"  (%2d days)",UsersData.RegExpDate - Today); break;
    case EXPIRED    : strcpy(Str,"  (EXPIRED)");
  }

  sprintf(Tmp,"%-23.23s %-15s%-15s%4d%10s%s\r\n",
          UsersData.Name,
          UsersData.BusDataPhone,
          UsersData.HomeVoicePhone,
          UsersData.SecurityLevel,
          countrydate(juliantodate(UsersData.RegExpDate)),
          Str);
  if (printline(Tmp,PerformHeadCheck) == -1)
    return(-1);
  return(0);
}


/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Calls   :
*
*  Returns :
*/

static int near checkexpdate(void) {
  char static *IgnoreDate = "000000";

  if (memcmp(UsersRead.RegExpDate,IgnoreDate,6) == 0)
    return(IGNORE);

  if (UsersData.RegExpDate <= Today)
    return(EXPIRED);

  if (UsersData.RegExpDate <= TodayPlus30)
    return(EXPIRESOON);

  return(NOTEXPIRED);
}


/********************************************************************
*
*  Function: printlisting()
*
*  Desc    : Scans through the users file and prints all user records beginning
*            at a specified record number until reaching an ending record.
*
*/

void pascal printlisting(void) {
  char    Divider[81];
  long    Counter;
  int     UserStatus;
  bool    Aborted;


  if (openusersfile() != -1) {
    if (openusersinffile(PcbData.InfFile) != -1) {
      List.Start    = 1;
      List.Stop     = numrecs(UsersFile,sizeof(URead));
      List.Exp      = FALSE;
      List.Style[0] = 'A';
      List.Style[1] = 0;
      doslseek(UsersFile,0,SEEK_SET);

      clsbox(1,1,78,24,Colors[OUTBOX]);
      fastprint(3,11,"A) Print Short Form (single line: name,city,security,phone)"      ,Colors[DISPLAY]);
      fastprint(3,12,"B) Print Long  Form (multiple lines: full user record)"           ,Colors[DISPLAY]);
      fastprint(3,13,"C) Print Reg. Expiration Form (single line: name,phone,exp. date)",Colors[DISPLAY]);
      fastcenter(22," Press PGDN to Begin or ESC to Exit ",Colors[DESC]);

      initquest(PrintFields,NUMPRINTFIELDS);
      readscrn(PrintFields,NUMPRINTFIELDS-1,0,MainHead1,"Print Users Listing",1,NOCLEARFLD); //lint !e534
      freeanswers(PrintFields,NUMPRINTFIELDS);

      if (List.Style[1] < 'A')
        List.Style[1] = 'A';

      if (KeyFlags != ESC) {
        fastprintmove(3,20,"Printing, please wait (pressing ESC will abort the listing)...",Colors[DISPLAY]);
        PrintHeadStr = "Users File Listing";

        Aborted = FALSE;
        printheading(TRUE);  //lint !e534
        doslseek(UsersFile,(long) sizeof(URead) * (List.Start-1),SEEK_SET);

        memset(Divider,'-',77);
        Divider[77] = '\r';
        Divider[78] = '\n';
        Divider[79] = 0;

        for (Counter = List.Start; Counter <= List.Stop && ! Aborted; Counter++) {
          if (readcheck(UsersFile,&UsersRead,sizeof(URead)) == (unsigned) -1) {
            Aborted = TRUE;
            break;
          }

          #ifndef DEMO
            decryptusersrec(&UsersRead);
          #endif

          convertreadtodata();
          UserStatus = checkexpdate();

          if ((! List.Exp) || (UserStatus >= EXPIRESOON)) {
            switch(List.Style[0]) {
              case 'A' : printshortrec(TRUE);           //lint !e534
                         break;
              case 'B' : printline(Divider,TRUE);       //lint !e534
                         if (printrec(TRUE) == -1)
                           Aborted = TRUE;
                         break;
              case 'C' : printexprec(TRUE,UserStatus);  //lint !e534
                         break;
            }
          }
          if (userabort())
            Aborted = TRUE;
        }
        if (print("\r\n") == -1)
          Aborted = TRUE;
        else if (dosflush(&prn) == -1)
          Aborted = TRUE;
      }
      dosfclose(&DosUsersInfFile);
    }
    dosclose(UsersFile);
  }
  KeyFlags = NOTHING;
}
