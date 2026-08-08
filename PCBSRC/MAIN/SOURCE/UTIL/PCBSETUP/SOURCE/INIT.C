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
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <help.h>
#include <dosfunc.h>
#include <vmdata.h>
#include "setup.h"
#include "setup.ext"
#include "defines.h"
#include "structs.h"
#include "fidonode.h"
#include "fidoinfo.h"
#include "fidothis.h"
#include "fidoph.h"
#include "fidolist.h"
#include "fidoarc.h"
#include "fidopath.h"
#include "fidoemsi.h"
#include "fidofreq.h"
#include "fidomgic.h"
#include "fidonofr.h"
#include "fidoorg.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

#ifdef FIDO
  #define COUNTFIDO 1
#else
  #define COUNTFIDO 0
#endif

char ConfigName[66];
char ComSpec[66];
smConfigType smConfig;
pcbdattype  PcbData;

char HighAsciiMask[] = {4, 'S','R','C','N'};
char ALLTEXT[]  = {11, ' ','!', 0,'#','+', 0,'-','~', 0,'€','þ' };
char ALLCONF[]  = {24, ' ',':','$','#','!','&', '-', '{', '}', '(',')',',','+','.', 0, '0','9', 0,'A','z', 0,'€','þ','/' };
// char ALLFILE[]  = {10, 0,'#',')', '!', 0,'-',':', 0,'@','z' };
char ALLFILE[]  = { 14,'!',0,'#',')','-','.',0,'0',':',0,'@','z','á','~'};
char ALLTIME[]  = { 3, 0,'0',':' };
char ALLPORT[]  = { 0, '0','9' };
char ALLLPT[]   = { 0, '0','3' };
char ALLBAUD[]  = { 9, ' ',0,'0','9' };
char ALLMARK[]  = { 2, ' ','X' };
char BINARY[]   = { 2, '0','1' };
char HEXNUM[]   = { 6, 0,'0','9', 0,'A','F' };
char ALLPROTO[] = { 6, 0,'0','9', 0,'A','Z' };
char YNSF[]     = { 4, 'Y','N','S','F' };
char ALLDRIVE[] = { 0, 'A','Z' };
char RXS[]      = { 3, 'R','X','S'};
char *DefaultLoc= "";

char VerifyStr[10] = { VERIFYSTR };


bool ShowSwapMessage;

void swapdisplay(char *Message) {
  char Str[80];
  if (ShowSwapMessage) {
    strcpy(Str,Message);
    stripall(Str,10);
    stripall(Str,13);
    fastcenter(23,Str,0x0F);
  }
}


/********************************************************************
*
*  Function: checkname()
*
*  Desc    :
*
*  Calls   :
*/

void near pascal checkname(char *Dest, char *Name, char *Ext) {
  char Temp[66];
  char *p;

  strcpy(Dest,Name);
  strcat(Dest,Ext);
  strcpy(Temp,Dest);
  if (srchpath(Temp) != -1)
    strcpy(Dest,Temp);
  else {
    strcpy(Dest,_argv[0]);
    if ((p = strrchr(Dest,'\\')) != NULL)
      strcpy(p+1,Temp);
    else {
      strcpy(Dest,Name);
      strcat(Dest,Ext);
    }
  }
}


static void near pascal domenu(int MenuNum) {
  int HelpNum;

  KeyFlags = 0;
  do {
    switch (MenuNum) {
      case  2: HelpNum = SYSFILEMENU; break;
      case  3: HelpNum = MODEMMENU;   break;
      case  4: HelpNum = OPTIONSMENU; break;
      case  5: HelpNum = LEVELSMENU;  break;
      case  6: HelpNum = FIDOCFGMENU; break;
      default: HelpNum = 0; break;
    }
    menusel(MenuNum,HelpNum,SMALL,0,TRUE);
  } while (KeyFlags != ESC);
}


void pascal filemenu(void) {
  domenu(2);
}

void pascal modemmenu(void) {
  domenu(3);
}

void pascal configmenu(void) {
  domenu(4);
}

void pascal securitymenu(void) {
  domenu(5);
}

#ifdef FIDO
void pascal fidomenu(void) {
  domenu(6);
}
#endif



/********************************************************************
*
*  Function: init()
*
*  Desc    : Initialize screen functions, variables and menus.
*
*/

