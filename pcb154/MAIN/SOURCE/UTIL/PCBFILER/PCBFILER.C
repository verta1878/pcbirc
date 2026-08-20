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
#include <misc.h>
#include <dosfunc.h>
#include <country.h>
#include <help.h>
#include "pcbfiler.h"
#include "pcbfiler.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


#ifdef __OVERLAY__
static char OvlText[12] = {'O','v','e','r','l','a','y','S','i','z','e',':'};
extern unsigned int _ovrbuffer = (int) ((32 * 1024L) / 16);  /* 32k */
#endif

extern unsigned int _stklen = 0x9000;

#define PCBVersion       "15.3"
//#define PCBDatVersion    "15.0"

/*********************************************************************/

/* Global variables */

char PCBVer[]    = PCBVersion;         /* used in init() */
//char PCBDatVer[] = PCBDatVersion;

bool FilesChanged = FALSE;
bool DisableShellToDos = FALSE;

/*********************************************************************/

#define NumEKeys 35

int  NumExitKeys = NumEKeys;   /* maximum number of exit keys                */
char ExitKeyNum[NumEKeys];     /* actual values are defined in their modules */
int  ExitKeyFlag[NumEKeys];

/*********************************************************************/

#define MenuTotal     3
#define MenuItemTotal 59

char   MenuList[MenuItemTotal][36];
char   MenuAvail[MenuItemTotal];
struct MenuType Menu[MenuTotal];
void  (*MenuFunc[MenuItemTotal])(void);


/*********************************************************************/
/*
NOTE:  this is here only to maintain compatibility with the checkexistence()
       function that is in the PCB libraries .. it is used by PCBSetup by is
       not needed here so it simply returns a NULL pointer.
*/
#pragma warn -par
#pragma argsused
extern "C" char *  getcontext(void *Ptr) {
  return(NULL);
}
#pragma warn +par


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

static void near  exitfunctions(void) {
  /* Execute "atexit" functions */
  while (_atexitcnt--)
    (*_atexittbl[_atexitcnt])();
  _atexitcnt = 0;
}


/*
char static OldDrive;
char static OldPath[66];

static void near  savepath(void) {
  OldDrive = getdisk();
  OldPath[0] = '\\';
  getcurdir(0,&OldPath[1]);
}


static void near  restorepath(void) {
  setdisk(OldDrive);
  chdir(OldPath);
}
*/


static void near  runrefresh(void) {
  char BatchFile[66];

  strcpy(BatchFile,"REFRESH.BAT");
  if (srchpath(BatchFile) != -1)
    shelltoprogram(BatchFile,"");
}


/********************************************************************
*
*  Function: editmenu()
*
*  Desc    :
*
*  Calls   :
*/

void  editmenu(void) {
  menusel(1,EDITMENU,SMALL,0,TRUE);
}


/********************************************************************
*
*  Function: main()
*
*
*/

main(int argc, char **argv) {
  int      X;
  fonttype SaveFont;
  char     CallingProgram[66];
  char     Params[66];

#ifdef DEBUG
  mc_startcheck(erf_standard);
#endif

  memset(ExitKeyNum ,0,NumEKeys);
  memset(ExitKeyFlag,0,NumEKeys * sizeof(int));
  getcountryspecs(0,0);

  DisableShellToDos = FALSE;

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
    if (memicmp(argv[X],"/NODOS",6) == 0) {
      DisableShellToDos = TRUE;
      continue;
    }
    BatchMode = TRUE;
  }

  init();
  SaveFont = getfont();
  setfont(BIGFONT);
  installhandlers();

  if (BatchMode) {
    checkforretry(argc,argv);
    BatchMode = TRUE;
    clscolor(Colors[OUTBOX]);
    generalscreen(MainHead1,MainHead2);
    for (argc--; argc > 0; argc--)
      batch(argv[argc]);
  } else {
    menusel(0,FILERMENU,SMALL,0,TRUE);
  }

  if (FilesChanged)
    runrefresh();

  if (CallingProgram[0] != 0) {
    sprintf(Params,"/FILE:%s",DatFile);
    srchpath(CallingProgram);     //lint !e534
    setfont(SaveFont);
    cls();
    gotoxy(0,0);
    setcursor(CUR_NORMAL);
    exitfunctions();
    closecnames();
    #ifdef DEBUG
      mc_endcheck();
    #endif
    execl(CallingProgram,CallingProgram,Params,NULL); //lint !e534
  }

  setfont(SaveFont);
  cls();
  gotoxy(0,0);
  setcursor(CUR_NORMAL);
  closecnames();
#ifdef DEBUG
  mc_endcheck();
#endif
  return(DosErrorLevel);   /* normally 0 - but is set to 1 if SHOWERR.C aborted */
}
