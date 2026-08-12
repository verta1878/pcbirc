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


#include <dos.h>
#include <mem.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include <country.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "unique.hpp"
#include "idx.hpp"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define LISTBUFSIZE 2048

DOSFILE DosChangedList;


char *  makebitarray(char *Str) {
  bool      Found;
  unsigned  Begin;
  unsigned  End;
  unsigned  X;
  char     *Buffer;
  char     *p;

  X = ((PcbData.NumAreas + 7) / 8) + 1;

  if ((Buffer = (char *) malloc(X)) == NULL)
    return(NULL);

  memset(Buffer,0,X);

  for (p = Str, Begin = End = 0xFFFF, Found = FALSE; TRUE; p++) {  //lint !e506 ;DOUBLE CHECK - maybe TRUE should be *p != 0 and remove *p == 0 from bottom
    if (*p < '0' || *p > '9') {
      X = atoi(Str);
      if (Begin == 0xFFFF)
        Begin = X;
      End = X;
      Str = p + 1;

      if (Begin > PcbData.NumConf)
        Begin = PcbData.NumConf;

      if (End > PcbData.NumConf)
        End = PcbData.NumConf;

      if (*p != ' ' && *p != '-') {
        Found = TRUE;
        for (X = Begin; X <= End; X++)
          setbit(Buffer,X);
        Begin = End = 0xFFFF;
      }

      if (*p == 0)
        break;
    }
  }

  if (! Found) {
    free(Buffer);
    return(NULL);
  }

  return(Buffer);
}


int  openchangewrite(void) {
  if (dosfopen("changed.lst",OPEN_RDWR|OPEN_DENYNONE|OPEN_CREATE,&DosChangedList) == -1)
    return(-1);
  dossetbuf(&DosChangedList,LISTBUFSIZE);
  return(0);
}


int  openchangeread(void) {
  if (dosfopen("changed.lst",OPEN_READ|OPEN_DENYNONE,&DosChangedList) == -1)
    return(-1);
  dossetbuf(&DosChangedList,LISTBUFSIZE);
  return(0);
}


static int near  findandbackup(pcbconftype *Conf, unsigned DirNumber, unsigned NumTextDirs, int DirLstFile, uniqueunlimited & DirName) {
  DirListType Rec;

  if (fastfinddirpath(DirNumber,&Rec,Conf,NumTextDirs,DirLstFile) != -1)
    if (Rec.SortType != 0)                             /* no sort, don't backup      */
      if (! DirName.foundinlist(Rec.DirPath))          /* make sure not already done */
        if (fileexist(Rec.DirPath) != 255)             /* if not found, don't backup */
          return(backupfile(Rec.DirPath,Rec.SortType));

  return(checkuserabort() ? -1 : 0);
}


void  process(void) {
  bool            UploadOnly;
  unsigned        BoardNumber;
  unsigned        Num;
  unsigned        NumTextDirs;
  unsigned        End;
  int             DirLstFile;
  char           *p;
  char           *BitArray;
  uniquelist      DirList(DIRLIST_LENGTH,NUM_NONDUPE_LSTS);
  uniqueunlimited DirName(DIRNAME_LENGTH);
  char            Temp[80];
  char            List[128];
  pcbconftype     Conf;

  if (! DirList.Allocated)
    return;
  if (! DirName.Allocated)
    return;

  sprintf(List,"0-%u",PcbData.NumConf);

  if (! BatchMode) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox   = TRUE;
    MsgData.Save      = TRUE;
    MsgData.Quest     = "Sort only upload DIR files (ESC to abort)";
    MsgData.QuestLine = 20;
    MsgData.Answer[0] = 'N';
    MsgData.Answer[1] = 0;
    MsgData.Mask      = YESNO;
    showmessage();
    if (KeyFlags == ESC)
      return;
    UploadOnly = (MsgData.Answer[0] == 'Y');
  } else {
    UploadOnly = FALSE;

    while ((p = parsepaths(NULL)) != NULL) {
      if (findfour("UPLO",p) == 1)  /* check for "/SORT;UPLOADS" */
        UploadOnly = TRUE;
      if (findfour("LIST",p) == 1)  /* check for /SORT;LIST:##-## */
        strcpy(List,p+5);
    }
  }

  if ((BitArray = makebitarray(List)) == NULL)
    return;