void pascal init(void) {
  char Str[51];
  clocktype OldShowClock;
  int  X;
  char *p;

  VMDataStartUp("VMDATA.$$$",32,64,VM_FALSE);
  VMDataSwapDisplayFuncSet(swapdisplay);
  ShowSwapMessage = FALSE;
  #ifdef VM_DEBUG
    VMDebugOn(VM_SANITY_CHECK|VM_MURPHY);
  #endif

  HelpVerifyStr  = VerifyStr;
  GeneralHelpNum = GENERAL;

  checkname(HelpName  ,PROGRAM,".HLP");
  checkname(ColorCnf  ,PROGRAM,".CLR");
  checkname(ConfigName,PROGRAM,".CNF");

  if ((p = getenv("COMSPEC")) != NULL)
    maxstrcpy(ComSpec,p,sizeof(ComSpec));
  else
    strcpy(ComSpec,"COMMAND.COM");

  initscrnio();
  fastprintmove(0,0,"Initializing, please wait...",0x0F);
  OldShowClock = ShowClock;
  ShowClock = NOCLOCK;

  getsmconfig(&smConfig,ConfigName);

/*  _fmode      = O_BINARY; */

  #ifdef DEMO
    strcpy(Str,"Use w/PCB ");
    strcat(Str,PCBVer);
    strcat(Str," DEMO");
  #else
    strcpy(Str,"Use w/ PCB ");
    strcat(Str,PCBVer);
  #endif

  setupscale(16,177,5);

  readpcbdat(DefaultLoc);
  strupr(DatFile);

  initmenu(0,TRUE,"Main Menu",endofstring(DatFile,20),Str);
  addmenu(0,ScreenName[0],sysop);
  addmenu(0,ScreenName[1],filemenu);
  addmenu(0,ScreenName[2],modemmenu);
  addmenu(0,ScreenName[3],node);
  addmenu(0,ScreenName[4],event);
  addmenu(0,ScreenName[5],subscription);
  addmenu(0,ScreenName[6],configmenu);
  addmenu(0,ScreenName[7],securitymenu);
  addmenu(0,ScreenName[8],account);

#ifdef FIDO
  addmenu(0,ScreenName[8+COUNTFIDO],fidomenu);
#endif

  addmenu(0,ScreenName[9+COUNTFIDO],uucp);
  addmenu(0,ScreenName[10+COUNTFIDO],mainbrd);
  addmenu(0,ScreenName[11+COUNTFIDO],conferences);
  addmenu(0,ScreenName[12+COUNTFIDO],searchandreplace);

  initmenu(1,TRUE,"Conferences Menu","","");
  for (X = 1; X <= 42; X++)
    addmenu(1,"",conffunc);

  initmenu(2,TRUE,"File Locations","","");
  addmenu(2,ScreenName[13+COUNTFIDO],systemfiles);
  addmenu(2,ScreenName[14+COUNTFIDO],configurationfiles);
  addmenu(2,ScreenName[15+COUNTFIDO],displayfiles);
  addmenu(2,ScreenName[16+COUNTFIDO],questionnairefiles);

  initmenu(3,TRUE,"Modem Information","","");
  addmenu(3,ScreenName[17+COUNTFIDO],modem);
  addmenu(3,ScreenName[18+COUNTFIDO],modemswitches);
  addmenu(3,ScreenName[19+COUNTFIDO],modemaccess);

  initmenu(4,TRUE,"Configuration Options","","");
  addmenu(4,ScreenName[20+COUNTFIDO],messageoptions);
  addmenu(4,ScreenName[21+COUNTFIDO],transferoptions);
  addmenu(4,ScreenName[22+COUNTFIDO],controloptions);
  addmenu(4,ScreenName[23+COUNTFIDO],systemoptions);
  addmenu(4,ScreenName[24+COUNTFIDO],loggingoptions);
  addmenu(4,ScreenName[25+COUNTFIDO],limits);
  addmenu(4,ScreenName[26+COUNTFIDO],colors);
  addmenu(4,ScreenName[27+COUNTFIDO],functionkeys);
  addmenu(4,ScreenName[28+COUNTFIDO],os2options);

  initmenu(5,TRUE,"Security Levels","","");
  addmenu(5,ScreenName[29+COUNTFIDO],sysopfunctions);
  addmenu(5,ScreenName[30+COUNTFIDO],sysopcommands);
  addmenu(5,ScreenName[31+COUNTFIDO],levels);

#ifdef FIDO
  initmenu(6,TRUE,"Fido Configuration","","");
  addmenu(6,ScreenName[32+COUNTFIDO],fidosetup);
  addmenu(6,ScreenName[33+COUNTFIDO],fidotosser);
  addmenu(6,ScreenName[34+COUNTFIDO],edit_fido_nodes);
  addmenu(6,ScreenName[35+COUNTFIDO],Configure_This_Address);
  addmenu(6,ScreenName[36+COUNTFIDO],edit_fido_emsi);
  addmenu(6,ScreenName[37+COUNTFIDO],edit_fido_dirs);
  addmenu(6,ScreenName[38+COUNTFIDO],edit_fido_arcs);
  addmenu(6,ScreenName[39+COUNTFIDO],edit_fido_phones);
  addmenu(6,ScreenName[40+COUNTFIDO],configure_nodelist);
  addmenu(6,ScreenName[41+COUNTFIDO],Configure_This_Path);
  addmenu(6,ScreenName[42+COUNTFIDO],edit_fido_freq);
  addmenu(6,ScreenName[43+COUNTFIDO],edit_fido_magic);
  addmenu(6,ScreenName[44+COUNTFIDO],edit_fido_deny);
  addmenu(6,ScreenName[45+COUNTFIDO],Configure_Origin);
#endif

  #ifdef DEMO
    MainHead1 = "PCBoard Setup Utility DEMO";
  #else
    MainHead1 = "PCBoard Setup Utility";
  #endif

  MainHead2 = "Copyright (C) 1988-1996 Clark Development Company, Inc";


  #ifdef DEMO
    PcbData.NumConf  = min(PcbData.NumConf,1);
    PcbData.NumAreas = min(PcbData.NumAreas,2);
  #endif

  MenuAvail[Menu[1].First+9] = (PcbData.NumConf > 0);
  ShowClock = OldShowClock;
}
