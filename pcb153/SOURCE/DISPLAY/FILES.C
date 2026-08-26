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
  #include <direct.h>
  #include <borland.h>
#else
/*  #include <dir.h> */
#endif

#include "wild.h"

#ifndef LIB
/*  #include <account.h> */
  #include "newscr.h"
  #include "menu.h"
#endif

#define DISPLAYBUFSIZE 2048+1
#define MAXFILES       20

#define DIFFERENTDRIVES(d1,d2)  ((d1[1] == ':') && (d1[0] != d2[0]))

enum {
  ALT_SLR,         /*  security + language + ripscrip = BRDM10R.FRE */
  ALT_SLG,         /*  security + language + graphics = BRDM10G.FRE */
  ALT_SMR,         /*  security + menu     + ripscrip = BRDM10R.MNU */
  ALT_SMG,         /*  security + menu     + graphics = BRDM10G.MNU */
  ALT_SL,          /*  security + language            = BRDM10.FRE  */
  ALT_SM,          /*  security + menu                = BRDM10.MNU  */
  ALT_SR,          /*  security + ripscrip            = BRDM10R     */
  ALT_SG,          /*  security + graphics            = BRDM10G     */
  ALT_S,           /*  security                       = BRDM10      */
  ALT_LR,          /*  language + ripscrip            = BRDMR.FRE   */
  ALT_LG,          /*  language + graphics            = BRDMG.FRE   */
  ALT_MR,          /*  menu     + ripscrip            = BRDMR.MNU   */
  ALT_MG,          /*  menu     + graphics            = BRDMG.MNU   */
  ALT_L,           /*  language                       = BRDM.FRE    */
  ALT_M,           /*  menu                           = BRDM.MNU    */
  ALT_R,           /*  ripscrip                       = BRDMR       */
  ALT_G,           /*  graphics                       = BRDMG       */
  ALT_DEFAULT,     /*  default                        = BRDM        */
  ALT_TOTAL };

#ifndef LIB
prottype Protocols[36];
char ValidProtocols[37];
static bool FoundBatch;

extern struct ffblk DTA;  /* declared in exist.c */

/* These two functions are used to maintain compatibility with pre-15.0 code */
/* where the PWRD file had FEWER FIELDS than it does now. */

static bool _NEAR_ LIBENTRY yes(char *Str, bool Default) {
  if (Str == NULL || *Str == 0)
    return(Default);
  if (*Str == 'Y')
    return(TRUE);
  return(FALSE);
}


