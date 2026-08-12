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
#include <dir.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <validate.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <help.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "idx.hpp"
#include "vmstruct.h"
#include "rules.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

bool    Aborted;
int     LineCounter;
bool    CheckDupes;
bool    NonStop;

static char        Perform;
static char        MainFileName[60];
static char        AppendName[60];
static char        SetFileDate;
static bool        AppendFileOpen;
static bool        UpdateAppendIdx;
static DOSFILE     AppendFile;
static diridxclass AppendIdx;

enum {APND_SUCCESS=0,APND_ERRCREATE,APND_ERRBACKUP,APND_ERROPEN,APND_ERRWRITE};

extern struct ffblk DTA;   /* really declared as "char DTA[45]" in exist.c */


static void near pascal showname(char *Name) {
  clsbox(2,3,78,3,Colors[QUESTION]);
  fastprint( 2,3,"Processing",Colors[QUESTION]);
  fastprint(14,3,Name,Colors[QUESTION]);
}

static void near pascal checkstatus(char *DskPath) {
  char Ans[2];

  clscolor(Colors[OUTBOX]);
  generalscreen("Process DIR Files","");
  fastprint(23,22," press ESC to abort processing ",Colors[DESC]);

  if (BatchMode) {
    Perform = Work.Perform;
    CheckDupes = FALSE;
    SetFileDate = Work.SetFileDate;
    KeyFlags = NOTHING;
  } else {
    if (DskPath[0] != 0) {
      fastprint( 7,5,"Y)es, perform operation on files in",Colors[DISPLAY]);
      fastprint(43,5,DskPath,Colors[ANSWER]);
    } else {
      fastprint(7,5,"----- no subdirectory attached to this DIR file",Colors[DISPLAY]);
    }
    fastprint(7,6,"N)o,  modify text DIR listings only leaving physical files alone",Colors[DISPLAY]);
    fastprint(7,7,"A)ll, perform operation on files found in ALL download paths" ,Colors[DISPLAY]);
    Ans[1] = 0;

    Ans[0] = Work.Perform;
    inputstr(4, 9,1,"Perform operation (Move/Rename/Delete) on marked files",Ans,Ans,YNAQ,INPUT_CAPS,DEFAULTS+3);
    Perform = Ans[0];
    if (KeyFlags == ESC)
      return;

    Ans[0] = (Work.CheckDupes ? 'Y' : 'N');
    inputstr(4,11,1,"Check for duplicate files in each text DIR listing processed",Ans,Ans,YESNO,INPUT_CAPS,DEFAULTS+4);
    CheckDupes = (Ans[0] == 'Y');

    SetFileDate = Work.SetFileDate;
    if (SetFileDate == 'A') {
      Ans[0] = 'N';
      inputstr(4,13,1,"Set FILE DATE when moving files (T=Today,O=Oldest,R=Recent,N=No)",Ans,Ans,TONAR,INPUT_CAPS,DEFAULTS+8);
      SetFileDate = Ans[0];
    }
  }
}


void pascal printscroll(char *Str, int ColorNum) {
  char Ch;

  LineCounter++;
  if ((! NonStop) && LineCounter >= (19 - 5 + 1)) {
    fastprintmove(2,19,"press any key to continue",Colors[HELPKEY]);
    if (LineCounter != 100)
      fastprintmove(28,19,"(C = continuous)",Colors[HELPKEY]);
    Ch = inkey(&Ch,CLOCK);
    switch (toupper(Ch)) {
      case 'C': NonStop = TRUE; break;
      case 27 : Aborted = TRUE; break;
    }
    LineCounter = 0;
    clsbox(2,19,45,19,Colors[DISPLAY]);
  }
  fastprint(2,19,Str,ColorNum);
  scrollup(1,5,78,19,Colors[DISPLAY]);
  gotoxy(2,19);
}


bool pascal alreadyinlist(char *FileName) {
  bool Found;
  int  Len;
  char Temp[80];

  dosrewind(&DosChangedList);
  Found = FALSE;

  while (dosfgets(Temp,80,&DosChangedList) != -1) {
    if (Temp[0] != '-' && Temp[0] != '=' && Temp[0] > 0) {
      Len = strlen(Temp)-2;
      if (Temp[Len-1] == '*')
        Temp[Len-6] = 0;
      else
        Temp[Len] = 0;
      if (strcmp(Temp,FileName) == 0) {
        Found = TRUE;
        break;
      }
    }
  }
  dosfseek(&DosChangedList,0,SEEK_END);
  return(Found);
}


static char * near pascal getfilename(char *Path) {
  char *p;

  if ((p = strrchr(Path,'\\')) == NULL)
    if ((p = strchr(Path,':')) == NULL)
      return(Path);
  return(++p);
}


