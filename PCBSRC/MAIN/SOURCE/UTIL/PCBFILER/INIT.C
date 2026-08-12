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
#include <dos.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <country.h>
#include <help.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#include "vmstruct.h"
#include "rules.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


#define  V140FORMAT     0
#define  V142FORMAT     1
#define  V145FORMAT     2
#define  V152FORMAT     3
#define  CURRENTRELEASE V152FORMAT


DefType  Defaults;
DefType  Work;

char ComSpec[66];

char CONFNUMS[8]    = {  7, ',',';',' ','-', 0,'0','9' };
char ABC[3]         = {  0, 'A','C' };
char OTT[3]         = {  0, '1','3' };
char ALLCONF[5]     = {  4, 'X',0,'0','9'};
// char ALLFILE[10] = {  9, 0,'#',')', 0,'-',':', 0,'A','z' };
char ALLFILE[]      = { 14,'!',0,'#',')','-','.',0,'0',':',0,'@','z','á','~'};
char TONAR[6]       = {  5, 'T', 'O', 'N', 'A', 'R' };
char YNAQ[5]        = {  4, 'Y', 'N', 'A', 'Q' };
char ALLDATE[21]    = { 20, '-', '/', 0,'0','9','O','F','L','I','N','E','A','R','C','H','V','D','T',' ','x'}; // last char is updated for each country
char ALLNAME[]      = { 13, '-', '.', 'á','~',
                             0, '#', ')',
                             0, '0', ':',
                             0, '@', 'z'};

static char VerifyStr[10] = { VERIFYSTR };

