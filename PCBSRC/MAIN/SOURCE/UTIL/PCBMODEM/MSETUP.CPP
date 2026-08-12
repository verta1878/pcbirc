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


//***************************************************************************
//* (c) 1995  Clark Development Company, Inc.  All Rights Reserved.         *
//* Revision:   2.00   (04/01/95)                                           *
//* Created by  Steve Morris  03/01/94                                      *
//***************************************************************************

// Borland C/C++ includes
#define gotoxy bor_gotoxy
#define wherex bor_wherex
#define wherey bor_wherey
#include <conio.h>
#undef gotoxy
#undef wherex
#undef wherey

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <time.h>

// PCBoard includes
#include <system.h>
#include <dosfunc.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <misc.h>
#include <help.h>

// Local includes
#include "msetup.h"
#include "common.h"
#include "modems.h"

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
char    ExitKeyNum  [ NumEKeys ],
        Filename    [   60 ],
        MFile       [   60 ],
        PCBModem    [   60 ],
        CFGFile     [   60 ],
        TempStr     [  120 ],
        MenuAvail   [MenuItemTotal],
        MenuList    [MenuItemTotal] [ 36 ],
        Mans        [190 ] [ 15 ],
        Names       [ 54 ] [ 25 ];

extern  char * Questions[];

int     NumExitKeys = NumEKeys,
        ExitKeyFlag  [ NumEKeys ],
        y,
        ManCount,
        DefaultNum,
        Update;

word    Loop1,
        Loop2,
        CurrentModem,
        Date;

double  Value;

bool    OK,
        local;

long    BaudRate,
        Length,
        Offset,
        ModemdataOffset;

DOSFILE  DataFile,
         Infile,
         Outfile,
         Tempfile;