int pascal backupfile(char *FileName, char SortType) {
  static char *ErrorMsg[5] = {"Error Opening Source File",
                              "Error Opening Destination File",
                              "Insufficient Memory for File Copy",
                              "Error Reading Source File",
                              "Error Writing Source File"};
  bool  Error;
  char *p;
  char  Name[66];
  char  Temp[80];

  if (! alreadyinlist(FileName)) {
    sprintf(Temp,"Backing up: %s",FileName);
    printscroll(Temp,Colors[QUESTION]);
    if (checkexistence(Work.BackupPath,CHECK_DIR,NULL,FALSE) != 0)
      return(-1);

    strcpy(Name,getfilename(FileName));
    if ((p = strrchr(Name,'.')) != NULL)   // remove the extension
      *p = 0;

    strcpy(Temp,Work.BackupPath);
    strcat(Temp,Name);
    strcat(Temp,".BAK");
    Error = copyfile(FileName,Temp,FALSE);

    switch (Error) {
      case COPYFILE_ERROPENSRCE:
      case COPYFILE_ERROPENDEST:
      case COPYFILE_ERRMEMALLOC:
      case COPYFILE_ERRREADSRCE:
      case COPYFILE_ERRWRITDEST:

                memset(&MsgData,0,sizeof(MsgData));
                MsgData.AutoBox   = TRUE;
                MsgData.Save      = TRUE;
                MsgData.Color1    = Colors[HEADING];
                MsgData.Msg1      = ErrorMsg[Error-1];
                MsgData.Line1     = 18;
                MsgData.Color2    = HEADING;

                if (Error == COPYFILE_ERROPENSRCE || Error == COPYFILE_ERRREADSRCE) {
                  MsgData.Msg2  = FileName;
                  MsgData.Line2 = 18;
                  MsgData.Line1 = 16;
                }
                if (Error == COPYFILE_ERROPENDEST || Error == COPYFILE_ERRWRITDEST) {
                  MsgData.Msg2  = Temp;
                  MsgData.Line2 = 18;
                  MsgData.Line1 = 16;
                }
                showmessage();
                break;

      default:  sprintf(Temp,"%s %d\r\n",FileName,SortType);
                Error = dosfputs(Temp,&DosChangedList);
                break;
    }
    return(Error);
  }
  return(0);
}


static int near pascal writechildren(DOSFILE *file, long ChildPos, bool NonUploadDir) {
  childtype *child;

  while (ChildPos != VM_INVALID_POS) {
    child = (childtype *) VMRecordGetByPos(&Children,ChildPos);
    if (NonUploadDir && file == &AppendFile) {
      if (Work.RemoveUpldBy && strstr(child->Str,"Uploaded by: ") != NULL)
        goto nextchild;
      if (Work.RemoveDates && strstr(child->Str,"Files:") != NULL && strstr(child->Str,"Oldest:") != NULL && strstr(child->Str,"Newest:") != NULL)
        goto nextchild;
    }
    if (dosfputs(child->Str,file) == -1 || dosfputs("\r\n",file) == -1)
      return(-1);

nextchild:
    ChildPos = child->NextChildPos;
  }
  return(0);
}


int pascal writeparent(DOSFILE *file, diridxclass &Idx, parenttype *parent, bool UpdateIdx, bool UploadDir) {
  char Temp[10];
  char Buffer[100];

  switch (parent->Line) {
    case DIRLINE : sprintf(Buffer,"%-12.12s%9ld  %s  %s\r\n",
                           parent->Fields.NName,
                           parent->Fields.FSize,
                           dtoc(parent->Fields.NDate,Temp),
                           parent->Fields.FDesc);
                   if (UpdateIdx) {
                     if (Idx.fill(parent->Fields.NName,Buffer,dosfseek(file,0,SEEK_CUR)))
                       Idx.write();
                   }
                   if (dosfputs(Buffer,file) == -1)
                     return(-1);

                   // don't strip Uploaded By out of upload directories
                   return(writechildren(file,parent->Fields.FirstChildPos,! UploadDir));
    case TEXTLINE: sprintf(Buffer,"%s\r\n",parent->Str);
                   return(dosfputs(Buffer,file));
    default      : return(0);
  }
}


/********************************************************************
*
*  Function:  append()
*
*  Desc    :  appends a description to a new file
*
*  Returns :  0 = line successfully appended to target file
*             1 = can't create target file
*             2 = can't backup target file
*             3 = can't open target file
*             4 = can't write line
*/


static int near pascal append(char *Name, parenttype *parent, char SortType, bool UploadDir) {
  bool AlreadyOpen;

  // check for EMPTY name ("") which indicates a secondary line being moved
  if (Name[0] != 0) {
    AlreadyOpen = (AppendFileOpen && (strcmp(AppendName,Name) == 0));
    strcpy(AppendName,Name);
  } else {
    AlreadyOpen = AppendFileOpen;
  }

  if (! AlreadyOpen) {          /* if it is a NEW file then enter here */
    if (AppendFileOpen) {       /* close append file if it's open      */
      dosfclose(&AppendFile);
      if (UpdateAppendIdx) {
        AppendIdx.close();
        UpdateAppendIdx = FALSE;
      }
    }

    if (fileexist(AppendName) == 255) {
      if (dosfopen(AppendName,OPEN_RDWR|OPEN_DENYWRIT,&AppendFile) == -1)
        return(APND_ERRCREATE);
      dossetbuf(&AppendFile,4096);
      UpdateAppendIdx = AppendIdx.create(AppendName);
    } else {
      if (backupfile(AppendName,SortType) != 0)
        return(APND_ERRBACKUP);
      if (dosfopen(AppendName,OPEN_RDWR|OPEN_DENYWRIT|OPEN_APPEND,&AppendFile) == -1)
        return(APND_ERROPEN);
      UpdateAppendIdx = AppendIdx.append(AppendName);
    }
    dossetbuf(&AppendFile,4096);
  }

  AppendFileOpen = TRUE;
  if (writeparent(&AppendFile,AppendIdx,parent,UpdateAppendIdx,UploadDir) == -1)
    return(APND_ERRWRITE);
  return(APND_SUCCESS);
}


