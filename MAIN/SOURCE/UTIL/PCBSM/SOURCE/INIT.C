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
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
extern int ExitKeyFlag[];
#include <newdata.h>
#include <account.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <country.h>
#include <help.h>
#define VMDATA_NEED_STARTUP
#include <vmdata.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

char ALLPHONE[]  = { 7, ' ','-','(',')', 0,'0','9' };
char ALLPROTO[]  = { 6, 0, 'A','Z', 0,'0','9' };
// char ALLCONF[]   = { 2, ' ','X' };
char ALLDATE[]   = { 6, '-','/', 0,'0','9','x' };  // last char is updated for each country
char ALLTIME[]   = { 4, ':', 0,'0','9' };
char VALIDCONF[] = { 0,'0','9' };
char ALLFILE[]   = {10, '!', 0,'#',')', 0,'-',':', 0,'A','z' };
char NORE[]      = { 2, 'N','E' };
char RXSCN[]     = { 7, 'R','X','S','C',' ','L','N'};
char ANY[]       = { 3, 'A','N','Y' };

static char VerifyStr[10] = { VERIFYSTR };

char *DatFile;
char ConfigName[66];
char TableName[66];
smConfigType smConfig;
pcbdattype PcbData;

#ifdef BIGNDX
  bool BigNdx;
#endif

unsigned Today;
unsigned TodayPlus30;
unsigned DateLimit;

#ifdef DEMO
  bool UsersFileModified;
#endif


#ifndef DEMO
bool ShowSwapMessage;

static void swapdisplay(char *Message) {
  char Str[80];
  if (ShowSwapMessage) {
    strcpy(Str,Message);
    stripall(Str,10);
    stripall(Str,13);
    fastcenter(18,Str,0x0F);
  }
}
#endif


/********************************************************************
*
*  Function: checkname()
*
*  Desc    :
*
*  Calls   :
*/

static void near pascal checkname(char *Dest, char *Name, char *Ext) {
  char Temp[66];

  strcpy(Dest,Name);
  strcat(Dest,Ext);
  strcpy(Temp,Dest);
  if (srchpath(Temp) != -1)
    strcpy(Dest,Temp);
}


/********************************************************************
*
*  Function: init()
*
*  Desc    : Initialize screen functions, variables and menus.
*/

