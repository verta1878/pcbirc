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


#ifndef DEMO

#include <io.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <help.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMFIELDS     3
#define NUMTABLES     4
#define MAXSECLEVELS 60

static SecTableType  SecTable[MAXSECLEVELS];


/********************************************************************
*
*  Function: readtable()
*
*  Desc    : Reads in the SecTable from disk.  SecTable is empty if the file
*            is not found on the disk.
*
*/

static void near pascal readtable(const int Num) {
  int Table;

  memset(SecTable,0,sizeof(SecTable));  /* empty table */
  if ((Table = dosopen(TableName,OPEN_READ|OPEN_DENYRDWR)) != -1) {
    doslseek(Table,sizeof(SecTable) * (long) Num,0);
    readcheck(Table,SecTable,sizeof(SecTable));  //lint !e534
    dosclose(Table);
  }
}

/********************************************************************
*
*  Function: compsecs()
*
*  Desc    : Compares two elements of SecTable (used to sort the table)
*
*  Calls   : NONE
*
*  Returns :  1 if "Num" of Rec1 is greater than "Num" of Rec2
*             0 if they are equal
*            -1 if "Num" of Rec2 is greater than "Num" of Rec1
*/

static int compsecs(const void *Rec1, const void *Rec2) {
  const SecTableType *A = (SecTableType *) Rec1;
  const SecTableType *B = (SecTableType *) Rec2;

  if (A->Num > B->Num)
    return(1);
  else
    if (A->Num < B->Num)
      return(-1);
    else
      return(0);
}

/********************************************************************
*
*  Function: writetable()
*
*  Desc    : Creates the disk file to hold SecTable.  Sorts all elements in
*            SecTable to ensure that they will be found in the proper order
*            when adjusting security levels.
*
*/

static void near pascal writetable(const int Num) {
  int           Table;
  int           Counter;
  SecTableType *p;
  SecTableType  EmptyTable[MAXSECLEVELS];

  if ((Table = dosopen(TableName,OPEN_WRIT|OPEN_DENYRDWR)) == -1) {
    if ((Table = doscreatecheck(TableName,OPEN_WRIT|OPEN_DENYRDWR,OPEN_NORMAL)) == -1)
      return;
    else {
      memset(EmptyTable,0,sizeof(EmptyTable));
      for (Counter = 0; Counter < NUMTABLES; Counter++)  /* create all empty tables */
        doswrite(Table,EmptyTable,sizeof(EmptyTable));   //lint !e534
    }
  }

  for (Counter = 0, p = SecTable; Counter < MAXSECLEVELS; Counter++, p++)
    if (p->Num == 0 && p->Sec == 0)
      p->Num = 999999999L-Counter;

  qsort(SecTable,MAXSECLEVELS,sizeof(SecTableType),compsecs);

  for (p = SecTable; p < &SecTable[MAXSECLEVELS]; Counter++, p++)
    if (p->Num >= 900000000L)
      p->Num = 0;

  doslseek(Table,sizeof(SecTable) * (long) Num,SEEK_SET);
  writecheck(Table,SecTable,sizeof(SecTable));   //lint !e534
  dosclose(Table);
  return;
}


/********************************************************************
*
*  Function: insertsec()
*
*  Desc    : Adds a blank line to the security table pointed to by *q at offset
*            Num from the beginning.
*/

static void near pascal insertsec(SecTableType *q, int Num) {
  SecTableType *p;

  for (p = q - 1; Num; Num--, p--, q--) {
    *q = *p;
  }
  q->Num = 0;
  q->Sec = 0;
}


/********************************************************************
*
*  Function: deletesec()
*
*  Desc    : Deletes a line from the security table pointed to by *q and places
*            a blank line at the end of the table.
*/

static void near pascal deletesec(SecTableType *q, int Num) {
  SecTableType *p;

  for (p = q + 1; Num; Num--, p++, q++) {
    *q = *p;
  }
  q->Num = 0;
  q->Sec = 0;
}




static void near pascal displaysec(SecTableType *q, int Top, int Bottom) {
  char   Temp[10];
  int    Counter;
  int    Line;

  Line = 7;
  for (Counter = Top; Counter <= Bottom; Counter++, Line++, q++) {
    sprintf(Temp,"%8ld",q->Num);
    fastprint( 4,Line,Temp,Colors[ANSWER]);
    sprintf(Temp,"%6d",q->Sec);
    fastprint(18,Line,Temp,Colors[ANSWER]);
  }
}


