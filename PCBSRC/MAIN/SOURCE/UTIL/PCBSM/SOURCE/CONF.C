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
#include <dos.h>
#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
extern int ExitKeyFlag[];
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include <dosfunc.h>
#include <help.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMMOVEFIELDS 10

static struct {
  bool     Main;
  bool     Exp;
  bool     Scan;
  bool     Sysop;
  bool     Net;
  unsigned FirstNum;
  unsigned LastNum;
  int      Low;
  int      High;
  bool     Reset;
  unsigned Bytes;
} Conf;

bool static Ins;
bool static FullUpdate;


/********************************************************************
*
*  Function: initconffields()
*
*  Desc    : Initializes the screen fields for input.
*/

static void near pascal initconffields(FldType *P, const bool Insert) {
  char static *Msg[2] = { "removed from registrations", "inserted in registrations " };
  char Q[80];

  sprintf(Q,"%s number of conferences to be %s","First",Msg[Insert]);
  addquest(P, 0,vUNSIGNED,ADJCONF+0,VALIDCONF, 3,10, 5,Q,&Conf.FirstNum,CLEAR,NULL);
  sprintf(Q,"%s number of conferences to be %s","Last ",Msg[Insert]);
  addquest(P, 1,vUNSIGNED,ADJCONF+0,VALIDCONF, 3,11, 5,Q,&Conf.LastNum,CLEAR,NULL);
  addquest(P, 2,vBOOL    ,ADJCONF+1,YESNO    , 3,13, 1,"Adjust conferences user is normally allowed to join         ",&Conf.Main ,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL    ,ADJCONF+2,YESNO    , 3,14, 1,"Adjust conferences user can join with expired subscription  ",&Conf.Exp  ,NOCLEARFLD,NULL);
  addquest(P, 4,vBOOL    ,ADJCONF+3,YESNO    , 3,15, 1,"Adjust the user selected conferences for scanning           ",&Conf.Scan ,NOCLEARFLD,NULL);
  addquest(P, 5,vBOOL    ,ADJCONF+4,YESNO    , 3,16, 1,"Adjust conferences where user becomes a sysop when joining  ",&Conf.Sysop,NOCLEARFLD,NULL);
  addquest(P, 6,vBOOL    ,ADJCONF+5,YESNO    , 3,17, 1,"Adjust conferences where user has Net Status                ",&Conf.Net  ,NOCLEARFLD,NULL);
  addquest(P, 7,vBOOL    ,ADJCONF+6,YESNO    , 3,18, 1,"Reset user's last message read to zero in these conferences ",&Conf.Reset,NOCLEARFLD,NULL);
  addquest(P, 8,vINT     ,ADJCONF+7,ALLNUM   , 3,20, 3,"Adjust users with a security level greater than or equal to ",&Conf.Low  ,CLEAR     ,NULL);
  addquest(P, 9,vINT     ,ADJCONF+7,ALLNUM   ,37,21, 3,                                  "and less than or equal to ",&Conf.High ,CLEAR     ,NULL);
}


static void near pascal setqwkconf(void) {
  unsigned Counter;

  for (Counter = Conf.FirstNum; Counter <= Conf.LastNum; Counter++)
    QwkConfFlags[Counter] = 1;
}


static void near pascal unsetqwkconf(void) {
  unsigned Counter;

  for (Counter = Conf.FirstNum; Counter <= Conf.LastNum; Counter++)
    QwkConfFlags[Counter] = 0;
}


static void near pascal setconf(void far *Conference) {
  unsigned Counter;

  for (Counter = Conf.FirstNum; Counter <= Conf.LastNum; Counter++)
    setbit(Conference,Counter);
}


static void near pascal unsetconf(void far *Conference) {
  unsigned Counter;

  for (Counter = Conf.FirstNum; Counter <= Conf.LastNum; Counter++)
    unsetbit(Conference,Counter);
}


static void near pascal resetconf(URead *p) {
  if (FullUpdate)
    fmemset(&MsgReadPtr[Conf.FirstNum],0,Conf.Bytes);
  else
    memset(p->LastMsgRead[Conf.FirstNum],0,Conf.Bytes);
}