/*
  if ((PathListing = (char *) mallochk(NUM_NONDUPE_DIRS * DIRNAME_LENGTH)) == NULL) {
    free(BitArray);
    return;
  }
*/

  if (openchangewrite() == -1)
    goto end;

  memset(Temp,'Ä',78); Temp[78] = 0;
  clsbox(1,2,78,23,Colors[OUTBOX]);
  fastprint(1, 4,Temp,Colors[DISPLAY]);
  fastprint(1,20,Temp,Colors[DISPLAY]);
  fastprint(1, 3,"Sort DIR Files",Colors[QUESTION]);

/* Produce a list of ALL DIR files.  Then if user asked for sort, back up */
/* each one before sorting/colorizing.  Note:  sortchanged() does the     */
/* colorizing because Work.Colorize is true.  It will actually skip the   */
/* sorting if the sort type is 0 as used behow.                           */

  LineCounter = 0;
  NonStop     = Work.NonStop;

  for (BoardNumber = 0; BoardNumber < PcbData.NumAreas; BoardNumber++) {
    if (isset(BitArray,BoardNumber)) {
      getconfrecord(BoardNumber,&Conf);
      stripright(Conf.Name,' ');
      if (Conf.Name[0] == 0 || fileexist(Conf.MsgFile) == 255)
        continue;

      if (! DirList.foundinlist(Conf.DirNameLoc)) {
        NumTextDirs = numrandrecords(Conf.DirNameLoc,sizeof(DirListType2));
        if (NumTextDirs != 0)
          DirLstFile = dosopencheck(Conf.DirNameLoc,OPEN_READ|OPEN_DENYWRIT);
        else
          DirLstFile = -1;

        if (UploadOnly) {
          if (findandbackup(&Conf,0,NumTextDirs,DirLstFile,DirName) == -1) {
            dosclose(DirLstFile);
            goto end;
          }
          if (findandbackup(&Conf,NumTextDirs+1,NumTextDirs,DirLstFile,DirName) == -1) {
            dosclose(DirLstFile);
            goto end;
          }
        } else {
          End = NumTextDirs;
          if (Conf.UpldDir[0] != 0)
            End++;
          for (Num = 0; Num <= End; Num++)
            if (findandbackup(&Conf,Num,NumTextDirs,DirLstFile,DirName) == -1) {
              dosclose(DirLstFile);
              goto end;
            }
        }
        dosclose(DirLstFile);
      }
    }
  }
  dosfclose(&DosChangedList);

  sortchanged();

end:;  /* we skip to here to avoid the sort if the backup aborts */
  free(BitArray);
}