/********************************************************************
*
*  Function: readsectable()
*
*  Desc    : Read the Ratio Table from the disk and allow user modification.
*
*/

void pascal readsectable(void) {
  int    Top;
  long   LongTop;
  int    Index;
  int    Column;
  int    Y;
  int    Num;
  bool   NeedToDisplay;
  char   *Head;
  SecTableType *q;

  switch (MenuSelection) {
    case 6 : Num = 0; Head = "Edit Upload/Download FILE Ratio Table"; break;
    case 7 : Num = 2; Head = "Edit Upload/Download BYTE Ratio Table"; break;
    case 8 : Num = 1; Head = "Edit Upload Table";   break;
    case 9 : Num = 3; Head = "Edit Download Table"; break;
    default: return;
  }

  readtable(Num);

  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1,Head);
  fastprint(1,23," ESC=Exit   PgDn=Forw   PgUp=Back   Alt-I=Insert a Line   Alt-D=Delete a Line ",Colors[DESC]);

  switch(Num) {
    case 2:
    case 0: fastprint( 2, 5,"  Ratio       Security",Colors[DISPLAY]);
            fastprint( 2, 6,"ออออออออออ    ออออออออ",Colors[DISPLAY]);
            fastprint(32, 5,"Adjust Securities by Upload/Download Ratio",Colors[DISPLAY]);
            fastprint(28, 6,"อออออออออออออออออออออออออออออออออออออออออออออออออ",Colors[DISPLAY]);
            fastprint(28, 7,"Define each of the upload/download ratios desired",Colors[DISPLAY]);
            fastprint(28, 8,"and attach a security level to each.  To enter a" ,Colors[DISPLAY]);
            fastprint(28, 9,"download to upload ratio use a negative integer." ,Colors[DISPLAY]);
            fastprint(28,10,"A positive integer for upload to download ratios.",Colors[DISPLAY]);
            fastprint(28,12,"Examples:"                                        ,Colors[DISPLAY]);
            fastprint(28,13,"อออออออออ"                                        ,Colors[DISPLAY]);
            fastprint(28,14,"-1000  means  100.0:1 (downloads to uploads)"     ,Colors[DISPLAY]);
            fastprint(28,15,"  -75  means    7.5:1 (downloads to uploads)"     ,Colors[DISPLAY]);
            fastprint(28,16,"    0  means  downloads equal uploads"            ,Colors[DISPLAY]);
            fastprint(28,17,"   50  means    5.0:1 (uploads to downloads)"     ,Colors[DISPLAY]);
            fastprint(28,18,"  155  means   15.5:1 (uploads to downloads)"     ,Colors[DISPLAY]);
            fastprint(28,20,"NOTE: Users with security levels that are NOT"    ,Colors[DISPLAY]);
            fastprint(28,21,"listed in the table will NOT have their security" ,Colors[DISPLAY]);
            fastprint(28,22,"levels adjusted."                                 ,Colors[DISPLAY]);
            break;
    case 1: fastprint( 2, 5," Uploads      Security",Colors[DISPLAY]);
            fastprint( 2, 6,"ออออออออออ    ออออออออ",Colors[DISPLAY]);
            fastprint(35, 5,"Adjust Securities by Number of Uploads",Colors[DISPLAY]);
            fastprint(26, 6,"อออออออออออออออออออออออออออออออออออออออออออออออออออ",Colors[DISPLAY]);
            fastprint(26, 7,"Define each of the upload counts desired and attach",Colors[DISPLAY]);
            fastprint(26, 8,"a security level to each.  A user with a specified" ,Colors[DISPLAY]);
            fastprint(26, 9,"upload count will get that level whether it causes" ,Colors[DISPLAY]);
            fastprint(26,10,"the caller's level to increase or decrease."        ,Colors[DISPLAY]);
            fastprint(26,12,"Example:       Uploads     Security"                ,Colors[DISPLAY]);
            fastprint(26,13,"ออออออออ       ออออออออ    ออออออออ"                ,Colors[DISPLAY]);
            fastprint(26,14,"assuming the       0          10"                   ,Colors[DISPLAY]);
            fastprint(26,15,"table to the      10          25"                   ,Colors[DISPLAY]);
            fastprint(26,16,"right is used     20          30"                   ,Colors[DISPLAY]);
            fastprint(26,17,"                  30          35"                   ,Colors[DISPLAY]);
            fastprint(26,19,"A user with 10-19 u/l's and a level of 20 would be" ,Colors[DISPLAY]);
            fastprint(26,20,"upgraded to 25.  If a user has a level that is NOT" ,Colors[DISPLAY]);
            fastprint(26,21,"listed in the table then his security level will"   ,Colors[DISPLAY]);
            fastprint(26,22,"remain unchanged."                                  ,Colors[DISPLAY]);
            break;
    case 3: fastprint( 2, 5,"Downloads     Security",Colors[DISPLAY]);
            fastprint( 2, 6,"ออออออออออ    ออออออออ",Colors[DISPLAY]);
            fastprint(35, 5,"Adjust Securities by Number of Downloads",Colors[DISPLAY]);
            fastprint(26, 6,"อออออออออออออออออออออออออออออออออออออออออออออออออออ" ,Colors[DISPLAY]);
            fastprint(26, 7,"Define each of the download counts and attach a"     ,Colors[DISPLAY]);
            fastprint(26, 8,"security level to each.  A user with a specified"    ,Colors[DISPLAY]);
            fastprint(26, 9,"download count will get that level whether it causes",Colors[DISPLAY]);
            fastprint(26,10,"the caller's level to increase or decrease."         ,Colors[DISPLAY]);
            fastprint(26,12,"Example:       Downloads   Security"                 ,Colors[DISPLAY]);
            fastprint(26,13,"ออออออออ       อออออออออ   ออออออออ"                 ,Colors[DISPLAY]);
            fastprint(26,14,"assuming the       0          35"                    ,Colors[DISPLAY]);
            fastprint(26,15,"table to the      10          25"                    ,Colors[DISPLAY]);
            fastprint(26,16,"right is used     20          20"                    ,Colors[DISPLAY]);
            fastprint(26,17,"                  30          15"                    ,Colors[DISPLAY]);
            fastprint(26,19,"A user with 20-29 d/l's and a level of 15 would be"  ,Colors[DISPLAY]);
            fastprint(26,20,"upgraded to 20.  If a user has a level that is NOT"  ,Colors[DISPLAY]);
            fastprint(26,21,"listed in the table then his security level will"    ,Colors[DISPLAY]);
            fastprint(26,22,"remain unchanged."                                   ,Colors[DISPLAY]);
            break;
  }


  Top = Index = Column = 0;
  displaysec(&SecTable[Top],Top,Top+14);

  ExitKeyNum[0] = 23;  ExitKeyFlag[0] = FLAG1;  /*  alt-i  */
  ExitKeyNum[1] = 32;  ExitKeyFlag[1] = FLAG2;  /*  alt-d  */

  KeyFlags = 0;
  setcursor(CUR_NORMAL);
  showkeystatus();
  while (KeyFlags != ESC) {
    q = &SecTable[Top + Index];
    Y = Index+7;
    scale(Top+Index,MAXSECLEVELS);
    switch (Column) {
      case 0: inputnum( 4,Y,8,"",&q->Num,vLONG,ADJTABLE);
              break;
      case 1: inputnum(16,Y,8,"",&q->Sec,vINT,ADJTABLE);
              break;
    }

    if (KeyFlags != ESC) {
      switch (KeyFlags) {
        case FLAG1 : insertsec(&SecTable[MAXSECLEVELS-1],MAXSECLEVELS-Top-Index-1);
                     NeedToDisplay = TRUE;
                     break;
        case FLAG2 : deletesec(&SecTable[Top+Index],MAXSECLEVELS-Top-Index-1);
                     NeedToDisplay = TRUE;
                     break;
        default    : LongTop = Top;
                     NeedToDisplay = processcursorkeys(&Index,&LongTop,&Column,MAXSECLEVELS,1,15);
                     Top = (int) LongTop;
                     break;
      }
      if (NeedToDisplay)
        displaysec(&SecTable[Top],Top,Top+14);
    }
  }

  ExitKeyNum[0] = 0;
  ExitKeyNum[1] = 0;

  if (savetext())
    writetable(Num);
}