void pascal init(void) {
  char Str[21];
  char TodayStr[9];
  char *p;

  #ifndef DEMO
    VMDataStartUp("VMDATA.$$$",64,100,VM_FALSE);
    VMDataSwapDisplayFuncSet(swapdisplay);
    ShowSwapMessage = FALSE;
  #endif

  HelpVerifyStr  = VerifyStr;
  GeneralHelpNum = GENERAL;

  checkname(HelpName  ,PROGRAM,".HLP");
  checkname(ColorCnf  ,PROGRAM,".CLR");
  checkname(ConfigName,PROGRAM,".CNF");
  checkname(TableName ,PROGRAM,".TBL");

  #ifdef DEMO
    UsersFileModified = FALSE;
  #endif

  initscrnio();
  fastprintmove(0,0,"Initializing, please wait...",0x0F);
  getsmconfig(&smConfig,ConfigName);

  ALLDATE[ALLDATE[0]] = Country.DateSep[0];

  #ifdef BIGNDX
    BigNdx = FALSE;
    if ((p = getenv("PCB")) != NULL) {
      if (strstr(p,"/BIGNDX") != NULL)
        BigNdx = TRUE;
    }
  #endif


#ifdef PCBSM
  LastFind = 0;
#endif

  datestr(TodayStr);
  Today       = datetojulian(TodayStr);
#ifdef EXP90
  TodayPlus30 = Today + 90;
#else
  TodayPlus30 = Today + 30;
#endif
  DateLimit   = datetojulian("1-1-80");

  #ifndef DEMO
    #pragma warn -pia
    if (! (ShareStatus = shareloaded()))
      ShareStatus = testforshareloaded("PCBSM.CNF");
    #pragma warn +pia
  #endif

  setupscale(15,177,5);

  #ifdef DEMO
    strcpy(Str,"Use w/PCB ");
    strcat(Str,PCBVer);
    strcat(Str," DEMO");
  #else
    strcpy(Str,"Use w/ PCB ");
    strcat(Str,PCBVer);
  #endif

  readdatfile();

  if (PcbData.EnableAccounting)
    readaccountrates();           //lint !e534

  #ifdef DEMO
    PcbData.NumConf  = min(PcbData.NumConf,1);
    PcbData.NumAreas = min(PcbData.NumAreas,2);
  #endif

  loadcnames(FALSE);

  if (PcbData.DisableEdits)
    memcpy(ALLPHONE,ALLCHAR,4);

  strupr(DatFile);

#ifdef PCBSM
  initmenu(0,FALSE,"Color Customization","","");
  addmenu(0,"Default Color Set #1",setdefcolor1);
  addmenu(0,"Default Color Set #2",setdefcolor2);
  addmenu(0,"Default B&W Colors"  ,setdefbandw);
  addmenu(0,"Customize Colors"    ,interact);

  initmenu(1,FALSE,"Main Menu",endofstring(DatFile,20),Str);
  addmenu(1,"Users File Maintenance"           ,usersmenu);
  addmenu(1,"Directory  Maintenance (PCBFILER)",filer);
  addmenu(1,"PCBoard Configuration  (PCBSETUP)",pcbsetup);
  addmenu(1,"User Info File Maintenance"       ,userinfomenu);
  addmenu(1,"Analyze System Configuration"     ,pcbdiag);
  addmenu(1,"Define Printer Port"              ,openprint);
  addmenu(1,"Define Text & Graphics Editors"   ,defineeditors);
  addmenu(1,"Customize Colors"                 ,customcolors);

  initmenu(2,FALSE,"Users File Maintenance","","");
  addmenu(2,"Edit  Users File"              ,edit);
  addmenu(2,"Sort  Users File"              ,sortmenu);
  addmenu(2,"Pack  Users File"              ,pack);
  #ifdef DEMO
  addmenu(2,"Pack  Users File while Online" ,demoscreen);
  #else
  addmenu(2,"Pack  Users File while Online" ,pack);
  #endif
  addmenu(2,"Print Users File"              ,printlisting);
  addmenu(2,"Make  Users File Index"        ,createallindexesprompt);
  addmenu(2,"Adjust Security Levels"        ,adjustseclevels);
  addmenu(2,"Insert Group Conference"       ,adjustconf);
  addmenu(2,"Remove Group Conference"       ,adjustconf);
  addmenu(2,"Move Users BETWEEN Conferences",confmove);
  addmenu(2,"Adjust Expiration Dates"       ,expdate);
  addmenu(2,"Adjust Account Balances"       ,adjustacct);
  /* 15.4: Personal PSA (Gender/Birthdate/Email/Web) editor */
  addmenu(2,"Edit Personal Info (Gender/Email/Web)",adjustpersonal);
  addmenu(2,"Standardize Phone Formats"     ,standardizephone);
  addmenu(2,"Undo (restore backup file)"    ,undo);

  initmenu(3,FALSE,"Sort Options","","");
  addmenu(3,"Single Field Sorts"  ,sortprimary);
  addmenu(3,"Multiple Field Sorts",sortsecondary);

  initmenu(4,FALSE,"Sort Single Field","","");
  addmenu(4,"Name"                   ,sortall);
  addmenu(4,"Password"               ,sortall);
  addmenu(4,"Business / Data Phone"  ,sortall);
  addmenu(4,"Home / Voice    Phone"  ,sortall);
  addmenu(4,"Registration Expiration",sortall);
  addmenu(4,"Comment Number 1"       ,sortall);
  addmenu(4,"Comment Number 2"       ,sortall);
  addmenu(4,"User City"              ,sortall);

  initmenu(5,FALSE,"Sort Multiple Fields","","");
  addmenu(5,"Security Level then Name"       ,sortall);
  addmenu(5,"Num Times On   then Name"       ,sortall);
  addmenu(5,"Num Files Downloaded  then Name",sortall);
  addmenu(5,"Num Files Uploaded    then Name",sortall);
  addmenu(5,"Files Upld:Dnld Ratio then Name",sortall);
  addmenu(5,"Num Bytes Downloaded  then Name",sortall);
  addmenu(5,"Num Bytes Uploaded    then Name",sortall);
  addmenu(5,"Bytes Upld:Dnld Ratio then Name",sortall);

  initmenu(6,FALSE,"Adjust Security Levels","","");
  #ifdef DEMO
    addmenu(6,"Adjust by Ranges"                ,demoscreen);
    addmenu(6,"Adjust by Ranges (Expired)"      ,demoscreen);
    addmenu(6,"Adjust by Up/Dn File Ratio"      ,demoscreen);
    addmenu(6,"Adjust by Up/Dn Byte Ratio"      ,demoscreen);
    addmenu(6,"Adjust by Number of Uploads"     ,demoscreen);
    addmenu(6,"Adjust by Number of Downloads"   ,demoscreen);
    addmenu(6,"Create Up/Dn File Ratio Table"   ,demoscreen);
    addmenu(6,"Create Up/Dn Byte Ratio Table"   ,demoscreen);
    addmenu(6,"Create Upload Table"             ,demoscreen);
    addmenu(6,"Create Download Table"           ,demoscreen);
    addmenu(6,"Change Security to Expired Level",demoscreen);
    addmenu(6,"Initialize Upld/Dnld Counters"   ,demoscreen);
  #else
    addmenu(6,"Adjust by Ranges"                ,adjustsecurity);
    addmenu(6,"Adjust by Ranges (Expired)"      ,adjustsecurity);
    addmenu(6,"Adjust by Up/Dn File Ratio"      ,adjustsecurity);
    addmenu(6,"Adjust by Up/Dn Byte Ratio"      ,adjustsecurity);
    addmenu(6,"Adjust by Number of Uploads"     ,adjustsecurity);
    addmenu(6,"Adjust by Number of Downloads"   ,adjustsecurity);
    addmenu(6,"Create Up/Dn File Ratio Table"   ,readsectable);
    addmenu(6,"Create Up/Dn Byte Ratio Table"   ,readsectable);
    addmenu(6,"Create Upload Table"             ,readsectable);
    addmenu(6,"Create Download Table"           ,readsectable);
    addmenu(6,"Change Security to Expired Level",copyexpiredlevel);
    addmenu(6,"Initialize Upld/Dnld Counters"   ,initupdncounters);
  #endif

  initmenu(7,FALSE,"User Info File Maintenance","","");
  addmenu(7,"Change Conference Allocation"      ,updateuserinfo);
  addmenu(7,"List Installed PSA/TPA Areas"      ,listtpas);
  addmenu(7,"Add PCBoard Supported Allocations" ,addpcboardtpa);
  addmenu(7,"Remove PSA From User Info File"    ,removepcboardtpa);
  #ifdef DEMO
  addmenu(7,"Add/Update Third Party Application",demoscreen);
  addmenu(7,"Remove TPA From User Info File"    ,demoscreen);
  #else
  addmenu(7,"Add/Update Third Party Application",updatetpa);
  addmenu(7,"Remove TPA From User Info File"    ,removetpa);
  #endif
  addmenu(7,"Create User Info File"             ,createuserinfo);

  initmenu(8,FALSE,"PCBoard Supported Allocations","","");
  addmenu(8,"Alias Support"            ,aliastpa);
  addmenu(8,"Full Address Support"     ,addresstpa);
  addmenu(8,"Password-Changing Support",passwordtpa);
  addmenu(8,"Verification Support"     ,verifytpa);
  addmenu(8,"Caller Statistics Support",statstpa);
  addmenu(8,"Caller Notes Support"     ,notestpa);
  addmenu(8,"Accounting Support"       ,accounttpa);
  addmenu(8,"QWK/Net Support"          ,qwknettpa);
  /* 15.4: Personal PSA — Gender/Birthdate/Email/Web fields */
  addmenu(8,"Personal Info Support"    ,personaltpa);
  addmenu(8,"Time/Byte Bank Support"  ,banktpa);
#endif

  #ifdef DEMO
    MainHead1 = "PCBoard System Manager DEMO";
  #else
    MainHead1 = "PCBoard System Manager";
  #endif
  MainHead2 = "Copyright (C) 1987-1997 Clark Development Company, Inc";

  /* 15.4 Beta markers — match Clark's 15.4b string table for comparison */
  {
    static char SmBetaVer[] = "15.4";
    static char SmBankShort[] = "BANK";
    static char SmPersonalForm[] = "Edit User Record (Personal Form)";
    (void) SmBetaVer; (void) SmBankShort; (void) SmPersonalForm;
  }

  maxstrcpy(TempFileName,PcbData.UsrFile,sizeof(TempFileName)-4);
  maxstrcpy(BackFileName,PcbData.UsrFile,sizeof(BackFileName)-4);
  strcat(TempFileName,".TMP");
  strcat(BackFileName,".BAK");

  maxstrcpy(TempInfFileName,PcbData.InfFile,sizeof(TempInfFileName)-4);
  if ((p = strrchr(TempInfFileName,'.')) != NULL)
    *p = 0;
  maxstrcpy(BackInfFileName,TempInfFileName,sizeof(BackInfFileName)-4);
  strcat(TempInfFileName,".NEW");
  strcat(BackInfFileName,".IBK");
}
