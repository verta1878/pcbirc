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
#include <sys\stat.h>
#include <alloc.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <process.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <help.h>
#include "pcbdiag.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define PCBVER  "14.0"
#define PROGRAM "PCBSM"

DOSFILE prn;
char ConfigName[66];
smConfigType smConfig;
pcbdattype  PcbData;
char *DatFile;

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

  strcpy(Dest,Name);
  strcat(Dest,Ext);
  strcpy(Temp,Dest);
  srchpath(Temp);
  if (Temp[0] != 0)
    strcpy(Dest,Temp);
}


/********************************************************************
*
*  Function: init()
*
*  Desc    : Initialize screen functions, variables and menus.
*/

void pascal init(void) {
/*HelpVerifyStr  = VerifyStr;*/
  GeneralHelpNum = GENERAL;

  checkname(ColorCnf  ,PROGRAM,".CLR");
  checkname(ConfigName,PROGRAM,".CNF");

  initscrnio();
  fastprintmove(0,0,"Initializing, please wait...",0x0F);

  getsmconfig(&smConfig,ConfigName);
  dosfopen(smConfig.Dev,OPEN_WRIT|OPEN_APPEND|OPEN_DENYNONE,&prn);

  _fmode      = O_BINARY;
#pragma warn -pia
  if (! (ShareStatus = shareloaded()))
    ShareStatus = testforshareloaded(PcbData.NetFile);
#pragma warn +pia

  DatFile = "PCBOARD.DAT";
  readdatfile();
  loadcnames(FALSE);

  MainHead1 = "Diagnostic Utility for PCBoard v15.21";
  MainHead2 = "Copyright (C) 1988-1996 Clark Development Company, Inc";
}
