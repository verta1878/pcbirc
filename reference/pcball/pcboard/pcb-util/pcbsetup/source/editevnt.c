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


#include <ctype.h>
#include <dir.h>
#include <dos.h>
#include <country.h>
#include <help.h>
#include <misc.h>
#include <dosfunc.h>
#include <pcb.h>
#include <process.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "setup.h"
#include "event.h"
#include "edit.hpp"

#ifdef DEBUG
#include <memcheck.h>
#endif


#define CPYSTR(dest,src,count) { memcpy((dest),(src),(count)); dest[count] = 0; }

extern  char pascal editexit(char *);
extern  int pascal  checktime(char *, int);

enum { F2 = NEXTKEY, ALTV };


#define LEFTSIDE              1
#define RIGHTSIDE            78
#define TOPLINE               6
#define BOTTOM                3

typedef struct  {
    char        Active[2];
    char        Os2[2];
    EventMode   Mode;
    unsigned    Node;
    char        Days[8];
    char        BeginTime[TIMELENGTH + 1];
    char        EndTime[TIMELENGTH + 1];
    char        Date[DATELENGTH + 1];
    char        BatchFile[FNLENGTH + 1];
} EventType2;

typedef struct  {
    unsigned    Version;                /*** PCBoard version number ********/
    unsigned    NumOfNodes;             /*** Number of nodes supported *****/
    unsigned    SizeOfRec;              /*** Size of each node record ******/
} usernethdrtype;

static  char        FileList[64][20];
static  char        EventModeMask[] = { 4, 'E', 'S', 'F', 'M' };
static  char        TimeMask[] = { 4, 0, '0', '9', ':' };
static  char        DateMask[] = { 6, 0, '0', '9', '-', 'X', '_' }; /* position 6 is hard coded below! */
static  char        FileMask[] = {11, 0,'#',')', '!','-', 0,'0','9', 0,'A','Z' };
static  unsigned    *LastDates;
static  unsigned    NodeNum = 1;
static  int         MaxEvents = MAX_EVENTS; /*** maximum events == 32 ******/
static  int         MaxNodes = 0;           /*** none defined **************/
static  char        ActiveEventList[MAXACTIVEEVENTSIZE];


class editeventclass : public editclass {
  public:
    editeventclass(char *Title, unsigned Size, int Top, int Bottom, int Left, int Right, int Columns, int NumKeys) :
         editclass(Title,Size,Top,Bottom,Left,Right,Columns,NumKeys) {
           memset(ActiveEventList, 0, ACTIVEEVENTSIZE);
         }

    void pascal createfile(char *Name);
    void pascal newrecord(void *Rec) { editclass::newrecord(Rec); }
    void pascal loadrecords(DOSFILE *File);
    void pascal saverecords(DOSFILE *File);
    void pascal showheaders(void);
    void pascal showfooters(void);
    void pascal displayrecord(long RecNum, int LineNum, void *Rec);
    void pascal editrecord(void *Rec);
    void pascal handlekeys(void *Rec);
    int  pascal scanforbadevents(void);
};


static void near pascal getlastdate(char *Str, int EventNum) {
  strcpy(Str,juliantodate(LastDates[(unsigned) LASTDATERECPOS(EventNum,NodeNum)]));
  countrydate(Str);
}

static void near pascal putlastdate(char *Str, int EventNum) {
  unsigned Num;

  if (strcmp(Str,"00-00-00") == 0)
    Num = 0;
  else
    Num = datetojulian(Str);

  LastDates[(unsigned) LASTDATERECPOS(EventNum,NodeNum)] = Num;
}


/***************************************************************************
 *** Validate a date string ************************************************/