void LIBENTRY readpwrdfile(int PrevTimeOn) {
  #ifdef COMM
  onlinetype Temp;
  #endif
  int     Limit;
  int     SecLevel;
  int     Time;
  int     NewTime;
  int     Bytes;
  char    *Password;
  #ifndef PCB_DEMO
  long    BaseBaud;
  long    BaudFactor;
  #endif
  DOSFILE File;
  char    Str[100];

  if (Status.CurSecLevel == 0)
    return;

  Time = Bytes = Limit = 0;

  #ifndef PCB_DEMO
    BaseBaud = 0;
  #endif


  if (dosfopen(PcbData.PwdFile,OPEN_READ|OPEN_DENYNONE,&File) != -1) {

top:
    while (dosfgets(Str,sizeof(Str)-1,&File) != -1) {
      Password = parse(Str);
      SecLevel = atoi(parse(NULL));

      if (SecLevel == Status.CurSecLevel) {
        if (Password[0] != 0 && stricmp(Password,UsersData.Password) != 0)
          continue;
        Time     = atoi(parse(NULL));
        Bytes    = atoi(parse(NULL));
        #ifdef PCB_DEMO
          parse(NULL);
        #else
          BaseBaud = atol(parse(NULL));
        #endif
        Limit    = atoi(parse(NULL));
        #ifdef PCB_DEMO
          Bytes    = max(Bytes,100);
          Limit    = max(Limit,5);
        #endif

        Status.FileRatio        = atol(parse(NULL));
        Status.ByteRatio        = atol(parse(NULL));
        Status.TotalFileLimit   = atol(parse(NULL));
        Status.TotalKByteLimit  = atol(parse(NULL));

        if (PcbData.EnforceTime)
          Status.EnforceTimeLimit = yes(parse(NULL),TRUE);
        else {
          Status.EnforceTimeLimit = FALSE;
          parse(NULL);  /*lint !e534   waste it! */
        }

        Status.AllowAlias        = yes(parse(NULL),FALSE);
        Status.ForceReadMail     = yes(parse(NULL),FALSE);
        Status.MultipleLogins    = yes(parse(NULL),FALSE);

        /* if we're NOT coming back from a door (i.e. fresh login)        */
        /* and the system is set to either NOT enforce the time limit, OR */
        /* the caller is online as a DEMO ACCOUNT, then ignore the        */
        /* time previously used online and give the caller a full session */

        if (! Status.BackFromDoor)
          if (Status.EnforceTimeLimit == FALSE || Status.MultipleLogins) {
            PrevTimeOn = 0;
            UsersData.ElapsedTimeOn = 0;
          }

        /* VerifyCaller is not being used.  However, we must still call */
        /* the parse() routine because there are more settings AFTER it */

        /* Status.VerifyCaller      =  */ yes(parse(NULL),FALSE); /*lint !e534 */
        Status.FileRatioCredits  = atoi(parse(NULL));
        Status.KByteRatioCredits = atoi(parse(NULL));

        #ifdef PCB152
        {
          char ActStatus;

          ActStatus = * parse(NULL);
          Status.ActStatus = ACT_DISABLED;

          if (AccountSupport && PcbData.EnableAccounting) {
            switch (ActStatus) {
              case 'T': if (PcbData.AccountTrack[0] != 0)
                          Status.ActStatus = ACT_TRACKONLY;
                        break;
              case 'Y': Status.ActStatus = ACT_ENFORCE;
                        break;
            }
          }

          calculatebalance();

          /* Check to see if the sysop wants to use the DropSecLevel (the   */
          /* security used on an empty account) be seeing if it is set to   */
          /* NOT ignore the value.  Then, if not ignoring, check to see if  */
          /* the user's credits are gone and, if so, check to see if the    */
          /* DropSecLevel != CurSecLevel.  If so, then go back up and re-   */
          /* read the PWRD file with the new security level to pick up any  */
          /* changes that might correspond with the new level.  Record the  */
          /* Status.UseDropSecLevel setting so that the processuserrecord() */
          /* function will know to use the proper security level.           */

          if (! PcbData.IgnoreDropSecLevel) {
            if (Status.ActStatus == ACT_ENFORCE && Status.Balance <= 0) {
              if (Status.CurSecLevel != UsersData.Account.DropSecLevel) {
/* temporarily set curseclevel, processuserrecord() will */
/* reset this value again the normal way */
                Status.CurSecLevel = UsersData.Account.DropSecLevel;
                Status.UseDropSecLevel = TRUE;
                dosfseek(&File,0,SEEK_SET);
                goto top;
              }
            }
          }
        }
        #endif
        break;
      }
    }
  } else {
    /* set defaults for when the PWRD file cannot be opened and read */
    SecLevel = 0;
  }

  if (FoundBatch) {
    if (Limit == 0) {
      #ifdef PCB_DEMO
        Status.BatchLimit = 3;
      #else
        Status.BatchLimit = 30;
      #endif
    } else
      Status.BatchLimit = (short) Limit;
  } else
    Status.BatchLimit = 1;

  if (SecLevel != Status.CurSecLevel) {
    #ifdef COMM
      /* avoid sending data to comm port if modem not open */
      Temp = Asy.Online;
      if (Asy.Online == REMOTE && ! ModemOpened)
        Asy.Online = LOCAL;
      displaypcbtext(TXT_NOMATCHINPWRD,NEWLINE|LOGIT);
      Asy.Online = Temp;
    #else
      displaypcbtext(TXT_NOMATCHINPWRD,NEWLINE|LOGIT);
    #endif
    Time = 10;  /* default to 10 minutes if no match is found */
  }

  NewTime = Time - PrevTimeOn;  /* subtract out any previously used time */
  if (NewTime < 0)              /* but make sure it doesn't go negative  */
    NewTime = 0;

  #ifdef PCB152
  {
/*  long ActMinLeft; */

/*  // If the caller didn't have sufficient credits to use up one minute, */
/*  // he'd be logged off.  But that prevents him from using up the REST of */
/*  // his credits so PCBoard couldn't give him the drop-sec-level.  The */
/*  // +1 below works around this by giving him one extra minute to use */
/*  // up credits and, potentially, go negative ... but at *least* it will */
/*  // run the account dry so that the drop-sec-level can be used */

/*  if (Status.ActStatus == ACT_ENFORCE && AccountRates.ChargeForTime != 0) { */
/*    ActMinLeft = (long) (Status.Balance / AccountRates.ChargeForTime) + 1; */
/*    if (NewTime > ActMinLeft) */
/*      NewTime = (int) ActMinLeft; */
/*  } */
  }
  #endif

  addtime(NewTime * 60L,NEWTIME);
  Status.PwrdTimeAllowed  = (short) Time;
  Status.MaxTimeAllowed   = (short) (Status.TimeAdjustedForEvent ? minutesleft() + 1 : Time);
  Status.MaxKBytesAllowed = (short) Bytes;
  Status.BytesRemaining   = (Bytes == 32767 ? -1 : (long) Bytes * 1024);

  if (Bytes == 32767)
    Status.BytesRemaining = -1;
  else {
    Status.BytesRemaining = (long) Bytes * 1024L;
    #ifndef PCB_DEMO
      if (BaseBaud != 0) {
        if (Asy.CarrierSpeed >= BaseBaud) {
          BaudFactor = Asy.CarrierSpeed / BaseBaud;
          Status.BytesRemaining *= BaudFactor;
        } else if (Asy.CarrierSpeed != 0) {
          BaudFactor = BaseBaud / Asy.CarrierSpeed;
          Status.BytesRemaining /= BaudFactor;
        }
      }
    #endif
    Status.MaxKBytesAllowed = (short) (Status.BytesRemaining / 1024);
  }
  dosfclose(&File);
}