/********************************************************************
*
*  Function: getlimits()
*
*  Desc    : Scans SecTable and finds the number of entries in the table (Max)
*
*  Returns : the number of actual entries in the table
*/

static int near pascal getentries(void) {
  int Count;
  int Max;
  SecTableType *p;

  for (Count = 1, Max = 0, p = SecTable; Count <= MAXSECLEVELS; Count++, p++)
    if (p->Sec > 0)
      Max = Count;

  return(Max);
}


/********************************************************************
*
*  Function: inrange()
*
*  Desc    : Scans the security level table for a match with the Security level
*            passed to it (SecLevel).
*
*  Returns : TRUE if a match is found, FALSE otherwise.
*/

static bool near pascal inrange(int SecLevel, int Max) {
  SecTableType *p;

  for (p = SecTable; p < &SecTable[Max]; p++)
    if (p->Sec == SecLevel)
      return(TRUE);

  return(FALSE);
}


/********************************************************************
*
*  Function: getsec()
*
*  Desc    : Scans the SecTable to find the security level that corresponds
*            with the Num passed to it by the caller.
*
*  Returns : The new security level as determined by the SecTable.
*/

static int near pascal getsec(const long Num,const int Max) {
  int  Count;
  int  Low;

  for (Count = 0, Low = 0; Count < Max; Count++) {
    if (Num >= SecTable[Count].Num)
      Low = Count + 1;
    else
      break;  /* found */
  }

  if (Low)
    return(SecTable[Low-1].Sec);  /* return table entry (offset 0) */
  else
    return(SecTable[0].Sec);      /* default to the first table entry */
}



