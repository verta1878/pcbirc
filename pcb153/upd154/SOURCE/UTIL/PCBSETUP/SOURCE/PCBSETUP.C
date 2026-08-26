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


#include <mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <newdata.h>
#include <pcb.h>
#include <country.h>
#include <help.h>
#include <dosfunc.h>
#include <cnameidx.h>
#include "setup.h"
#include "setup.ext"
#include <data.hpp>
#ifdef DEBUG
#include <memcheck.h>
#endif

#if __OVERLAY__
static char OvlText[12] = {'O','v','e','r','l','a','y','S','i','z','e',':'};
extern unsigned int _ovrbuffer = (int) ((28 * 1024L) / 16);  /* 28k */
#endif

extern unsigned int _stklen = 0x8000;

#define PCBVersion       "15.3"

/*********************************************************************/
/* Global variables */

char PCBVer[]    = PCBVersion;      /* used in init() */

/*********************************************************************/

#define NumEKeys 12

int  NumExitKeys = NumEKeys;  /* maximum number of exit keys                */
char ExitKeyNum[NumEKeys];    /* actual values are defined in their modules */
int  ExitKeyFlag[NumEKeys];

/*********************************************************************/

#define MenuTotal     7
#define MenuItemTotal 89

char   _FAR_ MenuList[MenuItemTotal][36];
char   MenuAvail[MenuItemTotal];
struct MenuType Menu[MenuTotal];
void pascal (*MenuFunc[MenuItemTotal])(void);

/********************************************************************
*
*  Function:  exitfunctions()
*
*  Desc    :  Manually executes 'registered-exit-functions'
*
*  NOTES   :  This function hooks into the TURBO C 'hidden' array of exit
*             functions.  Normally the exit() function will call these - but
*             in the case where we perform an exec() function we'll call them
*             manually to be sure it gets take care of!
*/

extern int        _atexitcnt;          /* count of atexit functions */
extern atexit_t   _atexittbl[];        /* array of atexit function pointers */

void near pascal exitfunctions(void) {
  /* Execute "atexit" functions */
  while (_atexitcnt--)
    (*_atexittbl[_atexitcnt])();
  _atexitcnt = 0;
}


static void near pascal runrebuild(void) {
  char BatchFile[66];

  strcpy(BatchFile,"PCBREBLD.BAT");
  if (srchpath(BatchFile) != -1)
    shelltoprogram(BatchFile,"");
}


int main(int argc, char *argv[]) {
  int      X;
  bool     AutoSave;
  fonttype SaveFont;
  char     CallingProgram[66];
  char     Params[66];

#ifdef DEBUG
  mc_startcheck(erf_standard);
#endif

  memset(ExitKeyNum ,0,NumEKeys);
  memset(ExitKeyFlag,0,NumEKeys * sizeof(int));
  getcountryspecs(0,0);

  for (X = 1, OldCnames = FALSE, AutoSave = FALSE; X < argc; X++) {
    if (memicmp(argv[X],"/FILE:",6) == 0)
      DatFile = &argv[X][6];
    if (memicmp(argv[X],"/CNAMES",7) == 0)
      OldCnames = TRUE;
    if (memicmp(argv[X],"/DEFAULT:",9) == 0) {
      DefaultLoc = &argv[X][9];
      AutoSave = TRUE;
    }
    if (memicmp(argv[X],"/FULLSAVE",9) == 0)
      AutoSave = TRUE;
  }

  init();
  SaveFont = getfont();
  setfont(BIGFONT);
  installhandlers();

  KeyFlags = 0;
  do {
    menusel(0,SETUPMENU,SMALL,0,TRUE);
    if (KeyFlags == PGUP)
      KeyFlags = 0;
  } while (KeyFlags != ESC);

  writefile(AutoSave ? SAVEWITHCHECK : ASKTOCHECK);
  if (ConfNameChanged || PerformValidation || AutoSave) {
    buildcnamesidx();
    runrebuild();
    if(PcbData.EnableFido) {
      cAREAS areas;
      areas.updateAkaIndex();
      areas.updateOriginsIndex();
    }
  }

  for (X = 1; X < argc; X++) {
    if (memicmp(argv[X],"/RELOAD:",8) == 0 && argv[X][8] != 0) {
      sprintf(Params,"/FILE:%s",DatFile);
      strcpy(CallingProgram,&argv[X][8]);
      srchpath(CallingProgram);
      setfont(SaveFont);
      exitfunctions();
      #ifdef DEBUG
        mc_endcheck();
      #endif
      execl(CallingProgram,CallingProgram,Params,NULL);
    }
  }

  setfont(SaveFont);
  cls();
  gotoxy(0,2);
  setcursor(CUR_NORMAL);
  exitfunctions();

#ifdef DEBUG
  mc_endcheck();
#endif
  fastprint(0,0,"all done",0x07);
  return(0);
}