int pascal datevalid(char *Date) {
  int Num;

  if (Date[0] == 0) {
    strcpy(Date,"        ");
    return(TRUE);
  }

  if (strlen(Date) != 8)
    return(FALSE);

  if (memcmp(Date,"        ",DATELENGTH) == 0 || memcmp(Date,"00-00-00",DATELENGTH) == 0)
    return(TRUE);

  if ((Date[2] != '-') || (Date[5] != '-'))
    return(FALSE);

  if (isdigit(Date[0]) && isdigit(Date[1])) {
    Num = atoi(&Date[0]);
    if (memcmp(&Date[0],"00",2) != 0 && (Num < 1 || Num > 12))
      return(FALSE);
  } else {
    if (memcmp(&Date[0],"XX",2) != 0)
      return(FALSE);
  }

  if (isdigit(Date[3]) && isdigit(Date[4])) {
    Num = atoi(&Date[3]);
    if (memcmp(&Date[3],"00",2) != 0 && (Num < 1 || Num > 31))
      return(FALSE);
  } else {
    if (memcmp(&Date[3],"XX",2) != 0)
      return(FALSE);
  }

  if (isdigit(Date[6]) && isdigit(Date[7])) {
    return(TRUE);
  } else {
    if (memcmp(&Date[6],"XX",2) != 0 && memcmp(&Date[6],"XX",2) != 0)
      return(FALSE);
  }

  return(TRUE);
}


/***************************************************************************
 *** Validate a time string ************************************************/
static  int near pascal     timevalid(char *Time)
{
    char    Temp[TIMELENGTH + 1];
    int     Hour,
            Min;

    if (strlen(Time)) {
        if (memcmp(Time, "     ", TIMELENGTH) == 0)
            return TRUE;
        if (checktime(Time, 0) == -1)
            return FALSE;

        strcpy(Temp, Time);
        Hour = atoi(&Temp[0]);
        Min  = atoi(&Temp[3]);
/*
        The following code used to prevent times from 23:59 to 00:01 (i.e.
        it made only 00:02 thru 23:58 valid).  This has been commented out to
        test the effect and see if the problem with 23:59 or 00:00 still exist.

        if ((Hour == 23 && Min == 59) ||
            (Hour == 24 && Min == 00) ||
            (Hour == 00 && Min <= 01))
            return FALSE;
*/
        if (Hour >= 24 || Min >= 60)
          return(FALSE);
    }

    return TRUE;
}


/***************************************************************************
 ***************************************************************************/
int pascal fileselect(int X, int Y, int NumItems, char List[64][20], int Width, int Heighth) {
    char            Key;
    char            Extended;
    char            Abort = FALSE;
    char            Done = FALSE;
    short           Select = 0;
    short           Top = 0;
    int             i;
    char            Blank[80];
    savescrntype    ScreenBuf;

    savescreen(&ScreenBuf);
    setcursor(CUR_BLANK);

    memset(Blank,' ',Width-1);
    Blank[Width-1] = 0;

    boxcls(X, Y, X + Width, Y + Heighth + 1, Colors[OUTBOX], SINGLE);
    while (Abort == FALSE && Done == FALSE) {
        for (i = 0; i < NumItems && i < Heighth; i++) {
            fastprint(X + 1, Y + i + 1, Blank        , (i == Select) ? Colors[MENUBAR] : Colors[MENUSELECT]);
            fastprint(X + 1, Y + i + 1, List[Top + i], (i == Select) ? Colors[MENUBAR] : Colors[MENUSELECT]);
        }
        Key = inkey(&Extended,CLOCK);
        if (Extended) {
            switch (Key) {
                case 60 : Done = TRUE;  break;
                case 80 :
                        if (Select == Heighth-1) {
                            if (Top < (NumItems - Heighth))
                                Top++;
                        } else {
                            if (Select < NumItems-1)
                                Select++;
                        }
                    break;
                case 72 :
                        if (Select == 0) {
                            if (Top)
                                Top--;
                        } else {
                            Select--;
                        }
                    break;
            }
        } else {
            switch (Key) {
                case 27 :
                        Abort = TRUE;
                    break;
                case 13 :
                        Done = TRUE;
                    break;
            }
        }
    }

    setcursor(CUR_NORMAL);
    restorescreen(&ScreenBuf);

    return (Abort == TRUE) ? -1 : (Top + Select);
}




