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
#include <dir.h>
#include <dos.h>
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
#include <country.h>
#include <help.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#ifdef __OVERLAY__
static char OvlText[12] = {'O','v','e','r','l','a','y','S','i','z','e',':'};
extern unsigned int _ovrbuffer = (int) ((32 * 1024L) / 16);  /* 32k */
#endif

extern unsigned int _stklen = 0x8000;

#define PCBVersion       "15.3"
// #define PCBDatVersion    "15.0"

/*********************************************************************/

/* Global variables */

long RecNum;                        /* accessed by edit() and f2() */
char PCBVer[]    = PCBVersion;      /* used in init() */
// char PCBDatVer[] = PCBDatVersion;   /* used in init() */

static char pgmFiler[]  = "PCBFILER.EXE";
static char pgmSetup[]  = "PCBSETUP.EXE";
static char pgmDiag[]   = "PCBDIAG.EXE";

/*********************************************************************/

#define NumEKeys 25

int  NumExitKeys = NumEKeys;   /* maximum number of exit keys                */
char ExitKeyNum[NumEKeys];     /* actual values are defined in their modules */
int  ExitKeyFlag[NumEKeys];

/*********************************************************************/

#define MenuTotal     9
#define MenuItemTotal 71

char   MenuList[MenuItemTotal][36];
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
#ifdef __WATCOMC__
typedef void (*atexit_t)(void);
#endif
extern atexit_t   _atexittbl[];        /* array of atexit function pointers */

static void near pascal exitfunctions(void) {
  /* Execute "atexit" functions */
  while (_atexitcnt--)
    (*_atexittbl[_atexitcnt])();
  _atexitcnt = 0;
}

/*********************************************************************/

void pascal adjustseclevels(void) {
  menusel(6,SECMENU,SMALL,0,TRUE);
}

void pascal usersmenu(void) {
  menusel(2,USERMENU,SMALL,0,TRUE);
}

void pascal userinfomenu(void) {
  menusel(7,INFMENU,SMALL,0,TRUE);
}


static void near pascal runprogram(char *Program) {
  char Path[66];
  char Params[256];

  sprintf(Params,"/FILE:%s /RELOAD:%s",DatFile,_argv[0]);

  strcpy(Path,Program);
  if (srchpath(Path) != -1) {
    exitfunctions();
    execl(Path,Path,Params,NULL); //lint !e534
    exit(1);
  }
}


void pascal filer(void) {
  runprogram(pgmFiler);
}

void pascal pcbdiag(void) {
  runprogram(pgmDiag);
}

void pascal pcbsetup(void) {
  runprogram(pgmSetup);
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

main(int argc,char *argv[]) {
  char CallingProgram[66];
  int  X;

#ifdef DEBUG
  mc_startcheck(erf_standard);
#endif

  memset(ExitKeyNum ,0,NumEKeys);
  memset(ExitKeyFlag,0,NumEKeys * sizeof(int));
  getcountryspecs(0,0);

  DatFile = "PCBOARD.DAT";
  for (X = 1, CallingProgram[0] = 0, BatchMode = FALSE; X < argc; X++) {
    if (memicmp(argv[X],"/FILE:",6) == 0) {
      DatFile = &argv[X][6];
      continue;
    }
    if (memicmp(argv[X],"/RELOAD:",8) == 0) {
      strcpy(CallingProgram,&argv[X][8]);
      continue;
    }
    BatchMode = TRUE;
  }

  init();
  usersalloc();
  installhandlers();

  if (BatchMode) {
    checkforretry(argc,argv);
    BatchMode = TRUE;
    clscolor(Colors[OUTBOX]);
    generalscreen(MainHead1,MainHead2);
    for (X = argc-1; X > 0; X--)
      batch(argv[X]);
  } else {
    menusel(1,MAINMENU,SMALL,0,TRUE);
  }

  usersdealloc();
  cls();
  gotoxy(0,0);
  setcursor(CUR_NORMAL);

  #ifdef DEMO
    if (UsersFileModified) {
      beep();
      fastprint(0,0,"WARNING!",0x0F);
      fastprint(0,2,"During processing PCBSM detected unauthorized modifications made within",0x0C);
      fastprint(0,3,"PCBoard's USERS File.  Be careful of any software that modifies PCBoard's",0x0C);
      fastprint(0,4,"files.  This notice is for your protection.",0x0C);
      gotoxy(0,6);
      mydelay(400);
    }
  #endif

  if (CallingProgram[0] != 0) {
    srchpath(CallingProgram);     //lint !e534
    exitfunctions();
    closecnames();
    #ifdef DEBUG
      mc_endcheck();
    #endif
    execl(CallingProgram,CallingProgram,NULL);   //lint !e534
  }

  exitfunctions();
  closecnames();
#ifdef DEBUG
  mc_endcheck();
#endif
  return(DosErrorLevel);   /* normally 0 - but is set to 1 if SHOWERR.C aborted */
}