void  makeallindexfiles(void) {
  unsigned        BoardNumber;
  unsigned        Num;
  unsigned        NumTextDirs;
  unsigned        End;
  int             DirLstFile;
  char           *BitArray;
  uniquelist      DirList(DIRLIST_LENGTH,NUM_NONDUPE_LSTS);
  uniqueunlimited DirName(DIRNAME_LENGTH);
  char            List[128];
  char            Temp[80];
  DirListType     Rec;
  pcbconftype     Conf;

  if (! DirList.Allocated)
    return;
  if (! DirName.Allocated)
    return;

  if (strlen(CommandLine) > 6 && CommandLine[5] == ':')
    maxstrcpy(List,&CommandLine[6],sizeof(List));
  else
    sprintf(List,"0-%u",PcbData.NumConf);

  if ((BitArray = makebitarray(List)) == NULL)
    return;

  if (openchangewrite() == -1)
    goto end;

  memset(Temp,'Ä',78); Temp[78] = 0;
  clsbox(1,2,78,23,Colors[OUTBOX]);
  fastprint(1, 4,Temp,Colors[DISPLAY]);
  fastprint(1,20,Temp,Colors[DISPLAY]);
  fastprint(1, 3,"Index DIR Files",Colors[QUESTION]);

  /* Produce a list of ALL DIR files.  */

  LineCounter = 0;
  NonStop     = Work.NonStop;

  for (BoardNumber = 0; BoardNumber < PcbData.NumAreas; BoardNumber++) {
    if (isset(BitArray,BoardNumber)) {
      getconfrecord(BoardNumber,&Conf);
      if (! DirList.foundinlist(Conf.DirNameLoc)) {
        NumTextDirs = numrandrecords(Conf.DirNameLoc,sizeof(DirListType2));
        if (NumTextDirs != 0)
          DirLstFile = dosopencheck(Conf.DirNameLoc,OPEN_READ|OPEN_DENYWRIT);
        else
          DirLstFile = -1;

        End = NumTextDirs;
        if (Conf.UpldDir[0] != 0)
          End++;
        for (Num = 0; Num <= End; Num++) {
          if (fastfinddirpath(Num,&Rec,&Conf,NumTextDirs,DirLstFile) != -1) {
            if (! DirName.foundinlist(Rec.DirPath)) {
              if (fileexist(Rec.DirPath) != 255) {
                sprintf(Temp,"%s\r\n",Rec.DirPath);
                if (dosfputs(Temp,&DosChangedList) == -1) {
                  dosclose(DirLstFile);
                  dosfclose(&DosChangedList);
                  goto end;
                }
              }
            }
          }
        }
        dosclose(DirLstFile);
      }
    }
  }

  dosfclose(&DosChangedList);
  indexchanged();

end:;  /* we skip to here to avoid the sort if the backup aborts */
  free(BitArray);
}


void  removecodes(char *Str) {
  char *p;

  if (Str[0] == ' ' && ((p = strchr(Str,'|')) != NULL))
    *p = ' ';

  while ((p = strchr(Str,'@')) != NULL) {
    if (p[1] == 'X' && isxdigit(p[2]) && isxdigit(p[3])) {
      memcpy(p,p+4,strlen(p+4)+1);
      Str = p;
    } else Str = p + 1;
  }
}


static int near  copydirfile(DOSFILE *Out, char *FileName) {
  int      RetVal;
  unsigned TestValue;
  char     Temp[20];
  DOSFILE  In;
  char     Buffer[1024];

  #ifdef DEBUG
    mc_register(Buffer,sizeof(Buffer));
    mc_register(&In,sizeof(In));
    mc_register(Temp,sizeof(Temp));
  #endif


  RetVal = 0;

  if (fileexist(FileName) != 255) {
    if (dosfopen(FileName,OPEN_READ|OPEN_DENYNONE,&In) != -1) {
      dossetbuf(&In,32768U);
      while (dosfgets(Buffer,sizeof(Buffer),&In) != -1) {
        if (Buffer[0] == '%' && fileexist(&Buffer[1]) != 255)
          copydirfile(Out,&Buffer[1]); //lint !e534
        else if (Buffer[0] == '!' && fileexist(&Buffer[1]) != 255) {
          /* skip over PPE files! */
        } else {
          removecodes(Buffer);
          if (Buffer[0] != ' ' && strlen(Buffer) >= 30) {
            memcpy(Temp,&Buffer[23],8); Temp[8] = 0;
            switch(TestValue = ctod2(Temp)) {
              case      0:
              case 0xFFFE:
              case 0xFFFF: break;
              default    : countrydate(dtoc(TestValue,Temp));
                           memcpy(&Buffer[23],Temp,8);
                           break;
            }
          }
          if (dosfputs(Buffer,Out) < 0 || dosfputs("\r\n",Out) < 0) {
            RetVal = -1;
            break;
          }
        }
      }
      dosfclose(&In);
    }
  }

  #ifdef DEBUG
    mc_unregister(Buffer);
    mc_unregister(&In);
    mc_unregister(Temp);
  #endif

  return(RetVal);
}