static int near pascal readusernetfile(void) {
  int             MaxNodes;
  DOSFILE         File;
  usernethdrtype  unHeader;

  /***************************************************************************
   Read the USERNET.XXX file to determine how many nodes this version of
   PCBoard supports.
   ***************************************************************************/
  if (PcbData.Network == TRUE) {
    if (fileexist(PcbData.NetFile) == 255) {
      beep();
      memset(&MsgData, 0, sizeof(MsgData));
      MsgData.Save    = TRUE;
      MsgData.AutoBox = TRUE;
      MsgData.Line1   = 18;
      MsgData.Msg1    = "Run PCBOARD to create a USERNET.XXX file first";
      MsgData.Color1  = Colors[HEADING];
      showmessage();
      return(-1);
    }
    if (dosfopen(PcbData.NetFile, OPEN_READ|OPEN_DENYNONE,&File) != -1)
      if (dosfread(&unHeader, sizeof(usernethdrtype), &File) > 0) {
        MaxNodes = unHeader.NumOfNodes;         /*** assign MaxNodes ***/
      dosfclose(&File);
    }
  } else {
    MaxNodes = 2;  // default to 2 for the lowest PCBoard version
  }

  return(MaxNodes);
}


void pascal editeventclass::createfile(char *Name) {
  long    loop;
  char    Data = 0x00;
  DOSFILE File;

  if (dosfopen(Name, OPEN_WRIT|OPEN_CREATE|OPEN_DENYRDWR, &File) != -1) {
    for (loop = 0; loop < TOTALEVENTSIZE; loop++)
      dosfwrite(&Data, sizeof(char), &File);

    dosfclose(&File);
  }
}


void pascal editeventclass::loadrecords(DOSFILE *File) {
  int        EventCount;
  int        DayCount;
  EventType  Buffer;
  EventType2 Rec;

  if (dosfread(&MaxEvents,MAXEVENTSIZE,File) != MAXEVENTSIZE)
    return;

  if (dosfread(ActiveEventList,ACTIVEEVENTSIZE,File) != ACTIVEEVENTSIZE)
    return;

  for (EventCount = 0; EventCount < MAX_EVENTS && dosfread(&Buffer,sizeof(EventType),File) == sizeof(EventType); EventCount++) {
    newrecord(&Rec);
    strcpy(Rec.Active, ((isset(ActiveEventList, EventCount)) ? "Y" : "N"));
    strcpy(Rec.Os2,(Buffer.Os2 ? "Y" : "N"));
    Rec.Mode = Buffer.Mode;
    for (DayCount = 0; DayCount < 7; DayCount++)
      Rec.Days[DayCount] = (isset(&Buffer.Days, DayCount) ? 'Y' : 'N');
    Rec.Days[DayCount] = 0;
    CPYSTR(Rec.BeginTime, Buffer.BeginTime, sizeof(Buffer.BeginTime));
    CPYSTR(Rec.EndTime, Buffer.EndTime, sizeof(Buffer.EndTime));
    CPYSTR(Rec.Date, Buffer.Date, sizeof(Buffer.Date));
    CPYSTR(Rec.BatchFile, Buffer.BatchFile, sizeof(Buffer.BatchFile));
    Rec.Node = Buffer.Node;
    addrecord(&Rec);
  }

  if (MaxNodes) {
    dosfseek(File,LASTDATEOFFSET,SEEK_SET);
    dosfread(LastDates,LASTDATESIZE,File);
  }
}


