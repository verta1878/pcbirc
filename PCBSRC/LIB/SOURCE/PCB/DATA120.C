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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "screen.h"
#include "scrnio.h"
#include "scrnio.ext"
#include "misc.h"
#include "dosfunc.h"
#include "validate.h"
#include "pcb.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define BUFLEN 2048

extern DOSFILE pcbfile;

static void near pascal insert(char *Dest, char *Srce) {
  char Temp[256];

  strcpy(Temp,Srce);
  strcat(Temp,Dest);
  strcpy(Dest,Temp);
}


static void near pascal makepathlist(char *DownPaths, char *FileName, bool Append) {
  char    Temp[66];
  char    Result[66];
  DOSFILE File;
  int     Count;
  char    *p;
  int     Flags;

  Flags = OPEN_RDWR|OPEN_DENYNONE;

  if (Append)
    Flags |= OPEN_APPEND;

  validatepath(NULL,FileName,Result,VALIDATE_FILE);
  if (dosfopen(FileName,Flags,&File) == -1)
    return;

  Count = 0;
  p = parsepaths(DownPaths);
  while (p != NULL && p[0] != 0) {
    Count++;
    strcpy(Temp,p);
    addbackslash(Temp,sizeof(Temp));
    if (dosfputs(Temp,&File) == -1 || dosfputs("\r\n",&File) == -1)
      break;
    p = parsepaths(NULL);
  }
  dosfclose(&File);
}


static void near pascal makedirlist(char *GenPath, char *FileName, int Num, int Start, char FirstLetter, bool Append) {
  DirListType2 List;
  char Result[66];
  char Buffer2[61];
  char Buffer1[30];
  char NumBuf[10];
  int  File;
  int  Count;

  Num--;   /*  reduce the number by ONE because we don't want to     */
           /*  include the UPLOAD directory in the DIR.LST file !!!  */

  if (Append) {
    if ((File = dosopencheck(FileName,OPEN_WRIT)) == -1)
      return;
    doslseek(File,0,SEEK_END);
  } else {
    validatepath(NULL,FileName,Result,VALIDATE_FILE);
    if ((File = doscreate(FileName,OPEN_WRIT,OPEN_NORMAL)) == -1)
      return;
  }

  memset(&List,0,sizeof(DirListType2));

  for (Count = Start; Count < Start+Num; Count++) {
    strcpy(Buffer1,GenPath);
    if (FirstLetter)
      addchar(Buffer1,FirstLetter);
    strcat(Buffer1,"DIR");
    strcat(Buffer1,itoa(Count,NumBuf,10));
    sprintf(Buffer2,"%-30.30s",Buffer1);
    memcpy(List.DirPath,Buffer2,30);
    strcpy(Buffer1,"dir number ");
    strcat(Buffer1,itoa(Count,NumBuf,10));
    sprintf(Buffer2,"%-30.30s",Buffer1);
    memcpy(List.DirDesc,Buffer2,30);
    List.SortType = 1;
    if (writecheck(File,&List,sizeof(DirListType2)) == (unsigned) -1)
      break;
  }
  dosclose(File);
}


static void near pascal makebltlist(char *FileName, char *Path, int Num) {
  char Result[66];
  char Buffer1[31];
  char Buffer2[31];
  int  File;
  int  Count;

  validatepath(NULL,FileName,Result,VALIDATE_FILE);
  if ((File = doscreate(FileName,OPEN_WRIT,OPEN_NORMAL)) == -1)
    return;

  for (Count = 1; Count <= Num; Count++) {
    sprintf(Buffer1,"%sBLT%d",Path,Count);
    sprintf(Buffer2,"%-30.30s",Buffer1);
    if (writecheck(File,Buffer2,30) == (unsigned) -1)
      break;
  }
  dosclose(File);
}