static int near pascal removefile(char *FileName, bool ZeroIt) {
  int code;

  if (fileexist(FileName) != 255)   /* does the file exist?  */
    if (unlink(FileName) == -1)     /* yes, try to delete it */
       return(-1);

  if (ZeroIt) {   /* create zero byte file */
    if ((code = doscreate(FileName,OPEN_WRIT,OPEN_NORMAL)) == -1)
      return(-1);
    dosclose(code);
  }
  return(0);
}


static void near pascal removedeletedfiles(void) {
  bool Found;
  bool ZeroIt;
  int  ColorNum;
  char Name[66];
  char Buffer[100];

  if (openchangeread() == -1)
    return;

  Found = FALSE;
  while (dosfgets(Buffer,80,&DosChangedList) != -1) {
    if (Buffer[0] == '-') {
      Found = TRUE;
      break;
    }
  }

  if (Found) {
    if (Work.EraseFiles != 'A') {
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.X1        = 6;
      MsgData.X2        = 71;
      MsgData.Y1        = 19;
      MsgData.Y2        = 23;
      MsgData.Msg1      = "There are files on disk marked for deletion";
      MsgData.Line1     = 20;
      MsgData.Color1    = Colors[HEADING];
      MsgData.Msg2      = "The entries in the DIR files have already been removed";
      MsgData.Line2     = 21;
      MsgData.Color2    = Colors[HEADING];
      MsgData.Quest     = "Do you want to delete them (PCBFiler cannot UnErase files)";
      MsgData.QuestLine = 22;
      MsgData.Answer[0] = Work.EraseFiles;
      MsgData.Answer[1] = 0;
      MsgData.Mask      = YESNO;
      MsgData.Save      = TRUE;
      showmessage();
    } else {
      MsgData.Answer[0] = 'Y';
    }

    if (MsgData.Answer[0] == 'Y' && KeyFlags != ESC) {
      dosrewind(&DosChangedList);
      while (dosfgets(Buffer,80,&DosChangedList) != -1) {
        if (Buffer[0] == '-') {
          strcpy(Name,&Buffer[3]);
          ZeroIt = (Buffer[1] == '+') && Work.ZeroByte;
          if (removefile(Name,ZeroIt) == -1) {
            sprintf(Buffer,"Unchanged : unable to delete %s",Name);
            ColorNum = Colors[HEADING];
          } else {
            ColorNum = Colors[ANSWER];
            sprintf(Buffer,(ZeroIt ? "Removed   : %s and left a 0-byte file" : "Deleted   : %s"),Name);
          }
          printscroll(Buffer,ColorNum);
        }
      }
    }
  }
  dosfclose(&DosChangedList);
}


static void near pascal undo(void) {
  bool CopyOnly;
  char *p;
  char Buffer[100];
  char Name[100];
  char Temp[100];

  if (openchangeread() == -1)
    return;

  clsbox(2,3,78,3,Colors[QUESTION]);
  fastprint( 2,3,"Restoring files from backup",Colors[QUESTION]);

  while (dosfgets(Buffer,80,&DosChangedList) != -1) {
    switch (Buffer[0]) {
      case '-': break;                 /* do nothing ... can't UnErase files */
      case '=': CopyOnly = (Buffer[1] == '+');             /* move file back */
                strcpy(Name,&Buffer[3]);
                p = strchr(Name,' ');
                *p = 0;
                p += 4;
                memmove(Buffer,p,strlen(p)+1);
                if (CopyOnly) {
                  sprintf(Temp,"Removed   : %s - copy remains at %s",Buffer,Name);
                  unlink(Buffer);
                } else {
                  sprintf(Temp,"Move Back : %s from %s",Name,Buffer);
                  checkmovefile(Buffer,Name);  //lint !e534
                }
                printscroll(Temp,Colors[DISPLAY]);
                break;
      default : Buffer[strlen(Buffer)-2] = 0;  /* restore backup of DIR file */
                strcpy(Name,Work.BackupPath);
                strcat(Name,getfilename(Buffer));
                strcat(Name,".BAK");
                sprintf(Temp,"Restoring : %s from backup",Buffer);
                printscroll(Temp,Colors[DISPLAY]);
                copyfile(Name,Buffer,FALSE);  //lint !e534
                break;
    }
  }
  dosfclose(&DosChangedList);
}



void pascal undolastchange(void) {
  char Temp[80];

  LineCounter = 0;
  NonStop     = Work.NonStop;

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox   = TRUE;
  MsgData.Save      = TRUE;
  MsgData.Quest     = "Do you want to UNDO the last changes made";
  MsgData.QuestLine = 20;
  MsgData.Answer[0] = 'N';
  MsgData.Answer[1] = 0;
  MsgData.Mask      = YESNO;
  showmessage();

  if (MsgData.Answer[0] == 'Y' && KeyFlags != ESC) {
    memset(Temp,'Ä',78); Temp[78] = 0;
    clsbox(1,2,78,23,Colors[OUTBOX]);
    fastprint(1, 4,Temp,Colors[DISPLAY]);
    fastprint(1,20,Temp,Colors[DISPLAY]);
    undo();
    LineCounter = 99;
    printscroll("",0);
  }
}