void pascal editeventclass::saverecords(DOSFILE *File) {
  int         DayCount;
  int         X;
  EventType2 *p;
  EventType   Buffer;

  MaxEvents = MAX_EVENTS;
  if (dosfwrite(&MaxEvents,MAXEVENTSIZE,File) == -1)
    return;

   memset(ActiveEventList, 0, ACTIVEEVENTSIZE);
   if (dosfwrite(ActiveEventList,ACTIVEEVENTSIZE,File) == -1)
     return;

  for (X = 1; X <= MAX_EVENTS; X++) {
    memset(&Buffer,0,sizeof(Buffer));
    if (X <= TotalRecs) {
      p = (EventType2 *) getrecord(X);
      if (p->Active[0] == 'Y')
        setbit(ActiveEventList,X-1);
      Buffer.Os2 = (p->Os2[0] == 'Y' ? TRUE : FALSE);
      Buffer.Mode = p->Mode;
      for (DayCount = 0; DayCount < 7; DayCount++)
        if (p->Days[DayCount] == 'Y')
          setbit(&Buffer.Days, DayCount);
      memcpy(Buffer.BeginTime, p->BeginTime, strlen(p->BeginTime));
      memcpy(Buffer.EndTime,   p->EndTime,   strlen(p->EndTime));
      memcpy(Buffer.Date,      p->Date,      strlen(p->Date));
      memcpy(Buffer.BatchFile, p->BatchFile, strlen(p->BatchFile));
      Buffer.Node = p->Node;
    }
    if (dosfwrite(&Buffer,sizeof(Buffer),File) == -1)
      break;
  }

  if (MaxNodes) {
    dosfwrite(LastDates,LASTDATESIZE,File);
    dosftrunc(File,-1);   // truncate the file at that size
  }

  dosrewind(File);
  dosfseek(File,ACTIVEEVENTOFFSET, SEEK_SET);
  dosfwrite(&ActiveEventList, ACTIVEEVENTSIZE, File);
}



static void near pascal warnevent(int EventCount, char *Msg) {
  char Str[80];

  sprintf(Str,"Error in Event #%d",EventCount);
  beep();
  memset(&MsgData, 0, sizeof(MsgData));
  MsgData.Save    = TRUE;
  MsgData.AutoBox = TRUE;
  MsgData.Line1   = 16;
  MsgData.Msg1    = Str;
  MsgData.Color1  = Colors[HEADING];
  MsgData.Line2   = 18;
  MsgData.Msg2    = Msg;
  MsgData.Color2  = Colors[HEADING];
  showmessage();
}


int pascal editeventclass::scanforbadevents(void) {
  int          Num;
  int          CountMatching;
  int          X;
  int          Last;
  char         Hour[3];
  char         Min[3];
  EventType2  *p;
  char        *q;
  struct ffblk ffblk;
  char         FileName[66];

  if (TotalRecs >= MAX_EVENTS)
    Last = MAX_EVENTS;
  else
    Last = (int) TotalRecs;

  MaxEvents = MAX_EVENTS;
  for (X = 1; X <= Last; X++) {
    p = (EventType2 *) getrecord(X);

    if (p->Active[0] == 'Y') {
      if (p->BatchFile[0] <= ' ' || strnchr(p->BatchFile,'.',sizeof(p->BatchFile)) != NULL) {
        warnevent(X,"Event is active but Batch File is named improperly");
        return(-1);
      }

      Hour[2] = 0;
      Min[2]  = 0;
      Hour[0] = p->BeginTime[0];
      Hour[1] = p->BeginTime[1];
      Min[0]  = p->BeginTime[3];
      Min[1]  = p->BeginTime[4];

      if (p->BeginTime[0] <= ' ' || p->BeginTime[2] != ':' ||
          (Num = atoi(Hour)) < 0 || Num > 23 ||
          (Num = atoi(Min)) < 0 || Num > 59) {
        warnevent(X,"Begin Time is improperly specified");
        return(-1);
      }

      Hour[0] = p->EndTime[0];
      Hour[1] = p->EndTime[1];
      Min[0]  = p->EndTime[3];
      Min[1]  = p->EndTime[4];
      if (p->EndTime[0] <= ' ' || p->EndTime[2] != ':' ||
          (Num = atoi(Hour)) < 0 || Num > 23 ||
          (Num = atoi(Min)) < 0 || Num > 59) {
        warnevent(X,"End Time is improperly specified");
        return(-1);
      }

      if (memcmp(p->BeginTime,p->EndTime,TIMELENGTH) > 0) {
        warnevent(X,"Begin and End Times cannot cross midnight");
        return(-1);
      }

      stripright(PcbData.EventFiles,' ');
      sprintf(FileName,"%s%-8.8s",PcbData.EventFiles,p->BatchFile);
      stripright(FileName,' ');
      strcat(FileName,".*");

      CountMatching = 0;
      if (findfirst(FileName,&ffblk,0) != -1) {
        do {
          if ((q = strchr(ffblk.ff_name,'.')) == NULL || atoi(q+1) > 0)
            CountMatching++;
        } while (findnext(&ffblk) != -1);
      }

      if (CountMatching == 0)
        warnevent(X,"Batch File not found.  Be sure to create one.");
    }
  }

  return(0);
}