void  makefileslist(void) {
  unsigned        BoardNumber;
  unsigned        HighNum;
  unsigned        Num;
  unsigned        NumTextDirs;
  char           *BitArray;
  DOSFILE         Out;
  DirListType     Rec;
  char            List[128];
  char            Temp[256];
  char            Str[256];
  pcbconftype     Conf;
  uniqueunlimited UsedList(sizeof(Rec.DirPath));

  #ifdef DEBUG
    mc_register(&Conf,sizeof(Conf));
    mc_register(&Rec,sizeof(Rec));
    mc_register(&Out,sizeof(Out));
    mc_register(Temp,sizeof(Temp));
    mc_register(List,sizeof(List));
    mc_register(Str,sizeof(Str));
  #endif

  memset(&Out,0,sizeof(Out));

  if (BatchMode && strlen(CommandLine) > 4 && CommandLine[4] == ':')
    strcpy(List,&CommandLine[5]);
  else
    strcpy(List,Work.ConfList);

  if ((BitArray = makebitarray(List)) == NULL) {
    if (! BatchMode)
      noworktodo("list of conferences to scan when creating PCBFILES.LST");
    else {
      showline("Error!  I have no work to do!",Colors[HEADING]);  //lint !e534
      mydelay(200);
    }
    return;
  }

  if (! BatchMode) {
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.AutoBox   = TRUE;
    MsgData.Save      = TRUE;
    MsgData.Quest     = "Create PCBFILES.LST from DIR files";
    MsgData.QuestLine = 20;
    MsgData.Answer[0] = 'N';
    MsgData.Answer[1] = 0;
    MsgData.Mask      = YESNO;
    showmessage();
    if (KeyFlags == ESC || MsgData.Answer[0] != 'Y')
      goto end;
  }

//if (openchangewrite() == -1)
//  goto end;

  clscolor(Colors[OUTBOX]);
  generalscreen("Create list of files","");

  if (dosfopen("PCBFILES.LST",OPEN_WRIT|OPEN_DENYRDWR|OPEN_CREATE,&Out) == -1)
    goto end;

  dossetbuf(&Out,32768U);

/* produce a list of ALL DIR files found inside conferences in ConfList */

  for (BoardNumber = 0; BoardNumber < PcbData.NumAreas; BoardNumber++) {
    if (isset(BitArray,BoardNumber)) {
      getconfrecord(BoardNumber,&Conf);
      showline(Conf.Name,Colors[QUESTION]);  //lint !e534
      NumTextDirs = numrandrecords(Conf.DirNameLoc,sizeof(DirListType2));
      HighNum = NumTextDirs;
      if (Conf.UpldDir[0] != 0)
        HighNum++;

      for (Num = 1; Num <= HighNum; Num++) {
        if (finddirpath(BoardNumber,Num,&Rec) != -1) {
//        if (Rec.DirPath[0] > ' ' && ! alreadyinlist(Rec.DirPath)) {
          if (Rec.DirPath[0] > ' ' && ! UsedList.foundinlist(Rec.DirPath)) {
            sprintf(Temp,"%s: %d) %s",Conf.Name,Num,Rec.DirDesc);
            sprintf(Str,"\r\n%s\r\n",Temp);
            if (dosfputs(Str,&Out) == -1)
              goto end;
            memset(Temp,'=',strlen(Temp));
            sprintf(Str,"%s\r\n\r\n",Temp);
            if (dosfputs(Str,&Out) == -1)
              goto end;
            sprintf(Str,"%s %d\r\n",Rec.DirPath,Rec.SortType);
//          if (dosfputs(Str,&DosChangedList) == -1)
//            goto end;
            showline(Rec.DirPath,Colors[ANSWER]);  //lint !e534
            if (copydirfile(&Out,Rec.DirPath) != 0)
              goto end;
            if (userabort())
              goto end;
          }
        }
      }
    }
  }

end:
  dosfclose(&Out);
//dosfclose(&DosChangedList);
  free(BitArray);

  #ifdef DEBUG
    mc_unregister(&Conf);
    mc_unregister(&Rec);
    mc_unregister(&Out);
    mc_unregister(Temp);
    mc_unregister(List);
    mc_unregister(Str);
  #endif
}
