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


#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <pcb.h>
#include <misc.h>
#include <help.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMPACKFIELDS 7


struct PackType Rem;

static unsigned OldDate;
static unsigned OldDays;


static int watchsince(int Before) {
  char Temp[9];

  if (Before) {
    OldDate = Rem.Since;
    return(0);
  }
  if (Rem.Since != OldDate) {
    if (Rem.Since < DateLimit) {
      Rem.Since = DateLimit;
      beep();
    }
    if (Rem.Since > Today) {
      Rem.Since = Today;
      beep();
    }
    datestr(Temp);
    Rem.NumDays = datetojulian(Temp) - Rem.Since;
    return(1);
  }
  return(0);
}


static int watchdays(int Before) {
  char Temp[9];

  if (Before) {
    OldDays = Rem.NumDays;
    return(0);
  }
  if (Rem.NumDays != OldDays) {
    datestr(Temp);
    Rem.Since = datetojulian(Temp) - Rem.NumDays;
    watchsince(Before);   //lint !e534
    return(1);
  }
  return(0);
}

static FldType PackFields[NUMPACKFIELDS] = {
  {vBOOL,YESNO  ,PACKUSER  , 3, 7, 1,NOCLEARFLD,"Remove Users that are Deleted or `LOCKED OUT'    ","",&Rem.Deleted    ,NULL},
  {vINT ,ALLNUM ,PACKUSER+1, 3, 8, 4,CLEAR     ,"Remove Users who have not been on for XXXX days  ","",&Rem.NumDays    ,watchdays},
  {vDATE,ALLDATE,PACKUSER+2, 3, 9, 8,CLEAR     ,"Remove Users who have not been on since          ","",&Rem.Since      ,watchsince},
  {vDATE,ALLDATE,PACKUSER+3, 3,10, 8,CLEAR     ,"Remove Users whose Reg. Exp. Date is older than  ","",&Rem.RegExp     ,NULL},
  {vINT ,ALLNUM ,PACKUSER+4, 3,15, 3,CLEAR     ,"Keep Users with security greater than or equal to","",&Rem.KeepSec    ,NULL},
  {vBOOL,YESNO  ,PACKUSER+5, 3,16, 1,NOCLEARFLD,"Keep Users that are `LOCKED OUT'                 ","",&Rem.KeepLockOut,NULL},
  {vBOOL,YESNO  ,PACKUSER+6, 3,19, 1,NOCLEARFLD,"Print Deleted User Records on the Printer        ","",&Rem.Printer    ,NULL}
};



/********************************************************************
*
*  Function: pack()
*
*  Desc    : Gets user information about the Pack and performs the operation by
*            calling dopack().
*
*/

void pascal pack(void) {
  char *p;                    /* 1   2   3   4   5   6   7   */
  static char BatchCommands[] = "REMODAYSSINCEXPDKEEPLOCKPRIN";

  PrintHeadStr = "Deleted Users";
  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1,"Pack Users File");

  Rem.Printer     = FALSE;
  Rem.KeepSec     = 100;
  Rem.NumDays     = 9999;
  Rem.RegExp      = datetojulian("0-0-0");
  Rem.Since       = DateLimit;

  if (BatchMode) {
    Rem.Deleted     = FALSE;
    Rem.KeepLockOut = FALSE;

    while ((p = parsepaths(NULL)) != NULL) {
      switch(findfour(BatchCommands,p)) {
        case 0: break;  /* skip if not found */
        case 1: Rem.Deleted = TRUE; break;
        case 2: Rem.NumDays = intparam(p); break;
        case 3: Rem.Since = dateparam(p);
                if (Rem.Since > Today)
                  Rem.Since = Today;
                break;
        case 4: Rem.RegExp = dateparam(p); break;
        case 5: Rem.KeepSec = intparam(p); break;
        case 6: Rem.KeepLockOut = TRUE; break;
        case 7: Rem.Printer = TRUE; break;
      }
    }
  } else {
    Rem.Deleted     = TRUE;
    Rem.KeepLockOut = TRUE;
    initquest(PackFields,NUMPACKFIELDS);
    fastprint(3, 5,"Criteria for User Record Removal",Colors[DISPLAY]);
    fastprint(3, 6,"컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴",Colors[DISPLAY]);
    fastprint(3,13,"Criteria for Keeping User Record",Colors[DISPLAY]);
    fastprint(3,14,"컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴",Colors[DISPLAY]);
    fastcenter(22," Press PGDN to begin, or press ESC to abort ",Colors[DESC]);
    readscrn(PackFields,NUMPACKFIELDS-1,0,MainHead1,"Pack Users File",1,NOCLEARFLD);  //lint !e534
    freeanswers(PackFields,NUMPACKFIELDS);
  }

  if (KeyFlags != ESC) {
    switch (MenuSelection) {
      case 2: if (lockusersfile("Pack Users File",BUFFERED) != -1) {
                if (dopack() != -1 && NeedToReindex)
                  createallindexes();
              }
              break;
      #ifndef DEMO
      case 3: packonline();  //lint !e534
              break;
      #endif
    }
  }
  KeyFlags = NOTHING;
}