void pascal editeventclass::displayrecord(long RecNum, int LineNum, void *Rec) {
  EventType2 *p = (EventType2 *) Rec;
  char        Date1[9];
  char        Date2[9];
  char        Temp[10];

  sprintf(Temp,"%3ld)",RecNum);
  fastprint(LEFTSIDE     , LineNum, Temp, Colors[ANSWER]);

  strcpy(Date1,p->Date);
  if (Date1[0] > ' ')
    countrydate(Date1);
  getlastdate(Date2,(int) RecNum-1);

  fastprint(LEFTSIDE +  8, LineNum, p->Active, Colors[ANSWER]);
  fastprint(LEFTSIDE + 13, LineNum, p->Os2, Colors[ANSWER]);
  fastprint(LEFTSIDE + 19, LineNum, (char *)&(p->Mode), Colors[ANSWER]);
  fastprint(LEFTSIDE + 23, LineNum, p->BatchFile, Colors[ANSWER]);
  fastprint(LEFTSIDE + 33, LineNum, p->BeginTime, Colors[ANSWER]);
  fastprint(LEFTSIDE + 40, LineNum, p->EndTime, Colors[ANSWER]);
  fastprint(LEFTSIDE + 47, LineNum, p->Days, Colors[ANSWER]);
  fastprint(LEFTSIDE + 56, LineNum, Date1, Colors[ANSWER]);
  fastprint(LEFTSIDE + 66, LineNum, Date2, Colors[ANSWER]);
}


static void near pascal shownodenum(void) {
  char Str[40];

  if (PcbData.Network) {
    sprintf(Str,"Node: %3d",NodeNum);
    fastprint(1,1,Str,Colors[DISPLAY]);
  }
}


void pascal editeventclass::showheaders(void) {
  shownodenum();
//fastprint(LEFTSIDE + 7, TOPLINE - 3, "          Batch     Begin  End                         Last"  , Colors[DISPLAY]);
//fastprint(LEFTSIDE + 7, TOPLINE - 2, "Act  Mod  File      Time   Time   SMTWTFS    Date      Date"  , Colors[DISPLAY]);
//fastprint(LEFTSIDE + 7, TOPLINE - 1, "อออ  อออ  ออออออออ  อออออ  อออออ  อออออออ  ออออออออ  ออออออออ", Colors[DISPLAY]);
  fastprint(LEFTSIDE + 7, TOPLINE - 3, "                Batch     Begin  End                         Last"  , Colors[DISPLAY]);
  fastprint(LEFTSIDE + 7, TOPLINE - 2, "Act  OS/2  Mod  File      Time   Time   SMTWTFS    Date      Date"  , Colors[DISPLAY]);
  fastprint(LEFTSIDE + 7, TOPLINE - 1, "อออ  ออออ  อออ  ออออออออ  อออออ  อออออ  อออออออ  ออออออออ  ออออออออ", Colors[DISPLAY]);
}


void pascal editeventclass::showfooters(void) {
  fastprint(1,Scrn_BottomRow-2, "  Modes: E = Expedited   S = Sliding   F = Fido   M = Mail Hour               ", Colors[DESC]);
  editclass::showfooters();
}


static void near pascal calleditor(char *FileName, char Type) {
  if (Type == 'F')
    editverblist(FileName);
  else
    editor(FileName, FALSE);
}