/********************************************************************
*
*  Function:
*
*  Desc    :
*
*/

static int  HighSec;     // highest security level to process
static int  LowSec;      // lowest security level to process
static int  Sec;         // change security to value of sec determined by table
static bool Printer;     // TRUE if printer output is requested
static int  MaxEntry;    // number of actual elements in table

static int pascal adjustsub(URead *p) {
  char Tmp[100];
  char Str[81];     /* temp variable to print changed records on the screen */
  int  ReadLevel;   /* security or expired security as read from users file */
  long Num;         /* temp variable holds upload/download ratio            */
  bool Proceed;     /* TRUE if record should be processed for sec change    */

  ReadLevel = (MenuSelection == 1 ? p->ExpSecurityLevel : p->SecurityLevel);
  Proceed   = (MenuSelection <  2 ? ReadLevel >= LowSec && ReadLevel <=HighSec : inrange(ReadLevel,MaxEntry));

  if (Proceed) {
    switch (MenuSelection) {
      case 2 : Num     = ludratio(p->NumUploads,p->NumDownloads);
               Sec     = getsec(Num,MaxEntry);
               Proceed = (Sec != p->SecurityLevel ? TRUE : FALSE);
               break;
      case 3 : Num     = ddratio(basdbletodouble(p->TotUpldBytes),basdbletodouble(p->TotDnldBytes));
               Sec     = getsec(Num,MaxEntry);
               Proceed = (Sec != p->SecurityLevel ? TRUE : FALSE);
               break;
      case 4 : Sec     = getsec(p->NumUploads,MaxEntry);
               Proceed = (Sec != p->SecurityLevel ? TRUE : FALSE);
               break;
      case 5 : Sec     = getsec(p->NumDownloads,MaxEntry);
               Proceed = (Sec != p->SecurityLevel ? TRUE : FALSE);
               break;
    }

    if (Proceed) {
      memcpy(&UsersRead,p,sizeof(URead));
      convertreadtodata();

      switch (MenuSelection) {
        case 0: sprintf(Str,"Old = %3d, New = %3d",UsersData.SecurityLevel,Sec);
                break;
        case 1: sprintf(Str,"Old = %3d, New = %3d",UsersData.ExpSecurityLevel,Sec);
                break;
        case 2: sprintf(Str,"%4dU:%5dD = %6ld, Sec %3d,%3d",
                            UsersData.NumUploads,UsersData.NumDownloads,
                            Num,UsersData.SecurityLevel,Sec); //lint !e644  Num is initialized
                break;
        case 3: sprintf(Str,"%10.0fU:%10.0fD=%6ld, Sec %3d,%3d",
                            UsersData.TotUpldBytes,UsersData.TotDnldBytes,
                            Num,UsersData.SecurityLevel,Sec); //lint !e644  Num is initialized
                break;
        case 4: sprintf(Str,"Uploads = %5d, Sec %3d,%3d",
                            UsersData.NumUploads,UsersData.SecurityLevel,Sec);
                break;
        case 5: sprintf(Str,"Downloads = %5d, Sec %3d,%3d",
                            UsersData.NumDownloads,UsersData.SecurityLevel,Sec);
                break;
      }

      scrollup(5,10,74,19,Colors[OUTBOX]);
      fastprint( 5,19,UsersData.Name,Colors[QUESTION]);
      fastprint(32,19,Str,Colors[HEADING]);
      if (Printer) {
        sprintf(Tmp,"%-25.25s  %s\r\n",UsersData.Name,Str);
        if (dosfputs(Tmp,&prn) == -1)
          return(-1);
        LineNum++;
        if (LineNum > 62)
          if (printheading(FALSE) == -1)
            return(-1);
      }
      if (MenuSelection == 1)
        p->ExpSecurityLevel = Sec;
      else
        p->SecurityLevel = Sec;
    }
  }
  return(0);
}