/*
static void near pascal secondaries(void) {
  unsigned ParentNumber;
  int      Count;
  DirType  OneLine;
  VirType  huge *p;
  char     ParentName[13];

  if (LoScanNum != 65535U) {
    /* search backwards from the LoScanNum to find a DIRLINE entry */

    ParentNumber = 0;
    for (Count = LoScanNum, p = &Virtual[LoScanNum]; Count >= 0; Count--, p--) {
      getvirtualrec(p,&OneLine);
      if (OneLine.Line == DIRLINE) {
        strcpy(ParentName,OneLine.Rec.Flds.NName);
        ParentNumber = OneLine.Rec.Flds.ParentNumber;
        break;
      }
    }

    /* scan forwards from the found DIRLINE entry until the last modified */
    /* entry .. adjust any TEXTLINE's found that should be DUPELINE's     */

    if (ParentNumber != 0) {
      for (Count++, p++; Count <= HiScanNum; Count++, p++) {
        getvirtualrec(p,&OneLine);
        switch (OneLine.Line) {
          case DUPELINE: ParentNumber = OneLine.Rec.SortFlds.ParentNumber;
                         break;
          case DIRLINE : strcpy(ParentName,OneLine.Rec.Flds.NName);
                         ParentNumber = OneLine.Rec.Flds.ParentNumber;
                         break;
          case TEXTLINE: if (ParentNumber != 0) {
                           if (strchr(OneLine.Rec.Str,'|') == NULL) {
                             ParentNumber = 0;
                           } else {
                             OneLine.Line = DUPELINE;
                             strcpy(OneLine.Rec.SortFlds.ParentName,ParentName);
                             OneLine.Rec.SortFlds.ParentNumber = ParentNumber;
                             updvirtualrec(p,&OneLine);
                           }
                         }
                         break;
        }
      }
    }

  }
}
*/


static int near pascal changefiledate(int NewDate) {
  bool   Okay;
  int    File;
  struct ftime TimeDate;
  char   *p;

  if ((File = dosopen(SrchDskPath,OPEN_RDWR|OPEN_DENYNONE)) != -1) {
    getftime(File,&TimeDate);
    p = (char*) &TimeDate;
    memcpy(p+2,&NewDate,2);
    Okay = setftime(File,&TimeDate);
    dosclose(File);
    return(Okay);
  } else return(-1);
}



static int near pascal scandates(unsigned CnfNum, unsigned DirNum) {
  char               Status;
  bool               ProcessToday;
  bool               ProcessLater;
  unsigned           Today;
  parenttype        *p;
  parentsonlytype   *parentsonly;
  long               RecNum;
  char               Date[9];
  DOSFILE            In;
  DOSFILE            Out;
  char               Str[80];

  if (Work.ProcName[0] == 0)
    return(-1);

  if (dosfopen(Work.ProcName,OPEN_WRIT|OPEN_DENYNONE,&Out) == -1)
    return(-1);

  if (dosfopen(Work.ProcName,OPEN_READ|OPEN_DENYNONE,&In) == -1) {
    dosfclose(&Out);
    return(-1);
  }

  boxcls(17,Scrn_BottomRow-8,62,Scrn_BottomRow-4,Colors[OUTBOX],SINGLE);
  fastprintmove(19,Scrn_BottomRow-6,"Saving Auto Move File ... Please Wait",Colors[HEADING]);

  // first, scrunch up the move file by removing any entries that already
  // exist for this conf+dir

  dossetbuf(&Out,16384);
  dossetbuf(&In , 8192);
  while (dosfgets(Str,sizeof(Str),&In) >= 0) {
    // if the Conf or Dir numbers are different, write it out, otherwise,
    // just skip over it
    if (atoi(Str) != CnfNum || atoi(&Str[6]) != DirNum) {
      dosfputs(Str,&Out);     //lint !e534
      dosfputs("\r\n",&Out);  //lint !e534
    }
  }

  dosfclose(&In);

  ProcessLater = FALSE;
  ProcessToday = FALSE;
  Today = getjuliandate();

  for (RecNum = 1; RecNum <= TotalParents; RecNum++) {
    parentsonly = (parentsonlytype *) VMRecordGetByIndex(&ParentsOnly,RecNum,NULL);
    p = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);

    if (p->Line != DIRLINE)
      continue;

    if (p->Fields.Keep == MOVE || p->Fields.Keep == COPYONLY) {
      dtoc(p->Fields.NDate,Date);
      Status = (p->Fields.Keep == MOVE ? 'M' : 'C');
      if (p->Fields.Days == 0)  {
        ProcessToday = TRUE;
//      continue;
      } else if (datetojulian(Date) + p->Fields.Days <= Today) {
        ProcessToday = TRUE;
//      continue;
      } else {
        // then change it back to KEEP so that it doesn't get processed today
        p->Fields.Keep = KEEP;
        ProcessLater = TRUE;
      }


//////// addendum to below:  let's go ahead and write ALL move files out then
//////// let the loader remove the ones that have actually been processed.


      // if we got this far, then the file is not being processed today, so
      // record the file that is moving, where to, and when
      if (writeonemove(&Out,CnfNum,DirNum,p->Fields.FName,p->Fields.NCnfNum,p->Fields.NDirNum,Date,p->Fields.Days,Status) == -1)
        break;
    }
  }

  dosftrunc(&Out,-1);  //lint !e534 truncate the file at this point
  dosfclose(&Out);

  if (ProcessToday)
    return(2);
  else if (ProcessLater)
    return(1);

  return(0);
}