char *MonthName[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

char *DatFile;
static char ConfName[66];
char DefName[66];
static smConfigType smConfig;
pcbdattype far PcbData;
int  SubstLen;
int  TodaysDate;
bool Protected = FALSE;


bool ShowSwapMessage;
bool ShowSwapQuick;

static void swapdisplay(char *Message) {
  char         Str[80];
  savescrntype Screen;

  if (ShowSwapMessage) {
    strcpy(Str,Message);
    stripall(Str,10);
    stripall(Str,13);
    if (ShowSwapQuick) {
      savescreen(&Screen);
      boxcls(2,19,77,23,Colors[OUTBOX],SINGLE);
      fastcenter(21,Str,Colors[HEADING]);
      mydelay(100);
      restorescreen(&Screen);
    } else {
      fastcenter(23,Str,Colors[HEADING]);
    }
  }
}


static VMDataSwapFailureAction swaphandler(VMDataSwapFailure Failure, char *NewName) {
  // note: this filename is different from our default which we are
  // placing in the current directory as VMDATA.$$$
  static char *path = "C:\\VMSWAP.TMP";

  if (Failure == VM_SWAP_OPEN_FAILED || Failure == VM_SWAP_FULL) {
    if (*path <= 'Z') {
      strcpy(NewName,path);
      (*path)++; // get ready for next call
      return(VM_CREATE_NEW);
    }
  }

  // handle remaining swap file exceptions by aborting
  return(VM_ABORT);
}



/********************************************************************
*
*  Function: checkname()
*
*  Desc    :
*
*  Calls   :
*/

static void near  checkname(char *Dest, char *Name, char *Ext) {
  char Temp[66];

  strcpy(Dest,Name);
  strcat(Dest,Ext);
  strcpy(Temp,Dest);
  if (srchpath(Temp) != -1)
    strcpy(Dest,Temp);
}


int  numrandrecords(char *FileName, int RecSize) {
  struct ffblk rec;
  long   NumRecs;

  if (findfirst(FileName,&rec,FA_ARCH) == -1)
    return(0);

  NumRecs = rec.ff_fsize / RecSize;

  #ifdef DEMO
    if (NumRecs > 3)
      NumRecs = 3;
  #endif
  return((int) NumRecs);
}


static void near  initv140format(void) {
  memset(&Defaults,0,sizeof(DefType));

/*
#ifdef DEMO
  Defaults.MaxDirLines   = 100;
  Defaults.MaxScanLines  = 500;
#else
  Defaults.MaxDirLines   = 1000;
  Defaults.MaxScanLines  = 10000;
#endif
*/

  Defaults.EraseFiles    = 'N';
  Defaults.VerifyExist   = 'N';
  Defaults.Perform       = 'Y';
  Defaults.CheckDupes    = FALSE;
  Defaults.Colorize      = FALSE;
  Defaults.ZeroByte      = TRUE;
  strcpy(Defaults.ConfList,"0");
  strcpy(Defaults.ScanList,"0");
  Defaults.DirColors[NAME] = 0x0E;
  Defaults.DirColors[SIZE] = 0x02;
  Defaults.DirColors[FDTE] = 0x04;
  Defaults.DirColors[FDSC] = 0x0B;
  Defaults.DirColors[HEAD] = 0x06;
  Defaults.DirColors[TEXT] = 0x06;
  Defaults.DirColors[DUPE] = 0x03;
  Defaults.DirColors[DLTD] = 0x0F;
  Defaults.DirColors[OFLN] = 0x05;
}


static void near  initv142format(void) {
  strcpy(Defaults.View[0].Ext,"ARC");
  strcpy(Defaults.View[0].Cmd,"INTERNAL");
  strcpy(Defaults.View[1].Ext,"ZIP");
  strcpy(Defaults.View[1].Cmd,"INTERNAL");
}


static void near  initv145format(void) {
  Defaults.ExpertMode = FALSE;
  Defaults.DirColors[NEWF] = 0x8F;
}


static void near  initv152format(void) {
  switch (Defaults.SetFileDate) {
    case  0 : Defaults.SetFileDate = 'A';  break;
    case  1 :
    case 'Y': Defaults.SetFileDate = 'T';  break;
  }

  Defaults.ShowParentsOnly = FALSE;
  Defaults.BigScreen       = FALSE;
  Defaults.RemoveUpldBy    = FALSE;
  Defaults.RemoveDates     = FALSE;

  strcpy(Defaults.RulesName,"PCBFILER.RLS");
  strcpy(Defaults.ProcName,"PCBFILER.MOV");
}


static void near  convertto145(DefType *New, V142DefType *Old) {
  int X;

  New->UseDiskDate  = Old->UseDiskDate;
  New->NonStop      = Old->NonStop;
  New->EraseFiles   = Old->EraseFiles;
//New->MaxDirLines  = Old->MaxDirLines;
  New->ReadAll      = Old->ReadAll;
  New->Perform      = Old->Perform;
  New->VerifyExist  = Old->VerifyExist;
  New->CheckDupes   = Old->CheckDupes;
  New->Colorize     = Old->Colorize;
//New->MaxScanLines = Old->MaxScanLines;
  New->ZeroByte     = Old->ZeroByte;

  strcpy(New->ScanList  ,Old->ScanList);
  strcpy(New->ConfList  ,Old->ConfList);
  strcpy(New->OffLine   ,Old->OffLine);
  strcpy(New->BackupPath,Old->BackupPath);
  strcpy(New->SubstOld  ,Old->SubstOld);
  strcpy(New->SubstNew  ,Old->SubstNew);

  for (X = 0; X < 6; X++)
    New->View[X] = Old->View[X];

  for (X = NAME; X <= OFLN; X++)
    New->DirColors[X] = Old->DirColors[X];
}


static void near  convertconflist(char *Str) {
  char NewStr[100];
  char Temp[10];
  int  Begin;
  int  End;
  int  X;

  if (strchr(Str,'X') == NULL)
    return;

  Begin = End = -1;
  for (X = 0, NewStr[0] = 0; X <= 39; X++) {
    if (Str[X] == 'X') {
      if (Begin == -1)
        Begin = X;
      End = X;
    } else if (Begin != -1) {
      if (Begin != End)
        sprintf(Temp,"%d-%d,",Begin,End);
      else
        sprintf(Temp,"%d,",Begin);
      strcat(NewStr,Temp);
      Begin = End = -1;
    }
  }

  NewStr[40] = 0;              /* make sure it's no longer than 40 chars */
  stripright(NewStr,',');      /* get rid of any terminating commas      */
  strcpy(Str,NewStr);
}


/********************************************************************
*
*  Function: init()
*
*  Desc    : Initialize screen functions, variables and menus.
*
*/

void  init(void) {
  V142DefType V142Defaults;
  char Str1[40];
  char Today[15];
  int  Counter;
  bool CreateNewDefFile;
  int  DefFile;
  int  Note;
  char *p;

  #ifndef DEMO
    VMDataStartUp("VMDATA.$$$",32,100,VM_FALSE);
    VMDataSwapDisplayFuncSet(swapdisplay);
    VMDataSwapFailureHandlerSet(swaphandler);
    ShowSwapMessage = FALSE;
    ShowSwapQuick   = FALSE;
    #ifdef VM_DEBUG
      VMDebugOn(VM_SANITY_CHECK|VM_MURPHY);
    #endif

    Protected = FALSE;
  #endif

  if ((p = getenv("COMSPEC")) != NULL)
    maxstrcpy(ComSpec,p,sizeof(ComSpec));
  else
    strcpy(ComSpec,"COMMAND.COM");

  datestr(Today);
  TodaysDate = ctod2(Today);

  HelpVerifyStr  = VerifyStr;
  GeneralHelpNum = GENERAL;

  checkname(HelpName,PROGRAM   ,".HLP");
  checkname(ColorCnf,PROGRAM   ,".CLR");
  checkname(ConfName,PROGRAM   ,".CNF");

  initscrnio();

  fastprintmove(0,0,"Initializing, please wait...",0x0F);

  getsmconfig(&smConfig,ConfName);
  ALLDATE[ALLDATE[0]] = Country.DateSep[0];

/*  _fmode      = O_BINARY; */
  setupscale(15,177,5);

  #ifdef DEMO
    strcpy(Str1,"Use w/PCB ");
    strcat(Str1,PCBVer);
    strcat(Str1," DEMO");
  #else
    strcpy(Str1,"Use w/ PCB ");
    strcat(Str1,PCBVer);
  #endif

  readdatfile();

  if (PcbData.ColorFile[0] == 0)
    checkname(DefName ,"PCBFILER",".DEF");
  else
    strcpy(DefName,PcbData.ColorFile);

  #ifdef DEMO
    PcbData.NumConf  = min(PcbData.NumConf,1);
    PcbData.NumAreas = min(PcbData.NumAreas,2);
  #endif

  loadcnames(FALSE);

  strupr(DatFile);

  initmenu(0,FALSE,"Main Menu",endofstring(DatFile,20),Str1);
  addmenu(0,"Edit DIR Files"                  ,editmenu);
  addmenu(0,"Sort all DIR Files"              ,process);
  addmenu(0,"Create Files List"               ,makefileslist);
  addmenu(0,"Scan for Duplicate/Missing Files",scandupe);
  addmenu(0,"Locate File Spec on Disk"        ,scandisk);
  addmenu(0,"Locate File Spec in DIR Files"   ,scandirs);
  addmenu(0,"Locate Text in DIR Files"        ,scandirs);
  addmenu(0,"Edit PCBFiler Defaults Page 1"   ,getdefaults1);
  addmenu(0,"Edit PCBFiler Defaults Page 2"   ,getdefaults2);
  addmenu(0,"Edit Auto-Selection Rules"       ,editrules);
  addmenu(0,"Edit Scheduled Moves File"       ,editmoves);
  addmenu(0,"Move Scheduled Files"            ,automove);
  addmenu(0,"Customize DIR File Colors"       ,choosedircolors);

  initmenu(1,FALSE,"Edit DIR Files","","");
  addmenu(1,"Main Board Directories",maingetdirtoedit);
  addmenu(1,"Conference Directories",confmenu);
  addmenu(1,"Off-Line Directory"    ,readofflinedir);
  addmenu(1,"Undo last change"      ,undolastchange);

  initmenu(2,FALSE,"Conferences Menu","","");
  for (Counter = 1; Counter <= 42; Counter++)
    addmenu(2,"",confgetdirtoedit);

  #ifdef DEMO
    MainHead1 = "PCBFiler DEMO - A Utility for PCBoard Directories";
  #else
    MainHead1 = "PCBFiler - A Utility for PCBoard Directories";
  #endif
  MainHead2 = "Copyright (C) 1986-1996 Clark Development Company, Inc";

  if ((DefFile = dosopen(DefName,OPEN_READ)) == -1)
    CreateNewDefFile = TRUE;
  else
    CreateNewDefFile = FALSE;

  Note = CURRENTRELEASE;
  if (filelength(DefFile) < sizeof(DefType)) {
    if (filelength(DefFile) <= 267)
      Note = V140FORMAT;
    else if (filelength(DefFile) <= 477)
      Note = V142FORMAT;
    else if (filelength(DefFile) <= 501)
      Note = V145FORMAT;
    else {
      dosclose(DefFile);
      CreateNewDefFile = TRUE;
    }
  }

  if (! CreateNewDefFile) {
    doslseek(DefFile,0,SEEK_SET);
    if (Note < V145FORMAT) {
      readcheck(DefFile,&V142Defaults,sizeof(V142DefType)); //lint !e534
      convertto145(&Defaults,&V142Defaults);
    } else
      readcheck(DefFile,&Defaults,sizeof(Defaults));  //lint !e534
    dosclose(DefFile);
    stripright(Defaults.OffLine,' ');
    stripright(Defaults.BackupPath,' ');
    stripright(Defaults.SubstOld,' ');
    stripright(Defaults.SubstNew,' ');
    for (Counter = 0; Counter < 6; Counter++) {
      stripright(Defaults.View[Counter].Ext,' ');
      stripright(Defaults.View[Counter].Cmd,' ');
    }
  }

  if (CreateNewDefFile || Note != CURRENTRELEASE) {
    if (CreateNewDefFile)
      initv140format();
    fastprint(0,1,"Creating a new PCBFILER.DEF file",0x0f);
    unlink(DefName);
    if (Note == V140FORMAT)
      initv142format();
    if (Note < V145FORMAT)
      initv145format();
    initv152format();
    if ((DefFile = doscreatecheck(DefName,OPEN_WRIT,OPEN_NORMAL)) != -1) {
      writecheck(DefFile,&Defaults,sizeof(DefType));  //lint !e534
      dosclose(DefFile);
    }
  }

  if (Defaults.OverWrite != 'A' && Defaults.OverWrite != 'Y' && Defaults.OverWrite != 'N')
    Defaults.OverWrite = 'A';

  addbackslash(Defaults.BackupPath,sizeof(Defaults.BackupPath));
  addbackslash(Defaults.OffLine,sizeof(Defaults.OffLine));
  SubstLen = strlen(Defaults.SubstOld);
  if (SubstLen != 0) {
    addbackslash(Defaults.SubstOld,sizeof(Defaults.SubstOld));
    addbackslash(Defaults.SubstNew,sizeof(Defaults.SubstNew));
  }

  if (Defaults.Indent == 0)
    Defaults.Indent = 32;

  convertconflist(Defaults.ConfList);
  convertconflist(Defaults.ScanList);

  Work = Defaults;
}