/********************************************************************
*
*  Function: adjustsecurity()
*
*  Desc    : Adjusts globally the security levels of users in the users file.
*            Supports 5 types of security changes:
*               - Security Level Changes by Range selection
*               - Expired Security Level Changes by Range selection
*               - Security Level Changes by Upload/Download Ratios
*               - Security Level Changes by Upload Count
*               - Security Level Changes by Download Count
*/

static FldType AdjScrn[NUMFIELDS] = {
  {vINT,ALLNUM,ADJSEC,3,6,3,CLEAR,"Change users whose security is greater than or equal to","",&LowSec ,NULL},
  {vINT,ALLNUM,ADJSEC,9,7,3,CLEAR,      "and whose security level is less than or equal to","",&HighSec,NULL},
  {vINT,ALLNUM,ADJSEC,3,9,3,CLEAR,"To a new security level of"                             ,"",&Sec    ,NULL}
};

void pascal adjustsecurity(void) {
  char *Str2;       /* screen heading                                       */
  long Recs;        /* number of records in users file                      */
  int  TableNum;    /* 0 if by ul/dl ratio, 1 if by number of uploads       */
  int  Command;
  char *p;                    /* 1   2   3   4   5   6   7   8   9   10  */
  static char BatchCommands[] = "SECUEXPSFILEBYTEUPLODOWNLOWSHIGHNEWSPRIN";

  Printer = FALSE;

  if (BatchMode) {
    MenuSelection = 0;
    while ((p = parsepaths(NULL)) != NULL) {
      Command = findfour(BatchCommands,p);
      switch(Command) {
        case  0: break;
        case  1:
        case  2:
        case  3:
        case  4:
        case  5:
        case  6: MenuSelection = Command; break;
        case  7: LowSec  = intparam(p);   break;
        case  8: HighSec = intparam(p);   break;
        case  9: Sec     = intparam(p);   break;
        case 10: Printer = TRUE;          break;
        default: return;
      }
    }
    if (MenuSelection != 0)
      MenuSelection--;
    else
      return;
  } else {
    if (MenuSelection < 2) {
      initquest(AdjScrn,NUMFIELDS);
      LowSec = HighSec = Sec = 0;
      clsbox(1,1,78,23,Colors[OUTBOX]);
      fastprint(19,21," Press PGDN to continue, or ESC to exit ",Colors[DESC]);
      Str2 = (MenuSelection == 0 ? "Adjust Security by Range" : "Adjust Expired Security by Range");
      readscrn(AdjScrn,NUMFIELDS-1,0,MainHead1,Str2,1,NOCLEARFLD);  //lint !e534
      freeanswers(AdjScrn,NUMFIELDS);
      if (KeyFlags == ESC)
        return;
    }
    boxcls(6,18,73,22,Colors[MENUBOX],SINGLE);
    inputnum(8,20,1,"Print users whose security is changed (press ESC to abort)",&Printer,vBOOL,SECCHNGE);
    if (KeyFlags == ESC)
      return;
  }

  switch(MenuSelection) {
    case 0 : PrintHeadStr = "Security Level Changes by Range selection";
             Str2 = "Adjust Security by Range";
             break;
    case 1 : PrintHeadStr = "Expired Security Level Changes by Range selection";
             Str2 = "Adjust Expired Security by Range";
             break;
    case 2 : PrintHeadStr = "Security Level Changes by Upload/Download File Ratios";
             Str2 = "Adjust Security by Upload/Download File Ratio";
             TableNum = 0;
             break;
    case 3 : PrintHeadStr = "Security Level Changes by Upload/Download Byte Ratios";
             Str2 = "Adjust Security by Upload/Download Byte Ratio";
             TableNum = 2;
             break;
    case 4 : PrintHeadStr = "Security Level Changes by Upload Count";
             Str2 = "Adjust Security by Upload Count";
             TableNum = 1;
             break;
    case 5 : PrintHeadStr = "Security Level Changes by Download Count";
             Str2 = "Adjust Security by Download Count";
             TableNum = 3;
             break;
    default: return;
  }

  if (Printer)
    if (printheading(TRUE) == -1)
      return;

  if ((Recs = lockusersfile(Str2,NONBUFFERED)) == -1)
    return;

  setcursor(CUR_BLANK);
  boxcls(3,9,75,20,Colors[OUTBOX],SINGLE);

  if (MenuSelection > 1) {
    readtable(TableNum);          //lint !e644 TableNum has been initialized
    MaxEntry = getentries();
  }

  processusersfile(Recs,FALSE,Printer,FALSE,NONBUFFERED,adjustsub);
}