void pascal editeventclass::handlekeys(void *Rec) {
  int            CurrentFile;
  int            NumFiles;
  int            Num;
  EventType2    *p = (EventType2 *) Rec;
  unsigned      *q;
  char           FileName[65];
  struct ffblk   ffblk;

  switch (KeyFlags) {
    case ALTI: if (CurRec < MAX_EVENTS) {
                 // First, handle the date array ourselves
                 q = &LastDates[(unsigned) LASTDATERECPOS((int) CurRec,1)];
                 Num = MAX_EVENTS - (int) CurRec - 1;
                 memmove(q + MaxNodes, q, Num * LASTDATERECSIZE);
                 memset(q, 0, LASTDATERECSIZE);
                 // then fall thru and let the base class handle the main data
                 editclass::handlekeys(Rec);
               }
               break;

    case ALTR: if (CurRec < MAX_EVENTS) {
                 // First, handle the date array ourselves
                 q = &LastDates[(unsigned) LASTDATERECPOS((int) CurRec,1)];
                 Num = MAX_EVENTS - (int) CurRec - 1;
                 memmove(q + MaxNodes, q, Num * LASTDATERECSIZE);
                 memcpy(q,q - MaxNodes,LASTDATERECSIZE);
//               memset(q, 0, LASTDATERECSIZE);
                 // then fall thru and let the base class handle the main data
                 editclass::handlekeys(Rec);
               }
               break;

    case ALTD:
               // First, handle the date array ourselves
               q = &LastDates[(unsigned) LASTDATERECPOS((int) CurRec-1,1)];
               if (CurRec < MAX_EVENTS) {
                 Num = MAX_EVENTS - (int) CurRec - 1;
                 memmove(q, q + MaxNodes, Num * LASTDATERECSIZE);
                 q += (Num * MaxNodes);
               }
               memset(q, 0, LASTDATERECSIZE);
               // then fall thru and let the base class handle the main data
               editclass::handlekeys(Rec);
               break;

    case ALTV: if (PcbData.Network) {
                 boxcls(23,Scrn_BottomRow-8,57,Scrn_BottomRow-4,Colors[OUTBOX],SINGLE);
                 while (1) {
                   inputnum(25,Scrn_BottomRow-6,3,"Enter Node Number to View",&NodeNum,vINT,0);
                   if (NodeNum >= 1 && NodeNum <= MaxNodes)
                     break;
                   beep();
                 }
                 NeedToDisplay = TRUE;
                 shownodenum();
               }
               break;

    case F2  : /*** Build a list of files... ******************/
               stripright(PcbData.EventFiles, ' ');
               stripright(p->BatchFile, ' ');
               if (p->BatchFile[0] == 0)
                 break;

               sprintf(FileName, "%s%s.*", PcbData.EventFiles, p->BatchFile);
               if (findfirst(FileName, &ffblk, 0) != -1) {
                 NumFiles = 0;
                 strcpy(FileList[NumFiles++], ffblk.ff_name);
                 while (findnext(&ffblk) != -1) {
                   /*** add to list ***********************/
                   strcpy(FileList[NumFiles++], ffblk.ff_name);
                 }
                 if (NumFiles == 1) {
                   sprintf(FileName, "%s%s", PcbData.EventFiles, FileList[0]);
                   calleditor(FileName,p->Mode);
                 } else {
                   if ((CurrentFile = fileselect(17, (LineNum > 11) ? LineNum - 7 : LineNum, NumFiles, FileList,13,8)) != -1) {
                     sprintf(FileName, "%s%s",PcbData.EventFiles, FileList[CurrentFile]);
                     calleditor(FileName,p->Mode);
                   }
                 }
               } else {
                 if (p->Node)
                   sprintf(FileName, "%s%s.%03d",PcbData.EventFiles, p->BatchFile, p->Node);
                 else
                   sprintf(FileName, "%s%s",PcbData.EventFiles, p->BatchFile);
                 if (FileName[0] != 0)
                   calleditor(FileName,p->Mode);
               }
               KeyFlags = NOTHING;
               break;
    case ESC : if (scanforbadevents())
                 KeyFlags = NOTHING;
               else
                 ExitEditor = TRUE;
    default  : editclass::handlekeys(Rec); break;
  }
}


