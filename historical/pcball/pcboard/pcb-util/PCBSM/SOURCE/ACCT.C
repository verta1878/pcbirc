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
#include <dos.h>
#include <alloc.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <help.h>
#include "pcbfiles.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMACCTFIELDS 7

typedef struct {
  long     Balance;
  bool     AddToExisting;
  bool     ClearAccumulators;
  bool     SetDropLevel;
  char     Low;
  char     High;
  char     DropSecLevel;
} accttype;

static accttype Acct;


/********************************************************************
*
*  Function: initacctfields()
*
*  Desc    : Initializes the screen fields for input.
*/

static void near pascal initacctfields(FldType *P) {
  addquest(P, 0,vLONG,CHGACTBAL+0,ALLNUM   , 3,11, 8,"Modification to Account Starting Balance           ",&Acct.Balance          ,CLEAR     ,NULL);
  addquest(P, 1,vBOOL,CHGACTBAL+1,YESNO    , 3,13, 1,"Add Value to Existing Starting Balance             ",&Acct.AddToExisting    ,NOCLEARFLD,NULL);
  addquest(P, 2,vBOOL,CHGACTBAL+2,YESNO    , 3,14, 1,"Zero Accumulator Values at the Same Time           ",&Acct.ClearAccumulators,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL,CHGACTBAL+3,YESNO    , 3,15, 1,"Set the Security Level Applied on an Empty Account ",&Acct.SetDropLevel     ,NOCLEARFLD,NULL);
  addquest(P, 4,vBYTE,CHGACTBAL+4,ALLNUM   , 3,17, 3,"Adjust users with security greater than or equal to",&Acct.Low              ,CLEAR     ,NULL);
  addquest(P, 5,vBYTE,CHGACTBAL+4,ALLNUM   ,29,18, 3,                          "and less than or equal to",&Acct.High             ,CLEAR     ,NULL);
  addquest(P, 6,vBYTE,CHGACTBAL+5,ALLNUM   , 3,20, 3,"Security Level Given to Users on an Empty Account  ",&Acct.DropSecLevel     ,CLEAR     ,NULL);
}


static int pascal adjustacctsub(URead *p) {
  if (p->SecurityLevel >= Acct.Low && p->SecurityLevel <= Acct.High) {
    if (Acct.AddToExisting)
      Account.StartingBalance += Acct.Balance;
    else
      Account.StartingBalance = Acct.Balance;
    if (Acct.ClearAccumulators)
      memset(&Account.StartThisSession,0,16*sizeof(double));
//        ((int) ((char *) &Account.DropSecLevel - (char *) &Account.StartThisSession)) * sizeof(long));
    if (Acct.SetDropLevel)
      Account.DropSecLevel = Acct.DropSecLevel;
  }
  return(0);
}


/********************************************************************
*
*  Function: adjustacct()
*
*
*/

void pascal adjustacct(void) {
  long      Recs;
  FldType  *Scrn;

  if ((Scrn = (FldType *) mallochk(NUMACCTFIELDS * sizeof(FldType))) == NULL)
    return;

  initacctfields(Scrn);

  clsbox(1,1,78,23,Colors[OUTBOX]);
  fastprint(19,22," Press PGDN to continue or ESC to abort ",Colors[DESC]);

  Acct.Balance           = 0;
  Acct.Low               = 0;
  Acct.High              = 110;
  Acct.AddToExisting     = FALSE;
  Acct.ClearAccumulators = FALSE;
  readscrn(Scrn,NUMACCTFIELDS-1,0,MainHead1,"",1,NOCLEARFLD);  //lint !e534
  freescrn(Scrn,NUMACCTFIELDS-1);

  if (KeyFlags != ESC) {
    if ((Recs = lockusersfile("",BUFFERED)) != -1) {
      setpsaorder();
      fastprintmove(3,9,"Adjusting Starting Balances...",Colors[DISPLAY]);
      processusersfile(Recs,TRUE,FALSE,TRUE,BUFFERED,adjustacctsub);
    }
  }
}