static void near pascal removefinished(void) {
  bool     Found;
  char    *p;
  DOSFILE  In;
  DOSFILE  Out;
  char     MoveName[13];
  char     MoveStr[80];
  char     DoneStr[80];

  if (Work.ProcName[0] == 0)
    return;

  if (fileexist("changed.lst") == 255 || openchangeread() == -1)
    return;

  if (dosfopen(Work.ProcName,OPEN_WRIT|OPEN_DENYNONE,&Out) == -1) {
    dosfclose(&DosChangedList);
    return;
  }

  if (dosfopen(Work.ProcName,OPEN_READ|OPEN_DENYNONE,&In) == -1) {
    dosfclose(&Out);
    dosfclose(&DosChangedList);
    return;
  }

  dossetbuf(&Out,16384);
  dossetbuf(&In , 8192);
  while (dosfgets(MoveStr,sizeof(MoveStr),&In) >= 0) {
    memcpy(MoveName,&MoveStr[12],12);
    MoveName[12] = 0;
    stripright(MoveName,' ');

    dosrewind(&DosChangedList);
    for (Found = FALSE; dosfgets(DoneStr,sizeof(DoneStr),&DosChangedList) >= 0;) {
      // see if the entry is a MOVE (or COPY) entry, if not skip it
      if (DoneStr[0] != '=')
        continue;

      // find the filename portion of the line (if can't be found, skip it)
      if ((p = strrchr(DoneStr,'\\')) == NULL)
        if ((p = strrchr(DoneStr,' ')) == NULL)
          continue;

      // p+1 points to the actual filename (skipping over the backslash)
      // compare the filename in changed.lst against the filename in moves
      // if equal then set flag as found and break out of the changed-loop-scan
      if (strcmp(p+1,MoveName) == 0) {
        Found = TRUE;
        break;
      }
    }

    // if the filename was found, then avoid copying the line
    if (Found)
      continue;

    // copy the line
    dosfputs(MoveStr,&Out);     //lint !e534
    dosfputs("\r\n",&Out);      //lint !e534
  }

  dosfclose(&In);
  dosflush(&Out);             //           flush the buffers out to disk
  doswrite(Out.handle,"",0);  //lint !e534 truncate the file at this point
  dosfclose(&Out);
  dosfclose(&DosChangedList);
}


static bool near pascal isuploaddir(int CnfNum, int DirNum) {
  static int  SaveCnfNum;
  static int  SaveDirNum;
  static bool SaveStatus;
  pcbconftype Conf;

  if (DirNum == 0)
    return(TRUE);

  if (CnfNum == SaveCnfNum && DirNum == SaveDirNum)
    return(SaveStatus);

  SaveCnfNum = CnfNum;
  SaveDirNum = DirNum;

  getconfrecord(CnfNum,&Conf);
  if (DirNum > numrandrecords(Conf.DirNameLoc,sizeof(DirListType2)))
    SaveStatus = TRUE;
  else
    SaveStatus = FALSE;

  return(SaveStatus);
}