/********************************************************************
*
*  Function: copyexpiredsub()
*
*  Desc    :
*
*  Calls   :
*/

static int pascal copyexpiredsub(URead *p) {
  char Tmp[100];
  char Str[81];
  char TempName[26];
  char TempDate[9];

  if (p->SecurityLevel == p->ExpSecurityLevel)
    return(0);

  if (memcmp(p->RegExpDate,"000000",6) != 0) {
    yymmddtostr(TempDate,p->RegExpDate);
    if (datetojulian(TempDate) < Today) {

      movestr(TempName,p->Name,25);
      sprintf(Str,"Old = %3d, New = %3d, ExpDate = %s",
              p->SecurityLevel,p->ExpSecurityLevel,TempDate);
      scrollup(5,10,74,19,Colors[OUTBOX]);
      fastprint( 5,19,TempName,Colors[QUESTION]);
      fastprint(32,19,Str,Colors[HEADING]);
      if (Printer) {
        sprintf(Tmp,"%s  %s\r\n",TempName,Str);
        if (dosfputs(Tmp,&prn) == -1)
          return(-1);
        LineNum++;
        if (LineNum > 62)
          if (printheading(FALSE) == -1)
            return(-1);
      }
      p->SecurityLevel = p->ExpSecurityLevel;
    }
  }
  return(0);
}


/********************************************************************
*
*  Function: copyexpiredlevel()
*
*  Desc    :
*
*  Calls   :
*/

void pascal copyexpiredlevel(void) {
  long Recs;
  char *p;                    /* 1   */
  static char BatchCommands[] = "PRIN";

  Printer = FALSE;

  if (BatchMode) {
    p = parsepaths(NULL);
    if (findfour(BatchCommands,p) == 1)
      Printer = TRUE;
  } else {
    boxcls(6,18,73,22,Colors[MENUBOX],SINGLE);
    inputnum(8,20,1,"Print users whose security is changed (press ESC to abort)",&Printer,vBOOL,SECCHNGE);
    if (KeyFlags == ESC)
      return;
  }

  if (Printer) {
    PrintHeadStr = "Change Security Levels to Expired Security Levels";
    if (printheading(TRUE) == -1)
      return;
  }

  if ((Recs = lockusersfile("Change to Expired Security",NONBUFFERED)) == -1)
    return;

  setcursor(CUR_BLANK);
  boxcls(3,9,75,20,Colors[OUTBOX],SINGLE);

  processusersfile(Recs,FALSE,Printer,FALSE,NONBUFFERED,copyexpiredsub);
}


/********************************************************************
*
*  Function: initupdnsub()
*
*  Desc    :
*
*  Calls   :
*/

static bool  FileFlg;
static bool  ByteFlg;
static long  ByteEquivalent;
static char  Options;