cfgtype        Config;
headertype     Header;
modemdatatype  Modemdata;
manufdatatype  * Manufdata;

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int main(void)
{
    _strdate(TempStr);    //lint !e534
    Date = datetojulian(TempStr);
    HelpVerifyStr  = VerifyStr;
    GeneralHelpNum = GENERAL;

    Manufdata = (manufdatatype *) malloc(sizeof(manufdatatype) * MaxModem);

    initall();
    generalscreen(Head1,Head2);
    window(3,5,77,24);

    strcpy(CFGFile,"MSETUP.CNF");
    strcpy(PCBModem,"C:\\PCB\\PCBMODEM.EXE");

    if (dosfopen(CFGFile,READ,&Infile) == -1)
    {
       y = 5;
       strcpy(Config.Modems,"C:\\PCB\\MODEMS.DAT");
       strcpy(Config.PCBDat,"C:\\PCB\\PCBOARD.DAT");
       strcpy(Config.History,"C:\\PCB\\PCBMODEM.NEW");
       editcfg();
    } else {
       if (dosfread(&Config, sizeof(cfgtype), &Infile) == -1)
         editcfg();
       dosfclose(&Infile);
    }

    strcpy(MFile, Config.Modems);

    if (!chkdatafile())
      createarray(TRUE);

    do {
       menusel(MAIN_MENU, 0, SMALL, 0, TRUE);
    } while (KeyFlags != ESC);

    quit(0,"");
    return(0);
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void pascal initall(void)
{
   initprogram();
   setupscale(15,177,5);

   initmenu(MAIN_MENU, FALSE, "Main Menu", "", PCBVERSTR);
   addmenu(MAIN_MENU, MenuChoice[0], newsetup);
   addmenu(MAIN_MENU, MenuChoice[1], changemodem);
   addmenu(MAIN_MENU, MenuChoice[2], updatehistory);
   addmenu(MAIN_MENU, MenuChoice[3], createzip);
   addmenu(MAIN_MENU, MenuChoice[4], editcfg);
   addmenu(MAIN_MENU, MenuChoice[5], defaultmodem);

   MainHead1 = "PCBModem Setup Program";
   MainHead2 = "Copyright (C) 1995  Clark Development Company, Inc.";
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void pascal initcfgfields(FldType *P)
{
   addquest(P,0,vUPSTR,0,mask_alphanum,2,5,39,Questions[23],Config.Modems,CLEAR,NULL);
   addquest(P,1,vUPSTR,0,mask_alphanum,2,6,39,Questions[24],Config.PCBDat,CLEAR,NULL);
   addquest(P,2,vUPSTR,0,mask_alphanum,2,7,39,Questions[25],Config.History,CLEAR,NULL);
   addquest(P,3,vSTR,0,mask_alphanum,2,8,14,Questions[26],Config.Name,CLEAR,NULL);
   addquest(P,4,vSTR,0,mask_alphanum,2,9,7,Questions[27],Config.Version,CLEAR,NULL);
}
// ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void pascal editcfg(void)
{
   FldType *Fields;

   if ((Fields = (FldType *) mallochk(CFGCMDS * sizeof(FldType))) == NULL)
     return;

   initquest(Fields,CFGCMDS-1);
   initcfgfields(Fields);
   readscrn(Fields,CFGCMDS-1,0,"  MSetup Configuration  ","",1,CLEAR);  //lint !e534
   freescrn(Fields,CFGCMDS-1);

   if (dosfopen(CFGFile, CREATE, &Tempfile) == -1)
     quit(7,"editcfg");

   if (dosfwrite(&Config,sizeof(cfgtype),&Tempfile) == -1)
     quit(1,"editcfg");

   dosfclose(&Tempfile);
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void pascal newsetup(void)
{
   memset(Manufdata[Header.NumOfModems].Manufs,0,sizeof(Manufdata[Header.NumOfModems].Manufs));
   memset(Manufdata[Header.NumOfModems].Name,0,sizeof(Manufdata[Header.NumOfModems].Name));
   memset(&Modemdata, 0, sizeof(modemdatatype));
   CurrentModem = Header.NumOfModems;
   Manufdata[CurrentModem].Number = CurrentModem + 1;
   Manufdata[CurrentModem].Offset = Header.NextMdmOffset;
   DefaultNum = 0;
   Update = 1;
   editdata(FALSE);
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void  pascal defaultmodem(void)
{
   setcursor(CUR_NORMAL);
   boxcls(7,16,72,18,12,SINGLE);
error:
   inputnum(9,17,2,"Modify which Default Configuration (1 - 12)",&DefaultNum,vINT,0);

   if (KeyFlags == ESC || DefaultNum == 0)
     return;

   if (DefaultNum < 1 || DefaultNum > 12)
     goto error;

   Update = 0;
   getmdmdata(TRUE);
   editdata(TRUE);
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void pascal updatehistory(void)
{
   setcursor(CUR_NORMAL);
   window(1,1,80,25);
   clrscr();
   spawnlp(P_WAIT,"qedit"," ",Config.History,NULL);    //lint !e534
   title();
   window(3,5,77,24);
   setcursor(CUR_BLANK);
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void pascal createzip(void)
{
   if (strlen(PCBModem) == 0)
   {
     fastprint(4, y, "Couldn't find PCBMODEM.EXE", 13);
     inputstr(4, y+2, 40, "Please enter name & location", PCBModem, PCBModem, mask_alphanum, INPUT_CAPS, 0);
     stripright(PCBModem, 32);
   }
   dosfclose(&DataFile);
   sprintf(TempStr, "-ex pcbmodem.zip %s %s %s", PCBModem, Config.Modems, Config.History);
   window(1,1,80,25);
   clrscr();
   spawnlp(P_WAIT, "pkzip", " ", TempStr, NULL);         //lint !e534
   title();
   window(3,5,77,24);
   setcursor(CUR_BLANK);
   fastcenter(5,"PCBMODEM.ZIP has been created -- Press any key to continue...",11);
   inkey((char*)&OK, CLOCK);     //lint !e534)
   if (dosfopen(MFile, READ, &DataFile) == -1)
     quit(0,"Unable to open MFile");
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
void pascal changemodem(void)
{
   if (selectmodem() < 1)
     return;

   Update = 0;
   editdata(FALSE);
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
int pascal chkdatafile(void)
{
  if (dosfopen(Config.Modems, READ, &DataFile) == -1) {
    boxcls(20, 8, 60, 19, 1, SINGLE);
    fastprint(21, 10, "                                       ", 0x4f);
    fastprint(21, 11, "           MISSING MODEMS.DAT          ", 0x4f);
    fastprint(21, 12, "                                       ", 0x4f);
    fastprint(21, 13, "If desired, a NEW 'MODEMS.DAT' will be ", 0x4f);
    fastprint(21, 14, "created for you.                       ", 0x4f);
    fastprint(21, 15, "                                       ", 0x4f);
    inputnum(28, 17, 1, "Are you sure (Y/N)", &OK, vBOOL, 0);
    if (!OK)
      quit(0,"");
    strcpy(Header.Version, Config.Version);
    Header.Date = Date;
    Header.NumOfModems = 0;
    Header.NextMdmOffset = (long) sizeof(headertype) +
                           (long) sizeof(manufdatatype) * MaxModem +
                           (long) sizeof(modemdatatype) * 12;

    if (dosfopen(Config.Modems, READWR, &DataFile) == -1)
      quit(8,"chkdatafile");

    if (dosfwrite(&Header, sizeof(headertype), &DataFile) == -1)
      quit(9,"chkdatafile");

    ecrypt(TRUE);

    if (dosfwrite(Manufdata, sizeof(manufdatatype)*MaxModem, &DataFile) == -1)
      quit(10,"chkdatafile");

    memset(&Modemdata, 27, sizeof(modemdatatype));

    if (dosfwrite(&Modemdata, sizeof(modemdatatype)*12, &DataFile) == -1)
      quit(11,"chkdatafile");

    return(1);
  }

  dosfclose(&DataFile);

  if (dosfopen(Config.Modems, READWR, &DataFile) == -1)
    quit(12,"chkdatafile");

  return(0);
}
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