int pascal savedir(DirListType *SaveRec, char *PathListName, int CnfNum, int DirNum, saveflagstype Flags) {
  static  char    *CopyCodes[6] = { "" , "Can't open srce", "Can't open dest", "Insuf mem", "Read error", "Write error" };
  static  char    *AppendCodes[6] = { "", "same srce & targ", "error creating", "error backing up", "error opening", "error writing"};
  bool             UpdateMainIdx;
  bool             WriteItem;
  bool             WriteDefault;
  bool             CannotMoveFile;
  bool             NeedToPerformAndNotListOnly;
  bool             SrceAndDestSame;
  bool             SetDate;
  bool             CopyOrMove;
  bool             UploadDir;
  bool             AppendUploadDir;
  int              Code;
  int              ColorNum;
  unsigned         Oldest;
  unsigned         Newest;
  long             Count;
  parentsonlytype *parentsonly;
  parenttype      *parent;
  char            *q;
  DOSFILE          MainFile;
  DirListType      Rec;
  char             Str[128];
  char             Temp[128];
  char             Temp2[128];
  diridxclass      MainIdx;

  memset(&MsgData,0,sizeof(MsgData));
  MsgData.AutoBox = TRUE;
  MsgData.Save    = TRUE;
  MsgData.Color1  = Colors[HEADING];
  MsgData.Color2  = Colors[HEADING];

  if (Flags & SCANMOVES)
    Code = scandates(CnfNum,DirNum);

  // BatchMode is used to automatically move files, no user interaction
  // is available in this mode (hey, da sysop may be sleeping!)
  if (! BatchMode) {
    // ForcedUpdate means there are changes that need to be processed immediately
    if (! ForcedUpdate) {
      MsgData.Line1     = Scrn_BottomRow-6;
      MsgData.QuestLine = Scrn_BottomRow-4;
      MsgData.Answer[0] = 'N';
      MsgData.Answer[1] = 0;
      MsgData.Mask      = YESNO;
      // Code 2 means there are files that need to be processed today
      if (Code == 2) {
        MsgData.Msg1  = "There are files that are marked for processing today.";
        MsgData.Quest = "Do you want to process them now";
        showmessage();
        if (KeyFlags == ESC || MsgData.Answer[0] != 'Y')
          return(-1);
      } else if (Code == 1) {
        // no forced update, nothing to do today, see if sysop wants out now
        MsgData.Msg1  = "The Post-Processing Information has been saved.";
        MsgData.Quest = "There is nothing to do today, do you still want to process now";
        showmessage();
        if (KeyFlags == ESC || MsgData.Answer[0] != 'Y')
          return(-1);
      }
    }
  }

  WriteDefault    = ! (Flags & OFFLINEDIR);
  LineCounter     = 0;
  AppendFileOpen  = FALSE;
  UpdateAppendIdx = FALSE;
  UpdateMainIdx   = FALSE;
  Aborted         = FALSE;
  NonStop         = Work.NonStop;
  UploadDir       = isuploaddir(CnfNum,DirNum);

  checkstatus(SaveRec->DskPath);
  if (KeyFlags != ESC) {
    memset(Temp,'Ä',78); Temp[78] = 0;
    clsbox(1,2,78,23,Colors[OUTBOX]);
    fastprint(1, 4,Temp,Colors[DISPLAY]);
    fastprint(1,20,Temp,Colors[DISPLAY]);
    showname(SaveRec->DirPath);
    fastprint(9,22," press ESC to abort processing and leave files as they were ",Colors[DESC]);

    if (openchangewrite() == -1)
      return(-1);

    if (! (Flags & OFFLINEDIR)) {
        // the data is still in memory, so sort it in memory first
        if (SaveRec->SortType != 0)
          sortinplace(SaveRec->SortType,FALSE);  //lint !e534

      // To save time, we pre-sorted what was already in memory prior to
      // saving it out to disk.  Therefore, we will tell backupfile() that
      // the sort type is 0 (NoSort) to avoid re-loading, re-sorting and
      // re-saving the file that we are saving right now.
      if (backupfile(SaveRec->DirPath,0) != 0) {
        dosfclose(&DosChangedList);
        return(-1);
      }

      strcpy(MainFileName,SaveRec->DirPath);
      if (dosfopen(MainFileName,OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&MainFile) == -1) {
        dosfclose(&DosChangedList);
        return(-1);
      }
      dossetbuf(&MainFile,8192);
      UpdateMainIdx = MainIdx.create(MainFileName);

      sprintf(Temp,"Updating  : %s",MainFileName);
      printscroll(Temp,Colors[QUESTION]);
    } else {
      MainFileName[0] = 0;
    }


//  secondaries();   Is this function still necessary?


    for (Count = 1; ! Aborted && Count <= TotalParents; Count++) {
      parentsonly = (parentsonlytype *) VMRecordGetByIndex(&ParentsOnly,Count,NULL);
      parent = (parenttype *) VMRecordGetByPos(&Parents,parentsonly->Pos);

      WriteItem   = WriteDefault;

      /***  if it's a TEXTLINE then just go write it out now  ***/

      if (parent->Line == TEXTLINE)
        goto process;

      /***  assume DIRLINE from here on down  ***/

      /***  this variable will be used several times below  ***/
      NeedToPerformAndNotListOnly = (Perform == 'A' || (parent->Fields.Exist != LISTING && Perform != 'N'));

      /***  Remove DESCRIPTION by not writing it & if the file exists and  ***/
      /***  Perform != NO then mark file for deletion later.               ***/

      if (parent->Fields.Keep == DELETE) {
        WriteItem   = FALSE;
        sprintf(Temp,"Deleted   : %-12.12s from DIR file",parent->Fields.FName);
        printscroll(Temp,Colors[ANSWER]);
        if (NeedToPerformAndNotListOnly) {
          if (verifyexist(Perform,SaveRec->DskPath,PathListName,parent->Fields.FName) != 255)
            FilesChanged = TRUE;
            sprintf(Str,"-- %s\r\n",SrchDskPath);
            if (dosfputs(Str,&DosChangedList) == -1) {
              Aborted = TRUE;
              break;
            }
        }
        goto process;
      }

      /***  remove file from disk leaving 0-byte file in its place  ***/

      if (parent->Fields.Keep == REMOVE) {
        if (NeedToPerformAndNotListOnly)
          if (verifyexist(Perform,SaveRec->DskPath,PathListName,parent->Fields.FName) != 255) {
            sprintf(Str,"-+ %s\r\n",SrchDskPath);
            if (dosfputs(Str,&DosChangedList) == -1) {
              Aborted = TRUE;
              break;
            }
            parent->Fields.FSize = 0;
          }
      }

      /***  rename file if filenames are different  ***/

      if (stricmp(parent->Fields.FName,parent->Fields.NName) != 0) {
        if (NeedToPerformAndNotListOnly) {
          if (verifyexist(Perform,SaveRec->DskPath,PathListName,parent->Fields.FName) != 255) {
            strcpy(Temp,SrchDskPath);
            if ((q = strstr(Temp,parent->Fields.FName)) != NULL) {
              strcpy(q,parent->Fields.NName);
              // in batch mode, don't let it attempt to move the file if the
              // target already exists in order to avoid getting stuck with a
              // prompt to overwrite the file.
              if (BatchMode) {
                if (fileexist(Temp) != 255)
                  Code = -1;
                else
                  Code = movefile(SrchDskPath,Temp);
              } else {
                if (Code == 0)
                  Code = checkmovefile(SrchDskPath,Temp);
              }

              if (Code == -1) {
                sprintf(Temp,"Unchanged : unable to rename %s to %s",parent->Fields.FName,parent->Fields.NName);
                strcpy(parent->Fields.NName,parent->Fields.FName);
                ColorNum = Colors[HEADING];
              } else {
                FilesChanged = TRUE;
                sprintf(Str,"== %s to %s\r\n",SrchDskPath,Temp);
                if (dosfputs(Str,&DosChangedList) == -1) {
                  Aborted = TRUE;
                  break;
                }
                sprintf(Temp,"Renamed   : %-12.12s to %s",parent->Fields.FName,parent->Fields.NName);
                ColorNum = Colors[ANSWER];
              }
              printscroll(Temp,ColorNum);
            }
          }
        }
      }

      /***  change date stamp on file  ***/

      CopyOrMove = (parent->Fields.Keep == MOVE || parent->Fields.Keep == COPYONLY);
      SetDate    = (SetFileDate != 'N' && CopyOrMove);

      if (parent->Fields.FDate != parent->Fields.NDate || SetDate) {
        if (NeedToPerformAndNotListOnly) {
          if (verifyexist(Perform,SaveRec->DskPath,PathListName,parent->Fields.NName) != 255) {
            if (SetDate) {
              switch (SetFileDate) {
                case 'T':  parent->Fields.NDate = TodaysDate;
                           break;
                case 'O':
                case 'R':  Oldest = Newest = parent->Fields.NDate;
                           if (getdates(SrchDskPath,&Oldest,&Newest) != -1)
                             parent->Fields.NDate = (SetFileDate == 'O' ? Oldest : Newest);
                           break;
              }
            }
            if (changefiledate(parent->Fields.NDate) != -1) {
              sprintf(Temp,"Changed   : %-12.12s file date",parent->Fields.NName);
              ColorNum = Colors[ANSWER];
            } else {
              sprintf(Temp,"Unchanged : unable to change date on %s",parent->Fields.NName);
              ColorNum = Colors[HEADING];
            }
            printscroll(Temp,ColorNum);
          }
        }
      }

      /***  default to NOT writing the line if the description is blank  ***/

      if (parent->Fields.Exist == DIRECT && parent->Fields.FDesc[0] == 0 && parent->Fields.FirstChildPos == VM_INVALID_POS)
        WriteItem = FALSE;

      /***  move file and/or description to a new location  ***/

      if (CopyOrMove && (parent->Fields.NCnfNum != CnfNum || parent->Fields.NDirNum != DirNum)) {
        AppendUploadDir = isuploaddir(parent->Fields.NCnfNum,parent->Fields.NDirNum);
        finddirpath(parent->Fields.NCnfNum,parent->Fields.NDirNum,&Rec);  //lint !e534
        CannotMoveFile = ! NeedToPerformAndNotListOnly;
        if (Rec.DskPath[0] == 0 && ! CannotMoveFile) {
          memset(&MsgData,0,sizeof(MsgData));
          MsgData.AutoBox   = TRUE;
          MsgData.Save      = TRUE;
          MsgData.Msg1      = "You do not have a hard disk subdirectory attached to the";
          MsgData.Msg2      = "Target DIR file.  If you continue now the file will instead be";
          MsgData.Msg3      = "moved to the ROOT directory of the current drive.";
          MsgData.Msg4      = "You can attach a subdirectory by pressing ESC once then F2 to edit";
          MsgData.Line1     = 14;
          MsgData.Line2     = 15;
          MsgData.Line3     = 16;
          MsgData.Line4     = 20;
          MsgData.Color1    = Colors[HEADING];
          MsgData.Color2    = Colors[HEADING];
          MsgData.Color3    = Colors[HEADING];
          MsgData.Color4    = Colors[DISPLAY];
          MsgData.Quest     = "Do you want to continue";
          MsgData.QuestLine = 18;
          MsgData.Answer[0] = 'N';
          MsgData.Answer[1] = 0;
          MsgData.Mask      = YESNO;
          showmessage();
          if (MsgData.Answer[0] != 'Y') {
            Aborted = TRUE;
            break;
          }
        }

        strcpy(Temp2,Rec.DskPath);
        strcat(Temp2,parent->Fields.NName);

        if (! CannotMoveFile) {
          verifyexist(Perform,SaveRec->DskPath,PathListName,parent->Fields.NName);  //lint !e534
          if (Flags & OFFLINEDIR) {                            /* if we are moving files from the OFFLINE dir */
            if (fileexist(Temp2) != 255 && DTA.ff_fsize == 0)  /* and the target file is a 0-byte file, then  */
              unlink(Temp2);                                   /* remove the target file before proceeding.   */
          }
          SrceAndDestSame = (strcmp(SrchDskPath,Temp2) == 0 ? TRUE : FALSE);
          if (SrceAndDestSame)
            Code = 0;  /* if SRCE and DEST are the same use a SUCCESSFUL code without trying to move the file */
          else {

            // if OverWrite = Y then always overwrite by deleting the
            //                  file if it already exists
            // if OverWrite = N then always avoid overwriting the file by
            //                  checking for existence and skipping the
            //                  checkmovefile() call if it exists
            // if OverWrite = A then just try to overwrite the file and
            //                  ask permission if it's already there

            Code = 0;
            if (Work.OverWrite == 'Y' || Work.OverWrite == 'N') {
              if (fileexist(Temp2) != 255) {
                if (Work.OverWrite == 'N')
                  Code = -1;
                else
                  unlink(Temp2);
              }
            }

            if (Code == 0) {
              if (parent->Fields.Keep == COPYONLY)
                Code = copyfile(SrchDskPath,Temp2,FALSE);
              else {
                // in batch mode, don't let it stop on an error, use the movefile()
                // function instead of the checkmovefile() function.
                if (BatchMode) {
                  if (fileexist(Temp2) != 255)
                    Code = -1;
                  else
                    Code = movefile(SrchDskPath,Temp2);
                } else
                  Code = checkmovefile(SrchDskPath,Temp2);
              }
            }
          }

          if (Code) {
            sprintf(Temp,"Unchanged : unable to move/copy FILE %s to %u/%u ",parent->Fields.NName,parent->Fields.NCnfNum,parent->Fields.NDirNum);
            if (Code != -1) {
              addchar(Temp,'(');
              strcat(Temp,CopyCodes[Code]);
              addchar(Temp,')');
            }
            printscroll(Temp,Colors[HEADING]);
            goto process;
          } else {
            FilesChanged = TRUE;

            /* Show that it MOVED if the move was either SUCCESSFUL or   */
            /* if the SOURCE and DESTINATION files were the same name!   */
            /* However, record the move in CHANGED.LST only if the file  */
            /* was in fact moved -- this to avoid "undo" problems.       */
            /* If it was MOVED then the entry in CHANGED.LST begins with */
            /* "==" if it was COPIED then it begins with "=+" so as to   */
            /* avoid "undo" problems.                                    */

            if (! SrceAndDestSame) {
              sprintf(Str,"=%c %s to %s\r\n",(parent->Fields.Keep == COPYONLY ? '+' : '='),SrchDskPath,Temp2);
              if (dosfputs(Str,&DosChangedList) == -1) {
                Aborted = TRUE;
                break;
              }
            }
            if (parent->Fields.NCnfNum == 0xFFFF) {
              parent->Fields.NDate = 0xFFFF;    // mark file as being OFFLINE
              sprintf(Temp,"Moved File: %-12.12s OFFLINE to %s",parent->Fields.NName,Rec.DskPath);
              if (Work.ZeroByte)
                if (removefile(SrchDskPath,TRUE) == -1) {
                  printscroll(Temp,Colors[ANSWER]);
                  sprintf(Temp,"WARNING   : unable to make %s into a 0-byte file",SrchDskPath);
                }
            } else
              sprintf(Temp,"%s: %-12.12s to Area %d, DIR %d",(parent->Fields.Keep == COPYONLY ? "Copied Fil" : "Moved File"),parent->Fields.NName,parent->Fields.NCnfNum,parent->Fields.NDirNum);
            printscroll(Temp,Colors[ANSWER]);
          }
        }

        /***  move the description only if TARGET is NOT the OFFLINE dir  ***/
        /***  and only if we were going to write it out anyway - as       ***/
        /***  indicated by the description being totally blank            ***/

        if (parent->Fields.NCnfNum == 0xFFFF || (parent->Fields.FDesc[0] == 0 && parent->Fields.FirstChildPos == VM_INVALID_POS)) {
        } else {
          // if SOURCE==TARGET then we don't want to move the description line
          // or any of the secondary lines, so reset to the source CNF/DIR
          if (strcmp(Rec.DirPath,MainFileName) == 0) {
          } else {
            Code = append(Rec.DirPath,parent,Rec.SortType,AppendUploadDir);
            if (Code == APND_SUCCESS) {
              if (parent->Fields.Keep != COPYONLY) {
                strcpy(Temp,"Moved De");
                WriteItem    = FALSE;        /* remove line from dir file */
              } else {
                strcpy(Temp,"Copied D");     /* line is kept in dir file */
              }
              sprintf(&Temp[8],"sc: %-12.12s to Area %d, DIR %d",parent->Fields.NName,parent->Fields.NCnfNum,parent->Fields.NDirNum);
              printscroll(Temp,Colors[ANSWER]);
            } else {
              sprintf(Temp,"Unchanged : unable to move desc for %s to %u/%u (%s)",parent->Fields.NName,parent->Fields.NCnfNum,parent->Fields.NDirNum,AppendCodes[Code]);
              printscroll(Temp,Colors[HEADING]);
              Aborted = TRUE;
              break;
            }
          }
        }
      }


process:
      if (WriteItem) {
        if (writeparent(&MainFile,MainIdx,parent,UpdateMainIdx,UploadDir) == -1) {
          Aborted = TRUE;
          break;
        }
      }

      if (checkuserabort()) {
        Aborted = TRUE;
        break;
      }
    }

    if (AppendFileOpen) {
      dosfclose(&AppendFile);
      if (UpdateAppendIdx)  {
        AppendIdx.close();
        UpdateAppendIdx = FALSE;
      }
    }

    if (! (Flags & OFFLINEDIR)) {
      dosfclose(&MainFile);
      if (UpdateMainIdx) {
        MainIdx.close();
        UpdateMainIdx = FALSE;
      }
    }

    dosfclose(&DosChangedList);
    removedeletedfiles();

    if (Aborted) {
      undo();
      return(-1);
    }

    removefinished();
    return(0);
  }
  return(-1);
}