static int pascal initupdnsub(URead *p) {
  long    UpldBytes;
  long    DnldBytes;

  if (FileFlg) {
    switch (Options) {
      case 1: p->NumUploads   = p->NumDownloads; break;
      case 2: p->NumDownloads = p->NumUploads;   break;
      case 3: p->NumDownloads = 0;
              p->NumUploads   = 0;
              break;
    }
  }
  if (ByteFlg) {
    switch (Options) {
      case 1: memcpy(p->TotUpldBytes,p->TotDnldBytes,sizeof(p->TotDnldBytes)); break;
      case 2: memcpy(p->TotDnldBytes,p->TotUpldBytes,sizeof(p->TotDnldBytes)); break;
      case 3: memset(p->TotUpldBytes,0,sizeof(p->TotDnldBytes));
              memset(p->TotDnldBytes,0,sizeof(p->TotDnldBytes));
              break;
      case 4: UpldBytes = p->NumUploads   * ByteEquivalent;
              DnldBytes = p->NumDownloads * ByteEquivalent;
              longtobasdble((char *) p->TotUpldBytes,UpldBytes);
              longtobasdble((char *) p->TotDnldBytes,DnldBytes);
              break;
    }
  }
  return(0);
}



void pascal initupdncounters(void) {
  long    Recs;
  FldType *Flds;
  char    static OPTIONS[3]  = { 0, '1','4' };

  if ((Flds = (FldType *) mallochk(NUMFIELDS * sizeof(FldType))) == NULL)
    return;

  addquest(Flds,0,vBYTE,INITCOUNTERS+0,OPTIONS, 3,10, 1,"Choose Option (1, 2, 3 or 4 from above)",&Options,NOCLEARFLD,NULL);
  addquest(Flds,1,vBOOL,INITCOUNTERS+1,YESNO  , 3,12, 1,"Adjust Upload / Download FILE Counters" ,&FileFlg,NOCLEARFLD,NULL);
  addquest(Flds,2,vBOOL,INITCOUNTERS+2,YESNO  , 3,13, 1,"Adjust Upload / Download BYTE Counters" ,&ByteFlg,NOCLEARFLD,NULL);

  Options = 1;
  FileFlg = FALSE;
  ByteFlg = FALSE;
  clsbox(1,1,78,23,Colors[OUTBOX]);
  fastprint( 3, 5,"1) Make fields EQUAL (based on download field)"                ,Colors[DISPLAY]);
  fastprint( 3, 6,"2) Make fields EQUAL (based on upload field)"                  ,Colors[DISPLAY]);
  fastprint( 3, 7,"3) Initialize both upload & download fields to ZERO"           ,Colors[DISPLAY]);
  fastprint( 3, 8,"4) Initialize both BYTE counters (based on Up:Down FILE ratio)",Colors[DISPLAY]);
  fastprint(19,21," Press PGDN to continue, or ESC to exit ",Colors[DESC]);
  readscrn(Flds,NUMFIELDS-1,0,MainHead1,"Initialize Upload/Download Counters",1,NOCLEARFLD);  //lint !e534
  freescrn(Flds,NUMFIELDS-1);

  if (Options < 1 || Options > 4 || KeyFlags == ESC)
    return;

  if (Options == 4) {
    FileFlg = FALSE;
    ByteFlg = TRUE;
    ByteEquivalent = 10000;
    boxcls(2,15,77,23,Colors[OUTBOX],SINGLE);
    fastprint(5,16,"Both of the BYTE counters will be changed according to the file ratio.",Colors[DISPLAY]);
    fastprint(5,17,"For example, if you type 10000 below and a user has 10 downloads and"  ,Colors[DISPLAY]);
    fastprint(5,18,"only 2 uploads then the download BYTES will be set to 100,000 and the" ,Colors[DISPLAY]);
    fastprint(5,19,"upload BYTES will be set to 20,000."                                   ,Colors[DISPLAY]);
    inputnum(19,21,7,"BYTE equivalent for each file",&ByteEquivalent,vLONG,0);
    if (KeyFlags == ESC)
      return;
  } else if (!FileFlg && !ByteFlg)
    return;

  if ((Recs = lockusersfile("Initialize Upload/Download Counters",NONBUFFERED)) == -1)
    return;

  processusersfile(Recs,FALSE,FALSE,FALSE,NONBUFFERED,initupdnsub);
}
#endif