static void near pascal makescriptlist(char *FileName, char *Path, char FirstLetter) {
  char Result[66];
  char Buffer3[61];
  char Buffer1[31];
  char Buffer2[31];
  char NumBuf[10];
  int  File;
  int  Counter;
  char *p;
  char *q;

  validatepath(NULL,FileName,Result,VALIDATE_FILE);
  if ((File = doscreate(FileName,OPEN_WRIT,OPEN_NORMAL)) == -1)
    return;

  strcpy(Buffer1,Path);
  if (FirstLetter)
    addchar(Buffer1,FirstLetter);
  strcpy(Buffer2,Buffer1);

  strcat(Buffer1,"SCRIPT");
  strcat(Buffer2,"ANSWER");

  p = &Buffer1[strlen(Buffer1)];
  q = &Buffer2[strlen(Buffer2)];
  Counter = 0;
  while (1) {
    Counter++;
    strcpy(p,itoa(Counter,NumBuf,10));
    if (fileexist(Buffer1) != 255) {
      strcpy(q,p);
      sprintf(Buffer3,"%-30.30s%-30.30s",Buffer1,Buffer2);
      if (writecheck(File,Buffer3,60) == (unsigned) -1)
        break;
    } else break;
  }

  dosclose(File);
}


void pascal read120file(void) {
  char ReadBuf[BUFLEN];
  char DownDrive[1024];
  pcbconftype Main;
  pcbconftype Conf;
  char Temp[256];
  char PubConf[41];
  char FileTemp[31];
  char GnrlDrive[25];
  char MainDrive[25];
  int  Counter;
  int  NumBull;
  int  NumTextDirs;
  int  MainNumTextDirs;
  bool ChangeToMain;
  bool MiniBBS;

  fgetstr(ReadBuf,BUFLEN);

  strcpy(PcbData.Sysop,parse(ReadBuf));
  getstr(PcbData.Password);
  PcbData.Graphics = getlog();


  PcbData.SysopSec[0] = getint();
  PcbData.SysopSec[4]  = PcbData.SysopSec[5] = PcbData.SysopSec[16] = getint();
  for (Counter = 6; Counter < 16; Counter++)
    PcbData.SysopSec[Counter] = getint();
  PcbData.SysopSec[17] = PcbData.SysopSec[15];
  PcbData.SysopSec[1] =
  PcbData.SysopSec[3] = 110;
  PcbData.SysopSec[2] = 100;

  getstr(Main.MsgFile);

  getstr(FileTemp);
  getstr(PcbData.ClrFile);
  stripall(FileTemp,' ');
  stripall(PcbData.ClrFile,' ');
  if (PcbData.ClrFile[0] != 0) {
    addbackslash(PcbData.ClrFile,sizeof(PcbData.ClrFile));
    strcat(PcbData.ClrFile,FileTemp);
    ChangeToMain = FALSE;
  } else {             /* default CALLERS to MAIN if not placed elsewhere */
    ChangeToMain = TRUE;
  }

  getstr(PcbData.WlcFile);
  getstr(PcbData.NewFile);
  getstr(PcbData.UsrFile);
  getstr(PcbData.CnfFile);
  getstr(PcbData.FscFile);
  getstr(PcbData.PwdFile);
  getstr(PcbData.TcnFile);
  getstr(Temp);                /* was name of REMOTE.SYS - no longer needed */
  getstr(Temp);                /* was name of script answer files - not needed */
  getstr(PcbData.AnsFile);
  getstr(Main.DrsFile);  strcat(Main.DrsFile,".DAT");
  getstr(Temp);                /* news file */
  getstr(PcbData.NetFile);
  getstr(PcbData.DldFile);

  getstr(Main.PubUpldLoc); addbackslash(Main.PubUpldLoc,sizeof(Main.PubUpldLoc));
  getstr(GnrlDrive);       addbackslash(GnrlDrive,sizeof(GnrlDrive));
  getstr(MainDrive);       addbackslash(MainDrive,sizeof(MainDrive));
  getstr(PcbData.HlpLoc);  addbackslash(PcbData.HlpLoc,sizeof(PcbData.HlpLoc));
  getstr(Main.UpldDir);    addbackslash(Main.UpldDir,sizeof(Main.UpldDir));
  getstr(PcbData.ChtLoc);  addbackslash(PcbData.ChtLoc,sizeof(PcbData.ChtLoc));

  strcpy(PcbData.NdxLoc,MainDrive);
  strcpy(Main.PrvUpldLoc,Main.PubUpldLoc);
  Main.PubUpldSort = 2;
  Main.PrvUpldSort = 2;

  getstr(DownDrive);

  if (ChangeToMain) {  /* default CALLERS to MAIN if not placed elsewhere */
    strcpy(PcbData.ClrFile,MainDrive);
    strcat(PcbData.ClrFile,FileTemp);
  }

  /*********************************************************************/

  insert(Main.MsgFile,MainDrive);
  insert(PcbData.UsrFile   ,MainDrive);
  insert(PcbData.CnfFile   ,MainDrive);
  insert(PcbData.FscFile   ,MainDrive);
  insert(PcbData.PwdFile   ,MainDrive);
  insert(PcbData.TcnFile   ,MainDrive);
  insert(Main.DrsFile,MainDrive);
  insert(PcbData.AnsFile   ,MainDrive);
  strcpy(Main.PrivDir,MainDrive);  strcat(Main.PrivDir,"PRIVATE");
  strcpy(PcbData.UscFile   ,MainDrive);  strcat(PcbData.UscFile   ,"UPSEC");
  strcpy(PcbData.LogOffAns ,MainDrive);  strcat(PcbData.LogOffAns ,"ANSWER0");
  strcpy(PcbData.RegFile   ,MainDrive);  strcat(PcbData.RegFile   ,"NEWASK");

  insert(PcbData.WlcFile      ,GnrlDrive);
  insert(PcbData.NewFile      ,GnrlDrive);
  strcpy(PcbData.SecLoc       ,GnrlDrive);
  strcpy(PcbData.TxtLoc       ,GnrlDrive);
  strcpy(PcbData.ClsFile      ,GnrlDrive);  strcat(PcbData.ClsFile      ,"CLOSED");
  strcpy(PcbData.WrnFile      ,GnrlDrive);  strcat(PcbData.WrnFile      ,"WARNING");
  strcpy(PcbData.ExpFile      ,GnrlDrive);  strcat(PcbData.ExpFile      ,"EXPIRED");
  strcpy(Main.UserMenu  ,GnrlDrive);  strcat(Main.UserMenu  ,"BRDM");
  strcpy(Main.SysopMenu ,GnrlDrive);  strcat(Main.SysopMenu ,"BRDS");
  strcpy(Main.NewsFile  ,GnrlDrive);  strcat(Main.NewsFile  ,"NEWS");
  strcpy(Main.BltNameLoc,GnrlDrive);  strcat(Main.BltNameLoc,"BLT.LST");
  strcpy(Main.DirNameLoc,GnrlDrive);  strcat(Main.DirNameLoc,"DIR.LST");
  strcpy(Main.PthNameLoc,GnrlDrive);  strcat(Main.PthNameLoc,"DLPATH.LST");
  strcpy(Main.ScrNameLoc,GnrlDrive);  strcat(Main.ScrNameLoc,"SCRIPT.LST");
  strcpy(PcbData.CnfMenu      ,GnrlDrive);  strcat(PcbData.CnfMenu      ,"CNFN");
  strcpy(PcbData.LogOffScr    ,GnrlDrive);  strcat(PcbData.LogOffScr    ,"SCRIPT0");
  strcpy(PcbData.MultiLang    ,GnrlDrive);  strcat(PcbData.MultiLang    ,"PCBML.DAT");
  strcpy(PcbData.GroupChat    ,GnrlDrive);  strcat(PcbData.GroupChat    ,"GCTOPIC");
  strcpy(Main.DrsMenu   ,GnrlDrive);  strcat(Main.DrsMenu   ,"DOORS");
  strcpy(Main.BltMenu   ,GnrlDrive);  strcat(Main.BltMenu   ,"BLT");
  strcpy(Main.ScrMenu   ,GnrlDrive);  strcat(Main.ScrMenu   ,"SCRIPT");
  strcpy(Main.DirMenu   ,GnrlDrive);  strcat(Main.DirMenu   ,"DIR");

  makescriptlist(Main.ScrNameLoc,GnrlDrive,0);

  strcpy(PcbData.TrnFile,"PCBPROT.DAT");

  makepathlist(DownDrive,Main.PthNameLoc,FALSE);

  /*********************************************************************/

  PcbData.Seconds = getint();
  getstr(PcbData.ModemInit);
  getstr(PcbData.ModemOff );
  getstr(PcbData.ModemPort);      PcbData.ModemPort[4] = 0;
  PcbData.ModemSpeed   = getlong();
  PcbData.AllowLowBaud = getlog();
  getstr(PcbData.AllowLowStrt);
  getstr(PcbData.AllowLowStop);

  for (Counter = 0; Counter < 30; Counter++)
    PcbData.UserLevels[Counter] = getint();

  NumBull                   = getint();
  makebltlist(Main.BltNameLoc,GnrlDrive,NumBull);
  PcbData.NumConf           = getint();
  PcbData.NumAreas          = PcbData.NumConf + 1;
  MainNumTextDirs           = getint();
  PcbData.EnforceTime       = getlog();
  Main.PrivUplds      = getlog();
  makedirlist(GnrlDrive,Main.DirNameLoc,MainNumTextDirs,1,0,FALSE);
  PcbData.AllowPwrdOnly     = getlog();
  PcbData.ClosedBoard       = getlog();
  PcbData.NonGraphics       = getlog();
  PcbData.DisplayNews       = TRUE;      /* default to TRUE  in conversion */
  PcbData.DisableCTSdrop    = FALSE;     /* default to FALSE in conversion */

  sprintf(Temp,"DIR%d",MainNumTextDirs);
  strcat(Main.UpldDir,Temp);

  getstr(&PubConf[1]);
  PubConf[0] = 'X';
  for (Counter = 1; Counter < 40; Counter++) {
    if (PubConf[Counter] > '0')
      PubConf[Counter] = 'X';
    else
      PubConf[Counter] = ' ';
  }
  PubConf[40] = 0;

  PcbData.ExitToDos         = getlog();
  PcbData.EventActive       = getlog();
  getstr(PcbData.EventTime);
  PcbData.EventSuspend      = 0;
  PcbData.EventStopUplds    = FALSE;

  PcbData.MaxMsgLines       = getint();
  PcbData.DefaultIntensity  = getint();
  PcbData.DefaultColor      = getint();
  PcbData.Network           = getlog();
  PcbData.NodeNum           = getint();
  PcbData.DisableDriveCheck = getlog();

  /*********************************************************************/

  for (Counter = 0; Counter < 10; Counter++)
    fgetstr(PcbData.FuncKeys[Counter],60);

  fgetstr(Temp,100);                        /* was COMMENT PROMPT in v12.x   */
  strcpy(PcbData.ViewBatch,"PCBVIEW.BAT");  /* now it's a batch file instead */
  strcpy(PcbData.ViewExt  ,"ZIP");

  fgetstr(PcbData.BoardName,63);

  /*********************************************************************/

  fgetstr(ReadBuf,BUFLEN);

  PcbData.ParallelPortNum   = atoi(parse(ReadBuf));
  PcbData.LastReadUpdate    = getlog();
  Main.MsgBlocks            = getint();
  PcbData.AllowEscCodes     = getlog();
  PcbData.AllowCCs          = getlog();
  PcbData.Validate          = getlog();
  PcbData.DisableCls        = getlog();
                              getlog();  /*slow modem*/
  PcbData.ResetModem        = getlog();
  PcbData.UploadBufSize     = getint(); if (PcbData.UploadBufSize > 32) PcbData.UploadBufSize = 32;
  PcbData.DisableEdits      = getlog();
  PcbData.AnswerRing        = getlog();
  PcbData.KbdTimeout        =(getlog() ? 0 : 5);  /* was boolean, now it's a value */
  PcbData.IncludeCity       = getlog();
  PcbData.EliminateSnow     = getlog();
  PcbData.DisableFilter     = getlog();
  PcbData.DisableCTS        = getlog();
  PcbData.EventSlide        = getlog();
  PcbData.StopFreeSpace     = getint();
  PcbData.DisableQuick      = getlog();
  PcbData.DisablePassword   = getlog();
  PcbData.NetTimeout        = getint();
  Main.PrivMsgs             = getlog();
  PcbData.SubscriptMode     = getlog();

  fgetstr(Temp,100);        /* was USER PROMPT-no longer needed */
  dosfclose(&pcbfile);

  loadcnames(TRUE);

  strcpy(Main.Name,"Main Board");
  Main.PublicConf  = TRUE;
  Main.AutoRejoin  = FALSE;
  Main.ViewMembers = FALSE;
  Main.EchoMail    = FALSE;
  Main.ReqSecLevel = 0;
  Main.AddSec      = 0;
  Main.AddTime     = 0;

  putconfrecord(0,&Main);

  /*********************************************************************/

  if (PcbData.NumConf == 0)
    return;

  if (dosfopen(PcbData.CnfFile,OPEN_READ|OPEN_DENYNONE,&pcbfile) == -1) {
    PcbData.NumConf  = 0;
    PcbData.NumAreas = 1;
    return;
  }

  for (Counter = 1; Counter <= PcbData.NumConf; Counter++) {
    fgetstr(ReadBuf,BUFLEN);
    if (Counter == 1 && memcmp(ReadBuf,"Main Board",10) == 0) {
      cls();
      fastprint(0,0,"PCBOARD.DAT is formatted for v12.x and is being converted",0x0F);
      fastprint(0,1,"CNAMES however has already been converted!",0x0F);
      fastprint(0,3,"You have two choices:",0x0F);
      fastprint(0,5,"1) Copy and use a converted PCBOARD.DAT already in v14.x format",0x0F);
      fastprint(0,6,"2) Or restore your CNAMES file to v12.x format and restart the conversion",0x0F);
      gotoxy(0,8);
      exit(99);
    }
    strcpy(Conf.Name,parse(ReadBuf));
    strcpy(FileTemp,Conf.Name);
    strupr(FileTemp);

    getstr(DownDrive);
    NumBull             = getint();
    NumTextDirs         = getint();
    Conf.PrivUplds      = getlog();
    Conf.AutoRejoin     = getlog();
    getstr(Conf.PubUpldLoc);
    addbackslash(Conf.PubUpldLoc,sizeof(Conf.PubUpldLoc));
    strcpy(Conf.PrvUpldLoc,Conf.PubUpldLoc);

    Conf.PubUpldSort = 2;
    Conf.PrvUpldSort = 2;

    getstr(Temp);                       /* conference files location */
    addbackslash(Temp,sizeof(Temp));
    strcpy(Conf.MsgFile   ,Temp);    strcat(Conf.MsgFile   ,FileTemp);
    strcpy(Conf.BltNameLoc,Temp);    strcat(Conf.BltNameLoc,"BLT.LST");
    strcpy(Conf.ScrNameLoc,Temp);    strcat(Conf.ScrNameLoc,"SCRIPT.LST");
    strcpy(Conf.DirNameLoc,Temp);    strcat(Conf.DirNameLoc,"DIR.LST");
    strcpy(Conf.PthNameLoc,Temp);    strcat(Conf.PthNameLoc,"DLPATH.LST");

    strcpy(Conf.DrsFile,Temp);
    addchar(Conf.DrsFile,FileTemp[0]);
    strcat(Conf.DrsFile,"DOORS.DAT");

    makescriptlist(Conf.ScrNameLoc,Temp,FileTemp[0]);

    Conf.ReqSecLevel    = 0;
    Conf.AddSec         = getint();
    Conf.AddTime        = getint();
    Conf.ViewMembers    = getlog();
    MiniBBS             = getlog();
    Conf.MsgBlocks      = getint();
                          getlog();   /* keep uploads inside conference */
    Conf.PrivMsgs       = getlog();

    if (NumTextDirs > 0)
      sprintf(Conf.UpldDir,"%s%cDIR%d",Temp,FileTemp[0],MainNumTextDirs+NumTextDirs);
    else
      strcpy(Conf.UpldDir,Main.UpldDir);

    sprintf(Conf.PrivDir,"%sPRIVATE%c",Temp,FileTemp[0]);
    sprintf(Conf.NewsFile,"%s%cNEWS"  ,Temp,FileTemp[0]);
    sprintf(Conf.DrsMenu,"%s%cDOORS"  ,Temp,FileTemp[0]);
    sprintf(Conf.BltMenu,"%s%cBLT"    ,Temp,FileTemp[0]);
    sprintf(Conf.ScrMenu,"%s%cSCRIPT" ,Temp,FileTemp[0]);
    sprintf(Conf.DirMenu,"%s%cDIR"    ,Temp,FileTemp[0]);

    strcpy(Conf.UserMenu ,Main.UserMenu);
    strcpy(Conf.SysopMenu,Main.SysopMenu);

    makescriptlist(Conf.ScrNameLoc,Temp,FileTemp[0]);

    if (MiniBBS) {
      memset(&MsgData,0,sizeof(MsgData));
      MsgData.AutoBox   = TRUE;
      MsgData.Msg1      = Conf.Name;
      MsgData.Msg2      = "The `Mini-BBS' Flag was found turned ON in this conference.";
      MsgData.Msg3      = "With it ON the conversion will leave MAIN BOARD files";
      MsgData.Msg4      = "inaccessible from within this conference.";
      MsgData.Line1     = 14;
      MsgData.Line2     = 15;
      MsgData.Line3     = 16;
      MsgData.Line4     = 17;
      MsgData.Color1    = Colors[HELPKEY];
      MsgData.Color2    = Colors[HEADING];
      MsgData.Color3    = Colors[HEADING];
      MsgData.Color4    = Colors[HEADING];
      MsgData.Quest     = "Do you want to turn the Mini-BBS flag OFF";
      MsgData.QuestLine = 20;
      MsgData.Answer[0] = 'N';
      MsgData.Answer[1] = 0;
      MsgData.Mask      = YESNO;
      showmessage();
      if (MsgData.Answer[0] == 'Y')
        MiniBBS = FALSE;
    }

    if (MiniBBS) {
      makedirlist(Temp,Conf.DirNameLoc,NumTextDirs,MainNumTextDirs+1,FileTemp[0],FALSE);
      makepathlist(DownDrive,Conf.PthNameLoc,FALSE);
    } else {
      copyfile(Main.DirNameLoc,Conf.DirNameLoc,FALSE);
      copyfile(Main.PthNameLoc,Conf.PthNameLoc,FALSE);
      copyfile(Main.DrsFile,Conf.DrsFile,FALSE);
      if (! Main.PrivUplds) {                                /* if main board uploads were not PRIVATE then add the upload */
        makedirlist(MainDrive,Conf.DirNameLoc,2,MainNumTextDirs,0,TRUE);  /* directory for the main board into the conference DIR.LST   */
      }
      makedirlist(Temp,Conf.DirNameLoc,NumTextDirs,MainNumTextDirs+1,FileTemp[0],TRUE);
      makepathlist(DownDrive,Conf.PthNameLoc,TRUE);
    }

    makebltlist(Conf.BltNameLoc,Temp,NumBull);

    if (Counter < 40 && PubConf[Counter] == 'X')  /* this will convert PRE-14.5 */
      Conf.PublicConf = TRUE;                     /* public conference strings! */

    putconfrecord(Counter,&Conf);
  }
  dosfclose(&pcbfile);

  boxcls(1,5,78,23,Colors[HELPBOX],SINGLE);
  fastprint(3, 7,"The following additional files have been created on your system .. one for",Colors[HELPTITLE]);
  fastprint(3, 8,"each section of the board.  PCBOARD.DAT & CNAMES have not yet changed."    ,Colors[HELPTITLE]);
  fastprint(3,10,"o Created a file called DIR.LST which contains a list of your DIRxx text"  ,Colors[HELPTEXT]);
  fastprint(5,11,  "files (file listings)."                                                  ,Colors[HELPTEXT]);
  fastprint(3,13,"o Created a file called BLT.LST which contains a list of your BLT text"    ,Colors[HELPTEXT]);
  fastprint(5,14,  "files (bulletins)."                                                      ,Colors[HELPTEXT]);
  fastprint(3,16,"o Created a file called DLPATH.LST which contains a list of your download" ,Colors[HELPTEXT]);
  fastprint(5,17,  "paths."                                                                  ,Colors[HELPTEXT]);
  fastprint(3,19,"o Created a file called SCRIPT.LST which contains a list of your script"   ,Colors[HELPTEXT]);
  fastprint(5,20,  "questionnaire and script answer files."                                  ,Colors[HELPTEXT]);
  fastprintmove(28,22,"press any key to continue",Colors[HELPSUB]);
  { char ext; MiniBBS = inkey(&ext,NOCLOCK); }
}
