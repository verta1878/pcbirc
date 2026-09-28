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
#include <dosfunc.h>
#include "pcbdiag.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

extern unsigned int _stklen = 0x8000;  // changed from 0x4000 to 0x8000 on 12/20/94

/*********************************************************************/

#define NumEKeys 9

int  NumExitKeys = NumEKeys;   /* maximum number of exit keys                */
char ExitKeyNum[NumEKeys];     /* actual values are defined in their modules */
int  ExitKeyFlag[NumEKeys];

/*********************************************************************/

#define MenuTotal     1
#define MenuItemTotal 4
#define WindowTotal   1

char   MenuList[MenuItemTotal][36];
char   MenuAvail[MenuItemTotal];
struct MenuType Menu[MenuTotal];
void pascal (*MenuFunc[MenuItemTotal])(void);

// struct WindType Windows[WindowTotal];

extern int        _atexitcnt;          /* count of atexit functions */
extern atexit_t   _atexittbl[];        /* array of atexit function pointers */

static void near pascal exitfunctions(void) {
  /* Execute "atexit" functions */
  while (_atexitcnt--)
    (*_atexittbl[_atexitcnt])();
  _atexitcnt = 0;
}



/********************************************************************
*
*  Function: main()
*
*  Desc    : The entry point for PCBoard System Manager.  It initializes the
*            keyboard definitions and calls the init() function for the rest.
*            It then presents the main menu of options.
*
*/

main(int argc, char *argv[]) {
  int   X;
  char *p;
  char  CallingProgram[66];
  char  Params[66];

#ifdef DEBUG
  mc_startcheck(erf_standard);
#endif

  memset(ExitKeyNum ,0,NumEKeys);
  memset(ExitKeyFlag,0,NumEKeys * sizeof(int));
  memset(&prn,0,sizeof(prn));

  DatFile = "PCBOARD.DAT";
  for (X = argc; X > 0; X--) {
    if (memicmp(argv[X],"/FILE:",6) == 0) {
      DatFile = &argv[X][6];
      continue;
    }
  }

  init();
  installhandlers();

  analyze();
  dosfclose(&prn);
  dosclose(HelpFile);
  uninstallhandlers();
  exitfunctions();

  for (; argc > 0; argc--) {
    if ((p = strstr(argv[argc-1],"/RELOAD:")) != NULL) {
      sprintf(Params,"/FILE:%s",DatFile);
      strcpy(CallingProgram,p+8);
      srchpath(CallingProgram);
      #ifdef DEBUG
       mc_endcheck();
      #endif
      execl(CallingProgram,CallingProgram,Params,NULL);
      exit(1);
    }
  }

  cls();
  gotoxy(0,0);
  setcursor(CUR_NORMAL);

#ifdef DEBUG
  mc_endcheck();
#endif
  return(0);
}