static int pascal adjustconfsub(URead *p) {
  if (p->SecurityLevel >= Conf.Low && p->SecurityLevel <= Conf.High) {
    if (FullUpdate) {
      convertreadtoconfandmsgptrs(p,ConfReg);
      if (Ins) {
        if (Conf.Main)  setconf(&ConfReg[REG]);
        if (Conf.Exp)   setconf(&ConfReg[EXP]);
        if (Conf.Scan)  setconf(&ConfReg[USR]);
        if (Conf.Sysop) setconf(&ConfReg[CON]);
        if (Conf.Net)   setqwkconf();
      } else {
        if (Conf.Main)  unsetconf(&ConfReg[REG]);
        if (Conf.Exp)   unsetconf(&ConfReg[EXP]);
        if (Conf.Scan)  unsetconf(&ConfReg[USR]);
        if (Conf.Sysop) unsetconf(&ConfReg[CON]);
        if (Conf.Net)   unsetqwkconf();
      }
      if (Conf.Reset)
        resetconf(p);
      convertconfandmsgptrstoread(p,ConfReg);
    } else {
      if (Ins) {
        if (Conf.Main) setconf(p->RegConf   );
        if (Conf.Exp)  setconf(p->ExpRegConf);
        if (Conf.Scan) setconf(p->UsrRegConf);
      } else {
        if (Conf.Main) unsetconf(p->RegConf   );
        if (Conf.Exp)  unsetconf(p->ExpRegConf);
        if (Conf.Scan) unsetconf(p->UsrRegConf);
      }
      if (Conf.Reset)
        resetconf(p);
    }
  }
  return(0);
}


/********************************************************************
*
*  Function: adjustconf()
*
*  Desc    : Inserts and Removes a conference a group of users from a specific
*            conference (global operation).
*
*/

void pascal adjustconf(void) {
  static char Msg1[2][7] = { "Remove","Insert" };
  static char Msg2[2][5] = { "from"  ,"into"   };
  bool        Buffered;
  long        Recs;
  unsigned    Temp;
  FldType    *ConfScrn;
  char        Head[40];
  char        Msg[80];

  char *p;                    /* 1   2   3   4   5   6   7   8   9   10  */
  static char BatchCommands[] = "INSEREMOFIRSLASTMAINEXPISCANLOWSHIGHRESE";


  if (BatchMode) {
    memset(&Conf,0,sizeof(Conf));
    while ((p = parsepaths(NULL)) != NULL) {
      switch(findfour(BatchCommands,p)) {
        case  0: break;  /* skip if not found */
        case  1: Ins = 1;                     break;
        case  2: Ins = 0;                     break;
        case  3: Conf.FirstNum = intparam(p); break;
        case  4: Conf.LastNum  = intparam(p); break;
        case  5: Conf.Main     = TRUE;        break;
        case  6: Conf.Exp      = TRUE;        break;
        case  7: Conf.Scan     = TRUE;        break;
        case  8: Conf.Low      = intparam(p); break;
        case  9: Conf.High     = intparam(p); break;
        case 10: Conf.Reset    = TRUE;        break;
      }
    }
    sprintf(Head,"%s Group Conference Registrations",Msg1[Ins]);
    clsbox(1,1,78,23,Colors[OUTBOX]);
    generalscreen(MainHead1,Head);
  } else {
    if ((ConfScrn = (FldType *) mallochk(NUMMOVEFIELDS * sizeof(FldType))) == NULL)
      return;

    Ins = (MenuSelection == 7);
    initconffields(ConfScrn,Ins);

    clsbox(1,1,78,23,Colors[OUTBOX]);
    fastprint(19,22," Press PGDN to continue or ESC to abort ",Colors[DESC]);

    sprintf(Head,"%s Group Conference Registrations",Msg1[Ins]);
    Conf.Main     = TRUE;
    Conf.Exp      = TRUE;
    Conf.Scan     = TRUE;
    Conf.Net      = FALSE;
    Conf.Sysop    = FALSE;
    Conf.FirstNum = 0;
    Conf.LastNum  = 0;
    Conf.Low      = 0;
    Conf.High     = 110;
    Conf.Reset    = FALSE;
    readscrn(ConfScrn,NUMMOVEFIELDS-1,0,MainHead1,Head,1,NOCLEARFLD); //lint !e534
    freescrn(ConfScrn,NUMMOVEFIELDS-1);
  }

  if (Conf.Net && ! QwkSupport)
    Conf.Net = FALSE;

  if (KeyFlags != ESC) {
    FullUpdate = (Conf.LastNum > 39 || Conf.Sysop || Conf.Net);
    Buffered   = (FullUpdate ? BUFFERED : NONBUFFERED);

    if ((Recs = lockusersfile("",Buffered)) != -1) {
      if (Conf.FirstNum > PcbData.NumAreas)
        Conf.FirstNum = PcbData.NumAreas;
      if (Conf.LastNum > PcbData.NumAreas)
        Conf.LastNum = PcbData.NumAreas;

      if (Conf.FirstNum > Conf.LastNum) {
        Temp = Conf.FirstNum;
        Conf.FirstNum = Conf.LastNum;
        Conf.LastNum = Temp;
      }

      if (Conf.Reset)
        Conf.Bytes = (Conf.LastNum-Conf.FirstNum+1)*sizeof(long);

      sprintf(Msg,"%s [%d-%d] %s conference registrations...",Msg1[Ins],Conf.FirstNum,Conf.LastNum,Msg2[Ins]);
      fastprintmove(3,8,Msg,Colors[DISPLAY]);

      processusersfile(Recs,TRUE,FALSE,FullUpdate,Buffered,adjustconfsub);
    }
  }
}