void LIBENTRY readpwrdfileadjustsecurity(adjusttype Adjust) {
  int     SecLevel;
  int     Previous;
  int     Temp;
  DOSFILE File;
  char    Str[100];

  if (dosfopen(PcbData.PwdFile,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return;

  Previous = 0;
  Temp     = 0;

  while (dosfgets(Str,sizeof(Str)-1,&File) != -1) {
    parse(Str);  /*lint !e534 throw away password */
    SecLevel = atoi(parse(NULL));
    switch (Adjust) {
      case INCREASE: if (SecLevel > Status.CurSecLevel) {
                       Temp = (SecLevel - Status.CurSecLevel);
                       goto done;
                     }
                     break;
      case DECREASE: if (SecLevel >= Status.CurSecLevel) {
                       Temp = (Previous - Status.CurSecLevel);
                       goto done;
                     }
    }
    Previous = SecLevel;
  }

done:
  Status.TempAddSec  += (short) Temp;
  Status.CurSecLevel += (char)  Temp;
  dosfclose(&File);
}


bool LIBENTRY readtcanfile(char *Name) {
  bool    Found;
  int     NumTokens;
  int     X;
  DOSFILE File;
  char    Buffer[80];
  char    Str[100];

  if (PcbData.TcnFile[0] == 0)
    return(FALSE);

  if (dosfopen(PcbData.TcnFile,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return(FALSE);

  dossetbuf(&File,4096);

  Found = FALSE;
  while (dosfgets(Str,sizeof(Str)-1,&File) != -1) {
    strupr(Str);
    if (strcmp(Str,Name) == 0) {
      Found = TRUE;
      goto done;
    }
    maxstrcpy(Buffer,Name,sizeof(Buffer));
    NumTokens = tokenize(Buffer);
    for (X = 0; X < NumTokens; X++) {
      if (strcmp(Str,getnexttoken()) == 0) {
        Found = TRUE;
        goto done;
      }
    }
  }

done:
  dosfclose(&File);
  return(Found);
}


int LIBENTRY protoffset(int Letter) {
  if (Letter < '0' || Letter > 'Z')
    return(-1);
  if (Letter >= 'A')
    return(Letter-'A');
  if (Letter <= '9')
    return(Letter-'0'+26);
  return(-1);
}


bool LIBENTRY isprotvalid(char Letter) {
  if (strnchr(&ValidProtocols[1],Letter,ValidProtocols[0]) != NULL)
    if (! Protocols[protoffset(Letter)].ErrCorrReq || Asy.ErrorCorrected)
      return(TRUE);
  return(FALSE);
}


bool LIBENTRY isprotbatch(int Letter) {
  if ((Letter = protoffset(Letter)) == -1)
    return(FALSE);
  return((bool) Protocols[Letter].Batch);
}


char * LIBENTRY protdesc(int Letter) {
  if ((Letter = protoffset(Letter)) == -1)
    return("");
  return(Protocols[Letter].Desc);
}


void LIBENTRY readprotfile(void) {
  char static *Name = "PCB!x";
  char     Letter;
  int      Offset;
  char     *p;
  prottype *q;
  char     *r;
  DOSFILE  File;
  char     Str[100];
  char     Temp[100];

  FoundBatch = FALSE;
  memset(Protocols,0,sizeof(Protocols));
  memset(ValidProtocols,0,sizeof(ValidProtocols));

  if (dosfopen(PcbData.TrnFile,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    errorexittodos("Unable to open PCBPROT.DAT file");

  r = &ValidProtocols[1];

  while (dosfgets(Str,sizeof(Str)-1,&File) != -1) {
    p = parse(Str);
    Letter = *p;
    if ((Offset = protoffset(Letter)) == -1)
      continue;

    *r = Letter;
    q  = &Protocols[Offset];
    p  = parse(NULL);
    q->BiDirect = FALSE;

    switch (*p) {
      case 'I': q->Internal   = (strchr("ACFGNOXY",Letter) != NULL);
                q->Batch      = (Letter == 'Y' || Letter == 'G' || Letter == 'N');
                q->ErrCorrReq = (Letter == 'F' || Letter == 'G');
                break;
      case 'E': q->Internal = FALSE;
                q->Batch    = FALSE;
                break;
      case 'B': q->BiDirect = TRUE;    /*lint !e542 */
                /* fall thru */
      case 'D': q->Internal = FALSE;
                q->Batch    = TRUE;    /*lint !e542 */
                break;
    }

    if (! q->Internal) {
      Name[4] = Letter;
      Name[3] = 'S';
      if (findbat(Temp,Name) == -1)
        continue;

      Name[3] = 'R';
      if (findbat(Temp,Name) == -1)
        continue;
    }

    if (Letter != 'N' && q->Batch)
      FoundBatch = TRUE;

    if ((q->BlockSize = atoi(parse(NULL))) == 0)
      q->BlockSize = 1024;  /* make SURE we have a valid block size! */

    maxstrcpy(Temp,parse(NULL),sizeof(Temp));
    maxstrcpy(q->Desc,Temp,sizeof(q->Desc));
    p = parse(NULL);
    if (! q->Internal)
      q->ErrCorrReq = (*p == 'Y');

    p = parse(NULL);
    q->PortOpen = (*p == 'Y');
    p = parse(NULL);
    q->LockLines = (*p == 'Y');
    r++;
    ValidProtocols[0]++;
  }

  dosfclose(&File);
}


static char * _NEAR_ LIBENTRY foundintcan(char *Name) {
  static  char Buf[13];
  char    Criteria[12];
  DOSFILE File;

  if (dosfopen(PcbData.FileTcan,OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return(NULL);

  while (dosfgets(Buf,sizeof(Buf),&File) != -1) {
    criteria(Buf,Criteria);  /* set up a wildcard matching criteria */
    if (wildmatch(Name,Criteria)) {  /* does our filename match the criteria? */
      dosfclose(&File);
      return(Buf);
    }
  }

  dosfclose(&File);
  return(NULL);
}


/********************************************************************
*
*  Function:  dosreservedword()
*
*  Desc    :  Checks to see if the parameter passed has one of the DOS reserved
*             words such as CLOCK$ or COM1 or PRN in the filename.  Use of
*             these words can cause a machine to lock up.
*
*  Returns :  TRUE if a reserved word is found, otherwise FALSE.
*/

bool LIBENTRY dosreservedword(char *Name) {
   return((bool) (strstr(Name,"CLOCK$") != NULL ||
          (memcmp(Name,"COM",3) == 0 && (Name[3] >= '1' && Name[3] <= '4') && (Name[4] == ':' || Name[4] == 0)) ||
          (memcmp(Name,"LPT",3) == 0 && (Name[3] >= '1' && Name[3] <= '3') && (Name[4] == ':' || Name[4] == 0)) ||
          strcmp(Name,"AUX") == 0 ||
          strcmp(Name,"PRN") == 0 ||
          strcmp(Name,"CON") == 0));
}


static bool _NEAR_ LIBENTRY matchspecialfile(char *Name, char *FullPath, char *SpecialFile) {
  char Q1[128];
  char Q2[128];

  if (stricmp(Name,findstartofname(SpecialFile)) == 0) {
    fullyqualifiedname(Q1,FullPath,sizeof(Q1));
    fullyqualifiedname(Q2,SpecialFile,sizeof(Q2));
    if (strcmp(Q1,Q2) == 0)
      return(TRUE);
  }
  return(FALSE);
}


static bool _NEAR_ LIBENTRY specialfile(char *FullPath) {
  char *Name;

  Name = findstartofname(FullPath);
  if (matchspecialfile(Name,FullPath,PcbData.UsrFile) ||
      matchspecialfile(Name,FullPath,PcbData.InfFile) ||
      matchspecialfile(Name,FullPath,PcbData.PwdFile) ||
      matchspecialfile(Name,FullPath,PcbData.FscFile) ||
      matchspecialfile(Name,FullPath,PcbData.UscFile) ||
      matchspecialfile(Name,FullPath,DatFile))
    return(TRUE);
  return(FALSE);
}


/********************************************************************
*
*  Function:  fsecokay()
*
*  Desc    :  Used to check both UPLOADS and DOWNLOADS for security and/or
*             password restrictions
*
*  Returns :  -1 = security or password violation
*              0 = passed security req
*              1 = passed security req AND file is considered 'free'
*              2 = passed security req AND time will not be counted
*              3 = passed security req AND both time and bytes are free
*
*  Internal:  Internally, RetVal is set to -2 until a match is found in the
*             FSEC file.  If no matches are found, then it is set to 0 to
*             show that the file passed the security requirements.  If a match
*             *is* found, then it is set to 0, 1, 2, or 3, according to the
*             line it matched on.
*
*             If RetVal is not equal to -2 at the bottom of the function, then
*             Status.FsecMultiplier is set to whatever value was passed in the
*             FSEC file (if any).
*
*  Notes   :  A 'free' file is one that can be downloaded without being charged
*             on the user's download bytes or download files fields.
*
*             If the file is 'free' AND it has a password of "NOTIME" then
*             the caller will not be charged for time to download the file.
*
*             An "Alternate Path" (AltPath) is used by the UPLOAD process to
*             allow the UPSEC file to check *both* the private upload directory
*             (which is contained in FullPath) as well as the public upload
*             directory (which is passed in as AltPath).  If AltPath==NULL then
*             no checking takes place.  If AltPath!=NULL then the path is
*             checked in addition to the one in FullPath.
*/

int LIBENTRY fsecokay(char *FullPath, char *Name, checksectype CheckSec, char *AltPath) {
  bool     EnforceSec;
  int      Sec;
  int      InsufSec;
  int      Password;
  int      BadPwrd;
  int      RetVal;
  char    *FsecName;
  char    *pPath;
  char    *pFileName;
  char    *pSecurity;
  char    *pPassword;
  #ifdef PCB152
  char    *pMultiplier;
  #endif
  char     Criteria[12];
  char     Pwrd[13];
  DOSFILE  file;
  char     Path[66];
  char     Buffer[128];

  #ifdef PCB152
    Status.FsecMultiplier = 1;
  #endif

/* if someone is just uploading a file into the message editor or attaching */
/* a file then skip the UPSEC check. */

  if (Status.EnteringMessage)
    return(0);

  switch (CheckSec) {
    case CHECKVIEW : FsecName = PcbData.FscFile;
                     InsufSec = TXT_INSUFSECTOVIEW;
                     Password = TXT_PWRDFORVIEW;
                     BadPwrd  = TXT_BADVIEWPWRD;
                     EnforceSec = (bool) (! IgnoreViewSec);
                     break;
    case CHECKFSEC : FsecName = PcbData.FscFile;
                     InsufSec = TXT_INSUFSECTODLFILE;
                     Password = TXT_PWRDFORDOWNLOAD;
                     BadPwrd  = TXT_BADDOWNLOADPWRD;
                     EnforceSec = TRUE;
                     break;
    case CHECKUPSEC: FsecName = PcbData.UscFile;
                     InsufSec = TXT_BADUPLOADFORMAT;
                     Password = TXT_PWRDFORUPLOAD;
                     BadPwrd  = TXT_BADUPLOADPWRD;
                     EnforceSec = TRUE;
                     break;
    default        : return(0);
  }

  maxstrcpy(Status.DisplayText,Name,sizeof(Status.DisplayText));
  strupr(FullPath);

  if (specialfile(FullPath) || dosreservedword(Name)) {
    displaypcbtext(InsufSec,NEWLINE|LFAFTER|LOGIT);
    return(-1);
  }

  if (CheckSec == CHECKUPSEC && PcbData.FileTcan[0] != 0 && fileexist(PcbData.FileTcan) != 255)
    if ((pFileName = foundintcan(Name)) != NULL) {
      maxstrcpy(Status.DisplayText,pFileName,sizeof(Status.DisplayText));
      displaypcbtext(TXT_FILENOTALLOWED,NEWLINE|LFAFTER|LOGIT);
      return(-1);
    }

  if (FsecName[0] == 0 || fileexist(FsecName) == 255)
    return(0);

  if (dosfopen(FsecName,OPEN_READ|OPEN_DENYNONE,&file) == -1)
    return(0);

  maxstrcpy(Path,FullPath,sizeof(Path));
  if ((pPath = strrchr(Path,'\\')) != NULL || (pPath = strrchr(Path,':')) != NULL)
    *(pPath+1) = 0;

  RetVal = -2;
  while (dosfgets(Buffer,128,&file) != -1) {
    strupr(Buffer);

    pPath       = parse(Buffer);
    pFileName   = parse(NULL);
    pSecurity   = parse(NULL);
    pPassword   = parse(NULL);
    #ifdef PCB152
    pMultiplier = parse(NULL);
    #endif

    /* the manual for v15.2 claims that a blank filename is equal to *.* */
    /* so now that it's in print, we'll have to make it so... 11/9/94    */
    if (*pFileName == 0)
      pFileName = "*.*";

    if (*pPath != 0) {            /* if the path is not blank ...         */

      /* Check for just a DRIVE designator such as C: which would protect */
      /* an entire drive.                                                 */

      if (strlen(pPath) == 2 && pPath[1] == ':') {
        if (AltPath == NULL) {
          if (DIFFERENTDRIVES(pPath,Path))
            continue;
        } else {
          if (DIFFERENTDRIVES(pPath,Path) && DIFFERENTDRIVES(pPath,AltPath))
            continue;
        }
      } else {
        if (AltPath == NULL) {
          if (strcmp(pPath,Path) != 0)  /* no match on complete path? */
            continue;               /* loop up and try again      */
        } else {
          if (strcmp(pPath,Path) != 0 && strcmp(pPath,AltPath) != 0)  /* no match on complete path? */
            continue;                                                /* loop up and try again      */
        }
      }
    }

    criteria(pFileName,Criteria);  /* set up a wildcard matching criteria   */
    if (wildmatch(Name,Criteria)){ /* does our filename match the criteria? */
      Sec = atoi(pSecurity);       /* get the security level requirement    */
      if (EnforceSec && Status.CurSecLevel<Sec){ /* if enforcing security, and security too low, inform user and get out */
        displaypcbtext(InsufSec,NEWLINE|LFAFTER|LOGIT);
        RetVal = -1;
        break;
      } else if (Sec == 0) {      /* is the security requirement 0?          */
        #ifndef PCB_DEMO
          RetVal = 1;             /*   yes, set it to a 'free' file          */
        #endif
      }

      /* get the password and if it isn't blank then check */
      /* to see if we should ask the caller for a password */
      if (*pPassword != 0) {
        #ifndef PCB_DEMO
          /* FREE bytes:  use "FREE" for backward compatibility */
          if (stricmp(pPassword,"FREE") == 0 || stricmp(pPassword,"FREEFILE") == 0) {
            RetVal = 1;
            break;  /* get out now before checking password */
          }

          /* FREE time:  new, gives *only* time back, not bytes */
          if (stricmp(pPassword,"FREETIME") == 0) {
            RetVal = 2;
            break;  /* get out now before checking password */
          }

          /* FREE time and bytes:  use "NOTIME" for backward compatibilty */
          if (stricmp(pPassword,"NOTIME") == 0 || stricmp(pPassword,"FREETIMEFILE") == 0) {
            RetVal = 3;
            break;  /* get out now before checking password */
          }
        #endif

        /* if the password matches the CALLER's password then */
        /* don't ask just give him clearance on the file      */
        if (strcmp(pPassword,UsersData.Password) == 0) {
          RetVal = 0;
          break;
        }

        /* the passwordokay() function will check to see if the last  */
        /* password entered is the same as the current and will avoid */
        /* asking the caller again - otherwise it will ask and check  */
        /* for a match between the password given and that required   */
        Pwrd[0] = 0;
        if (! passwordokay(pPassword,Pwrd,Password,BadPwrd,1)) {
          RetVal = -1;
          break;
        }
      }
      RetVal = 0;
      break;                      /* matched on path and name - so get out   */
    }
  }

  #ifdef PCB152
    /* if a match was found, then get the multiplier, if there is one */
    /* if there isn't a multiplier, then default to a value of 1 */
    if (RetVal >= 0)
      Status.FsecMultiplier = (short) (*pMultiplier == '\0' ? 1 : atoi(pMultiplier));
  #endif

  /* if no match was found, then it's okay to download, set RetVal to 0 */
  if (RetVal == -2)
    RetVal = 0;

  dosfclose(&file);
  return(RetVal);
}
#endif


static bool _NEAR_ LIBENTRY filenameokay(char *Name) {
  int  Len;
  char *p;

  if ((p = strrchr(Name,'\\')) == NULL) {
    if (Name[1] == ':')
       Name += 2;
  } else Name = p + 1;

  if ((Len = strlen(Name)) == 0 || Len > 12)
    return(FALSE);

  if ((p = strchr(Name,'.')) == NULL) {
    if (Len > 8)
      return(FALSE);
  } else {
    if (p > &Name[8])
      return(FALSE);
    if (Len - (p-Name) > 4)
      return(FALSE);
  }
  return(TRUE);
}


/* NOTE:  An if() statement ahead of this function verifies that this  */
/* function is NEVER called if RecSize = 0, no need to worry about a   */
/* division by zero problem                                            */

static bool _NEAR_ LIBENTRY recordsizeokay(long Size, int RecSize) {
  return((bool) (Size == (Size / RecSize) * RecSize));
}


/* This function substitutes an environment variable inside of a filename */

void LIBENTRY substenvinfile(char *FileName, int Len) {
  int  X;
  char *p;
  char *q;
  char Temp[66];

  for (p = FileName; (p = strchr(p,'%')) != NULL; ) {
    if ((q = strchr(p+1,'%')) != NULL) {
      X = (int) (q - p);
      memcpy(Temp,p,X);
      Temp[X] = 0;
      if ((q = getenv(&Temp[1])) != NULL) {
        Temp[X] = '%';
        Temp[X+1] = 0;
        substitute(FileName,Temp,q,Len);
      }
    }
    p++;
  }
}


unsigned char LIBENTRY checkenvfileexist(char *FileName, int Len) {
  substenvinfile(FileName,Len);
  return(fileexist(FileName));
}


/********************************************************************
*
*  Function:  getalternatename()
*
*  Desc    :  runs a filename thru the checkforalternate() logic to see if an
*             alternate file exists
*
*             A recent addition is a check for the RecordSize.  This is used
*             to quickly rule out file that are FOUND which are found to be
*             the wrong size, but it only works on .LST files.  For instance,
*             if someone accidently specifes the BLT List File as "BLT" and
*             then has a BLT20 text file, the size of the BLT20 file, if it is
*             not a multiple of the BLT.LST record size, would allow us to
*             quickly rule it out as a potential "alternate file" for "BLT".
*
*             NOTE:  A better recommendation is for the sysop to name the
*             BLT.LST file "BLT.LST" instead of "BLT" to avoid wrong matches!
*
*  Returns :  returns the found filename or a blank filename if none were found
*/

void LIBENTRY getalternatename(char *FileName, DISPLAYFILETYPE Type, int RecordSize) {
  int    X;
  int    First;
  int    NumFilesFound;
  int    Count;
  char   *p;
  char   Sec[5];
  struct ffblk blk[MAXFILES];
  struct ffblk *pblk;
  char   Search[66];
  char   Masks[ALT_TOTAL][20];
  char   Found[ALT_TOTAL][20];
  #ifdef __OS2__
  int DirHandle = MAKEDIRHANDLE;  /*lint !e569 */
  #endif

  stripright(FileName,' ');
  if (FileName[0] == 0)
    return;

  substenvinfile(FileName,60);  /* substitute environment variable in file name */
                                /* WARNING!  60 characters in buffer is ASSUMED! */

  strupr(FileName);
  if ((p = strrchr(FileName,'\\')) != NULL)
    p++;
  else if (FileName[1] == ':')
    p = &FileName[2];
  else
    p = FileName;

  /* at this point p is pointing to the FILENAME portion of the FILESPEC */
  /* i.e. after the drive letter, after the path, just the filename      */
  /* check to make sure that the FILENAME length is valid                */

  if (! filenameokay(p)) {
    FileName[0] = 0;
    return;
  }


  memset(Masks,0,sizeof(Masks));
  memset(Found,0,sizeof(Found));


  if (NoLanguage || Status.MultiLangExt[0] != '.' || strchr(p,'.') != NULL)
    Type &= ~LANGUAGE;   /*lint !e64 */

  if (NoGraphics || ! Control.GraphicsMode)
    Type &= ~GRAPHICS;   /*lint !e64 */

  if (NoSecurity)
    Type &= ~SECURITY;   /*lint !e64 */
  else
    ascii(Sec,Status.CurSecLevel);

  if ((Type & LANGUAGE) == LANGUAGE) {
    if ((Type & (LANGUAGE|SECURITY|GRAPHICS)) == (LANGUAGE|SECURITY|GRAPHICS)) {
      if (Control.RipMode)
        sprintf(Masks[ALT_SLR],"%s%sR%s",p,Sec,Status.MultiLangExt);
      sprintf(Masks[ALT_SLG],"%s%sG%s",p,Sec,Status.MultiLangExt);
    }
    if ((Type & (LANGUAGE|GRAPHICS)) == (LANGUAGE|GRAPHICS)) {
      if (Control.RipMode)
        sprintf(Masks[ALT_LR],"%sR%s",p,Status.MultiLangExt);
      sprintf(Masks[ALT_LG],"%sG%s",p,Status.MultiLangExt);
    }
    if ((Type & (LANGUAGE|SECURITY)) == (LANGUAGE|SECURITY))
      sprintf(Masks[ALT_SL],"%s%s%s",p,Sec,Status.MultiLangExt);
    sprintf(Masks[ALT_L],"%s%s",p,Status.MultiLangExt);
  }

  #ifndef LIB
  if ((Type & RUNMENU) == RUNMENU) {
    if ((Type & (RUNMENU|SECURITY|GRAPHICS)) == (RUNMENU|SECURITY|GRAPHICS)) {
      if (Control.RipMode)
        sprintf(Masks[ALT_SMR],"%s%sR.MNU",p,Sec);
      sprintf(Masks[ALT_SMG],"%s%sG.MNU",p,Sec);
    }
    if ((Type & (RUNMENU|SECURITY)) == (RUNMENU|SECURITY))
      sprintf(Masks[ALT_SM],"%s%s.MNU",p,Sec);
    if ((Type & (RUNMENU|GRAPHICS)) == (RUNMENU|GRAPHICS)) {
      if (Control.RipMode)
        sprintf(Masks[ALT_MR],"%sR.MNU",p);
      sprintf(Masks[ALT_MG],"%sG.MNU",p);
    }
    sprintf(Masks[ALT_M],"%s.MNU",p);
  }
  #endif

  if ((Type & (SECURITY|GRAPHICS)) == (SECURITY|GRAPHICS)) {
    if (Control.RipMode)
      sprintf(Masks[ALT_SR],"%s%sR",p,Sec);
    sprintf(Masks[ALT_SG],"%s%sG",p,Sec);
  }

  if ((Type & SECURITY) == SECURITY)
    sprintf(Masks[ALT_S],"%s%s",p,Sec);

  if ((Type & GRAPHICS) == GRAPHICS) {
    if (Control.RipMode)
      sprintf(Masks[ALT_R],"%sR",p);
    sprintf(Masks[ALT_G],"%sG",p);
  }

  /* Include the actual filename as the final filename to match.  Also, */
  /* don't forget to remove the trailing period so that "TEST." is      */
  /* properly compared to the found filename "TEST"                     */

  strcpy(Masks[ALT_DEFAULT],p);
  stripright(Masks[ALT_DEFAULT],'.');

  strcpy(Search,FileName);
  strcat(Search,(Type & (LANGUAGE|RUNMENU) ? "*.*" : "*"));

  NumFilesFound = MAXFILES;
  if (dosfindfirst(Search,blk,0x07,&NumFilesFound PDIRHANDLE) != 0) {
    FileName[0] = 0;
    return;
  }

  First = ALT_TOTAL;

  while (1) {
/* loop through each file that was found */
    for (Count = NumFilesFound, pblk = blk; Count > 0; Count--, pblk++) {
/* then loop through each mask to find out which of the files we found */
/* should actually be considered for display purposes */
      for (X = 0; X < First; X++) {
/* if the mask is non-blank then the filename that was found matches */
/* the mask, then we may want to record it */
        if (Masks[X][0] != 0 && stricmp(Masks[X],pblk->ff_name) == 0) {
/* but first, check to see that the record size is okay... */
/* if no record size was requested, then it's okay, otherwise check */
/* the size of the file and make sure that the size of the file is */
/* an even multiple of the record size */
          if (RecordSize == 0 || recordsizeokay(pblk->ff_fsize,RecordSize)) {
/* finally, we're happy with this file that we found, so record */
/* it in our Found[] array */
            strcpy(Found[X],pblk->ff_name);
/* and if the array element we found is closer to the top of the */
/* array than the previous, adjust our limits so that we don't */
/* bother searching for matches that are further down the list */
/* (i.e. the more generic ones are at the bottom, the more specific */
/* ones are at the top, and we want to find the most specific) */
            if (X < First)
              First = X;
          }
          break;
        }
      }
    }

    if (NumFilesFound < MAXFILES)
      break;

    #ifndef __OS2__
/* the DOS version needs to keep the blk parameter in order to find */
/* the next file */
      blk[0] = blk[NumFilesFound-1];
    #endif

/* just in case there are more files to be found, go back and check for */
/* more files right now and loop back up if there is more */
    if (dosfindnext(blk,&NumFilesFound PDIRHANDLE2) == -1)
      break;
  }

  #ifdef __OS2__
    dosclosedirhandle(DirHandle);
  #endif

  if (First != ALT_TOTAL) {
    strcpy(p,Found[First]);
    strupr(FileName);
    return;
  }

/* This code used to have to search for the first one found, now the loop up
   above uses if (X < First) First=X to record the first one found and then
   just returns that one instead of having to search for it.

  for (X = 0; X < ALT_TOTAL; X++) {
    if (Found[X][0] != 0) {
      strcpy(p,Found[X]);
      return;
    }
  }
*/

  FileName[0] = 0;  /* no matches found */
}


/********************************************************************
*
*  Function:  checkforalternatelist()
*
*  Desc    :  Checks to see if an alternate list file exists based on the
*             caller's security level.
*
*  Returns :  FoundName is modified for the filename that was found and 0 is
*             returned.  If the file was not found then FoundName is not
*             modified and a -1 is returned.
*/

#ifndef LIB
int LIBENTRY checkforalternatelist(char *FoundName, char *OriginalName, int RecordSize) {
  char *p;
  char Temp[66];

  if (OriginalName[0] == 0) {
    FoundName[0] = 0;
    return(-1);
  }

  maxstrcpy(Temp,OriginalName,sizeof(Temp));
  substenvinfile(Temp,60);  /* substitute environment variable in file name */
                            /* WARNING!  60 characters in buffer is ASSUMED! */

  if (fileexist(Temp) == 255 || (RecordSize != 0 && ! recordsizeokay(DTA.ff_fsize,RecordSize))) {
    FoundName[0] = 0;
    return(-1);
  }

  if ((p = strchr(Temp,'.')) == NULL) {
    strcpy(FoundName,Temp);
    getalternatename(FoundName,SECURITY|LANGUAGE,RecordSize);
    if (FoundName[0] != 0)
      return(0);
  } else {
    *p = 0;
    sprintf(FoundName,"%s%d.%s",Temp,Status.CurSecLevel,p+1);
    *p = '.';
    if (filenameokay(FoundName) && fileexist(FoundName) != 255)
      return(0);
  }

  strcpy(FoundName,Temp);
  return(0);
}
#endif



/********************************************************************
*
*  Function:  displayfile()
*
*  Desc    :  Displays a file on the screen.
*
*  Returns :  -1 if the display was aborted
*             -2 if the file wasn't found
*              0 in all other cases
*/

int LIBENTRY displayfile(char *Name, DISPLAYFILETYPE Type) {
  #ifndef LIB
  bool     AutoLogoff;
/*bool     InFileListing;*/
  #endif
  int      RetVal;
  int      Okay;
  DOSFILE  InFile;
  char     FileName[66];
  char     Buffer[DISPLAYBUFSIZE];

  stripright(Name,' ');
  if (Name[0] == 0)
    return(-2);

  #ifndef LIB
    if (Type & RUNPPL) {
      maxstrcpy(FileName,Name,sizeof(FileName)-4);
      strcat(FileName,".PPE");
      if (checkenvfileexist(FileName,sizeof(FileName)) != 255) {
        doScript(FileName,NULL,0);     /*lint !e534 */
        return(0);
      }
    }
  #endif

  maxstrcpy(FileName,Name,sizeof(FileName)-8);
  getalternatename(FileName,Type,0);
  if (FileName[0] == 0) {
    /* if it was not a CMDFILE, warn that the file is missing */
    if ((Type & CMDFILE) == 0) {
      sprintf(Buffer,"(%s) is missing!",Name);
      if (Status.UserRecNo == 1) {
        bell();
        printcolor(PCB_RED);
        newline();
        println(Buffer);
      }
      writelog(Buffer,SPACERIGHT);
    }
    return(-2);
  }

  #ifndef LIB
    if ((Type & RUNMENU) && strstr(FileName,".MNU") != NULL) {
      doMenu(FileName,0);    /*lint !e534 */
      return(0);
    }
  #endif

  #ifndef LIB
/* make sure it is not one of our special files (users, users.inf, etc) */
    if (specialfile(FileName)) {
      char Str[80];
      writelog("Error: Attempted to display PCBoard file",SPACERIGHT);
      sprintf(Str,"Error: %s",FileName);
      writelog(Str,SPACERIGHT);
      return(0);
    }

/* Check to see if the file is already open and, if so, avoid going */
/* recursive by getting out *now* instead of displaying the file again */
/* solves the problem of a %FILESPEC that includes itself, or of a */
/* !FILESPEC.PPE that displays the file that launched it. */
    if (fileisopen(FileName))
      return(0);
  #endif

  if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&InFile) == -1)
    return(-2);

  printdefcolor();

  Display.Break = TRUE;

  #ifndef LIB
    if (Type & WELCOME) {
      Display.Break = FALSE;
      startdisplay(FORCENONSTOP);
      clsbox(0,23,79,24,0);
      if (Control.GraphicsMode)
        setlimits(25);
    }

  /* because the MORE? prompt changes capabilities while "in a file listing" */
  /* we will save the value of Status.InFileListing and turn it off and then */
  /* set it back the way it was when we're done to avoid problems            */

/*  InFileListing = Status.InFileListing; */
/*  Status.InFileListing = FALSE; */
    AutoLogoff = FALSE;
  #endif

  RetVal = 0;
  Display.AbortPrintout = FALSE;

  while ((Okay = dosfgets(Buffer,sizeof(Buffer)-1,&InFile)) != -1) {

/* DISABLESUBFILES disables the display of any sub-file specifications */
/* such as %filespec to display a file, !filespec to run a PPE, $filespec */
/* to run a menu and also @hangup@ */
    if ((Type & DISABLESUBFILES) != 0) {
/* simply display the line */
      printxlated(Buffer);
    } else {
      switch (Buffer[0]) {
        case '%': maxstrcpy(FileName,&Buffer[1],sizeof(FileName));
                  stripright(FileName,' ');
                  if (checkenvfileexist(FileName,sizeof(FileName)) != 255) {
                    #ifndef LIB
                    if (! specialfile(FileName))
                    #endif
                      displayfile(FileName,GRAPHICS|SECURITY|LANGUAGE|(Type & WELCOME));
                    continue;
                  }
                  break;
        #ifndef LIB
        case '!': if (Buffer[1] != '|') {
                    if (runscriptwithparams(&Buffer[1]))
                      continue;
                  }
                 printxlated(Buffer);
                  break;
        case '$': maxstrcpy(FileName,&Buffer[1],sizeof(FileName));
                  stripright(FileName,' ');
                  if (checkenvfileexist(FileName,sizeof(FileName)) != 255) {
                    doMenu(FileName,0);  /*lint !e534 */
                    continue;
                  }
                  printxlated(Buffer);
                  break;
        case '@': if (memcmp(&Buffer[1],"HANGUP@",7) == 0) {
                    AutoLogoff = TRUE;
                    continue;
                  } else
                    printxlated(Buffer);
                  break;
        #endif
        default : printxlated(Buffer);
                  break;

      }
    }

    if (Okay == 0)
      newline();

    if (Display.AbortPrintout) {
      RetVal = -1;
      break;
    }
  }
  dosfclose(&InFile);

  #ifndef LIB
    if (Type & WELCOME) {
      if (awherey() > Display.NumScrnLines)
        newline();
      setlimits(Display.NumScrnLines);
      newline();
      turnkbdtimeron();  /* startdisplay(FORCENONSTOP) turned it off */
    }
  #endif


  Display.Break = TRUE;
  #ifndef LIB
/*  Status.InFileListing = InFileListing; */
    redisplaystatusline();
  #endif
  checkdisplaystatus();

  #ifndef LIB
    if (AutoLogoff) {
      Status.AutoLogoff = TRUE;
      loguseroff(NLOGOFF);
    }
  #endif
  return(RetVal);
}


#ifndef LIB
int LIBENTRY displaycmdfile(char *Command) {
  bool     NonStop;
  unsigned LocLen;
  unsigned CmdLen;
  int      RetVal;
  char     FileName[66];

  if (PcbData.CmdLoc[0] == 0 || (CmdLen = strlen(Command)) > 8)
    return(-3);

  LocLen = strlen(PcbData.CmdLoc);
  if (CmdLen + LocLen > sizeof(FileName))
    return(-3);

  memcpy(FileName,PcbData.CmdLoc,LocLen);
  memcpy(&FileName[LocLen],Command,CmdLen+1);

  NonStop = Display.NonStopDuringCmd;
  RetVal = displayfile(FileName,SECURITY|LANGUAGE|GRAPHICS|CMDFILE);
  if (NonStop) {
    startdisplay(FORCENONSTOP);
    Display.NonStopDuringCmd = TRUE;
  }

  return(RetVal);
}


bool LIBENTRY findcmdfile(char *Command, char *FileName) {
  if (PcbData.CmdLoc[0] == 0)
    return(FALSE);

  strcpy(FileName,PcbData.CmdLoc);
  strcat(FileName,Command);
  getalternatename(FileName,SECURITY|LANGUAGE|GRAPHICS|CMDFILE,0);
  return((bool) (FileName[0] != 0));
}


void LIBENTRY flagfilefordownload(char *FileName) {
  int  static Limit;
  int  FileLen;
  int  TaggedLen;
  char *p;

  if (Status.TaggedFileNames == NULL) {
    Limit = 14 * Status.BatchLimit;
    if ((Status.TaggedFileNames = (char *) checkmalloc(Limit,"FLAG FILES")) == NULL)
      return;
    memset(Status.TaggedFileNames,0,Limit);
    Status.NumTaggedFiles = 0;
  }

  if (FileName[0] != 0 && Status.NumTaggedFiles < Status.BatchLimit) {
    stripright(FileName,' ');
    FileLen = strlen(FileName);
    TaggedLen = strlen(Status.TaggedFileNames);
    if (TaggedLen+FileLen+1 < Limit) {
      p = &Status.TaggedFileNames[TaggedLen];
      memcpy(p,FileName,FileLen);
      p += FileLen;
      *p++ = ';';
      *p = 0;
      Status.NumTaggedFiles++;
    }
  }
}
#endif