void pascal editeventclass::editrecord(void *Rec) {
  int         ValTime;
  int         ValDate;
  EventType2 *p = (EventType2 *) Rec;

  switch (Column) {
    case  0: inputstr(LEFTSIDE + 8,LineNum, 1, "", p->Active, p->Active, YESNO, INPUT_CAPS, EVENTSETUP + 0);
             break;
    case  1: inputstr(LEFTSIDE + 13,LineNum, 1, "", p->Os2, p->Os2, YESNO, INPUT_CAPS, EVENTSETUP + 1);
             break;
    case  2: inputstr(LEFTSIDE + 19,LineNum, 1, "", (char *)&p->Mode, (char *)&p->Mode, EventModeMask, INPUT_CAPS, EVENTSETUP + 2);
             break;
    case  3: inputstr(LEFTSIDE + 23,LineNum, FNLENGTH - 2, "", p->BatchFile, p->BatchFile, FileMask, INPUT_CAPS, EVENTSETUP + 3);
             break;
    case  4: do {
               inputstr(LEFTSIDE + 33,LineNum, 5, "", p->BeginTime, p->BeginTime, TimeMask, INPUT_CAPS, EVENTSETUP + 4);
             } while ((ValTime = timevalid(p->BeginTime)) == FALSE && KeyFlags != ESC);
             if (ValTime == FALSE)
               memset(p->BeginTime, ' ', TIMELENGTH);
             break;
    case  5: do {
               inputstr(LEFTSIDE + 40,LineNum, 5, "", p->EndTime, p->EndTime, TimeMask, INPUT_CAPS, EVENTSETUP + 5);
             } while ((ValTime = timevalid(p->EndTime)) == FALSE && KeyFlags != ESC);
             if (ValTime == FALSE)
               memset(p->EndTime, ' ', TIMELENGTH);
             break;
    case  6: inputstr(LEFTSIDE+47,LineNum,7,"",p->Days,p->Days,YESNO,INPUT_CAPS,EVENTSETUP + 7);
             break;
/*
    case  6:
    case  7:
    case  8:
    case  9:
    case 10:
    case 11:
    case 12: inputstr(LEFTSIDE + 47 + (Column - 6),LineNum, 1, "", p->Days[Column - 6], p->Days[Column - 6], YESNO, INPUT_CAPS, EVENTSETUP + 7);
             break;
*/
    case  7: do {
               if (p->Date[0] > ' ')
                 countrydate(p->Date);
               inputstr(LEFTSIDE + 56,LineNum, 8, "", p->Date, p->Date, DateMask, INPUT_CAPS, EVENTSETUP + 6);
               if (p->Date[0] > ' ')
                 uncountrydate(p->Date);
               ValDate = datevalid(p->Date);
             } while (! ValDate && KeyFlags != ESC);
             if (! ValDate)
               memset(p->Date, ' ', DATELENGTH);
             break;
    case  8: /*** Edit the LastDate field ***************/
             if (MaxNodes) {
               char Str[9];
               char SaveStr[9];

               getlastdate(SaveStr,(int) CurRec-1);
               do {
                 getlastdate(Str,(int) CurRec-1);
                 inputstr(LEFTSIDE + 66,LineNum, 8, "", Str, Str, DateMask, INPUT_CAPS, EVENTSETUP + 8);
                 if (Str[0] > ' ')
                   uncountrydate(Str);
               } while ((ValDate = datevalid(Str)) == FALSE && KeyFlags != ESC);

               if (ValDate == FALSE)
                 strcpy(Str,"00-00-00");
               putlastdate(Str,(int) CurRec-1);

               if (strcmp(SaveStr,Str) != 0)
                 Changed = TRUE;
             }
             break;
  }
}



void pascal editeventlist(char *Name) {
  editeventclass Event("Event Editor",sizeof(EventType2),TOPLINE,BOTTOM,LEFTSIDE,RIGHTSIDE,8,2);

  MaxEvents = MAX_EVENTS;
  if (PcbData.Network)
    NodeNum = PcbData.NodeNum;

  if ((MaxNodes = readusernetfile()) == -1)
    return;

  if ((LastDates = (unsigned *) mallochk(LASTDATESIZE)) == NULL)
    return;

  memset(LastDates,0,LASTDATESIZE);

  if (fileexist(Name) == 255)
    Event.createfile(Name);

  DateMask[6] = Country.DateSep[0];

  if (Event.load(Name) != -1) {
    Event.addexitkey(60,F2,"F2=Edit");
    Event.addexitkey(47,ALTV,"AltV=View");
    Event.edit();
  }
  free(LastDates);
}
