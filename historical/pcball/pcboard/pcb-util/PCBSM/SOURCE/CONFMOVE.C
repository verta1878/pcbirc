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
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
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

#define NUMCONFMOVEFIELDS 11

#ifdef COPYONLY
  #define unsetbit(x,y) { }
#endif

bool static FullUpdate;

static struct {
  bool     Main;
  bool     Exp;
  bool     Scan;
  bool     Sysop;
  bool     Net;
  unsigned Remove;
  unsigned Add;
  int      Low;
  int      High;
  bool     Flags;
  bool     MsgPtr;
} Move;


static FldType ConfMoveFields[NUMCONFMOVEFIELDS] = {
  {vUNSIGNED,VALIDCONF,ADJMOVE  , 2,10, 5,CLEAR     ,"REMOVE user(s) from which conference"                        ,"",&Move.Remove,NULL},
  {vUNSIGNED,VALIDCONF,ADJMOVE  ,47,10, 5,CLEAR     ,"ADD to which conference"                                     ,"",&Move.Add   ,NULL},
  {vBOOL    ,YESNO    ,ADJCONF+1, 2,12, 1,NOCLEARFLD,"Adjust conferences user is allowed in (at all times)        ","",&Move.Main  ,NULL},
  {vBOOL    ,YESNO    ,ADJCONF+2, 2,13, 1,NOCLEARFLD,"Adjust conferences user is allowed in (expired subscription)","",&Move.Exp   ,NULL},
  {vBOOL    ,YESNO    ,ADJCONF+3, 2,14, 1,NOCLEARFLD,"Adjust conferences user-scan preference list                ","",&Move.Scan  ,NULL},
  {vBOOL    ,YESNO    ,ADJCONF+4, 2,15, 1,NOCLEARFLD,"Adjust conferences where user becomes a sysop upon joining  ","",&Move.Sysop ,NULL},
  {vBOOL    ,YESNO    ,ADJCONF+5, 2,16, 1,NOCLEARFLD,"Adjust conferences where user has Net Status                ","",&Move.Net   ,NULL},
  {vBOOL    ,YESNO    ,ADJMOVE+1, 2,17, 1,NOCLEARFLD,"Set the 'Last Conference In' flag                           ","",&Move.Flags ,NULL},
  {vBOOL    ,YESNO    ,ADJMOVE+2, 2,18, 1,NOCLEARFLD,"Move the 'Last Message Read' pointer to the new conference  ","",&Move.MsgPtr,NULL},
  {vINT     ,ALLNUM   ,ADJMOVE+3, 2,20, 3,CLEAR     ,"Include users with security levels GREATER THAN or EQUAL TO ","",&Move.Low   ,NULL},
  {vINT     ,ALLNUM   ,ADJMOVE+4, 2,21, 3,CLEAR     ,"Include users with security levels  LESS THAN   or EQUAL TO ","",&Move.High  ,NULL}
};


static int pascal confmovesub(URead *p) {
  long    MsgPtrValue;
  char far *Ptr;

  if (p->SecurityLevel >= Move.Low && p->SecurityLevel <= Move.High) {
    if (Move.Main) {
      Ptr = (FullUpdate ? &ConfReg[REG] : p->RegConf);
      if (isset(Ptr,Move.Remove)) {
        setbit(Ptr,Move.Add);
        unsetbit(Ptr,Move.Remove);
        if (Move.Flags) {
          p->LastConf2 = Move.Add;
          if (Move.Add > 255)
            p->LastConference = 255;
          else
            p->LastConference = Move.Add;
        }
      }
    }
    if (Move.Exp) {
      Ptr = (FullUpdate ? &ConfReg[EXP] : p->ExpRegConf);
      if (isset(Ptr,Move.Remove)) {
        setbit(Ptr,Move.Add);
        unsetbit(Ptr,Move.Remove);
      }
    }
    if (Move.Scan) {
      Ptr = (FullUpdate ? &ConfReg[USR] : p->UsrRegConf);
      if (isset(Ptr,Move.Remove)) {
        setbit(Ptr,Move.Add);
        unsetbit(Ptr,Move.Remove);
      }
    }
    if (Move.Sysop) {
      if (isset(&ConfReg[CON],Move.Remove)) {
        setbit(&ConfReg[CON],Move.Add);
        unsetbit(&ConfReg[CON],Move.Remove);
      }
    }
    if (Move.Net) {
      if (isset(&QwkConfFlags,Move.Remove)) {
        setbit(&QwkConfFlags,Move.Add);
        unsetbit(&QwkConfFlags,Move.Remove);
      }
    }
    if (Move.MsgPtr) {
      if (FullUpdate) {
        MsgReadPtr[Move.Add] = MsgReadPtr[Move.Remove];
        MsgReadPtr[Move.Remove] = 0;
      } else {
        MsgPtrValue = bassngltolong((char *) p->LastMsgRead[Move.Remove]);
        longtobassngl((char *) p->LastMsgRead[Move.Add],MsgPtrValue);
        longtobassngl((char *) p->LastMsgRead[Move.Remove],0);
      }
    }
  }
  return(0);
}


/********************************************************************
*
*  Function: confmove()
*
*  Desc    : Inserts and Removes a conference a group of users from a specific
*            conference (global operation).
*
*/

void pascal confmove(void) {
  bool  Buffered;
  long  Recs;
  char  Msg[80];

  initquest(ConfMoveFields,NUMCONFMOVEFIELDS);

  clsbox(1,1,78,23,Colors[OUTBOX]);
  fastprint(19,23," Press PGDN to continue or ESC to abort ",Colors[DESC]);

  Move.Main   = TRUE;
  Move.Exp    = TRUE;
  Move.Scan   = TRUE;
  Move.Sysop  = FALSE;
  Move.Net    = FALSE;
  Move.Remove = 0;
  Move.Add    = 0;
  Move.Low    = 0;
  Move.High   = 110;
  Move.Flags  = FALSE;
  Move.MsgPtr = FALSE;
  readscrn(ConfMoveFields,NUMCONFMOVEFIELDS-1,0,MainHead1,"Move User(s) Between Conferences",1,NOCLEARFLD); //lint !e534
  freeanswers(ConfMoveFields,NUMCONFMOVEFIELDS);


  if (KeyFlags != ESC && (Move.Add != Move.Remove) && (Move.Add <= PcbData.NumAreas) && (Move.Remove <= PcbData.NumAreas)) {
    FullUpdate = (Move.Add > 39 || Move.Remove > 39 || Move.Sysop || Move.Net);
    Buffered   = (FullUpdate ? BUFFERED : NONBUFFERED);

    if ((Recs = lockusersfile("",Buffered)) != -1) {
      sprintf(Msg,"Moving Users from Conference [%d] to [%d]...",Move.Remove,Move.Add);
      fastprintmove(3,8,Msg,Colors[DISPLAY]);

      processusersfile(Recs,TRUE,FALSE,FullUpdate,Buffered,confmovesub);
    }
  }
}
