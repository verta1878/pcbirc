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


/******************************************************************************/
/*                                                                            */
/*                                 EVALP.CPP                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                  Data and functions for the cSCRIPT class                  */
/*                    specifically related to evalpostfix                     */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1994 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

#ifdef  ___EXEC___

/******************************************************************************/

// Pragmas

/******************************************************************************/

// Included Files

#if defined(__BORLANDC__) || defined(__TURBOC__)
  #include      <dir.h>
  #include      <alloc.h>
  #include      <malloc.h>
#else
  #include      <direct.h>
  #include      <malloc.h>
  #include      <borland.h>
#endif

#include        <dos.h>
#include        <stdio.h>
#include        <stdlib.h>
//#include        "lrand.hpp"
#include        "pcbmisc.hpp"
#include        "scrmisc.hpp"
//#include        <assert.h>
#ifdef FIDO
//#include        "tossmisc.h"
#include        "data.hpp"
//#include        "structs.h"
#endif

#include        "account.h"
#ifdef DBASE
  #include      "dbase.hpp"
#endif

//#include        "bug.h"
#include        "exec.hpp"
#ifndef LIB
  #include      "pcbmsgs.hpp"
  #include      "msgstub.hpp"
#endif
#ifdef DEBUGSCR
  #include      <memcheck.h>
#endif

/******************************************************************************/

/******************************************************************************/

// Types

/******************************************************************************/

// Constants

char mask_alf   [] = { 6, 0, 'A', 'Z', 0, 'a', 'z' };
char mask_an    [] = { 9, 0, 'A', 'Z', 0, 'a', 'z', 0, '0', '9' };
/******************************************************************************/

// Variables

extern char Scrn_BottomRow;
//extern char *errText[];

extern int pushFlag;
#ifdef LIB

extern int         DebugLevel;
extern int         NumFiles;


extern char *      LastDefaultAnswer;
extern char *      LastAcceptedAnswer;


#endif

bool v1, v2, v3, v4;
bool f1, f2, f3, f4;
/******************************************************************************/

// Function Prototypes
extern char * LIBENTRY scrtext(char * b, int x, int y, int l, bool f);
// void LIBENTRY t2c(int time, char * buf);

        /*------------------------------------------------------------------*/

#if defined(__OS2__) && defined(__BORLANDC__)
  // There is a bug in BC++ for OS/2 which causes the compiler to get hung up
  // trying to compile this function if Global Register Allocation is enabled.
  // To get around the bug, we disable GRA here and re-enable it at the bottom
  // of this function
  #pragma option -O-e
  #ifdef DBASE
    // if DBASE is enabled, then another bug in BC++ for OS/2 causes the
    // compiler to get hung up if the Optimize Locally option is enabled, so
    // we'll disable it here as well and re-enable it down below.
    #pragma option -O-c
  #endif
#endif


cVARVAL * LIBENTRY cSCRIPT::AccountSub(cVARVAL * p1)
{
    cVARVAL acc1(vtINTEGER);
    acc1 = *p1;
    switch (acc1.values.vINTEGER)
    {
        case 0:  return(new cVARVAL(vtDREAL,ptrUData->Account.StartingBalance));
        case 1:  return(new cVARVAL(vtDREAL,ptrUData->Account.StartThisSession));
        case 2:  return(new cVARVAL(vtDREAL,ptrUData->Account.DebitCall));
        case 3:  return(new cVARVAL(vtDREAL,ptrUData->Account.DebitTime));
        case 4:  return(new cVARVAL(vtDREAL,ptrUData->Account.DebitMsgRead));
        case 5:  return(new cVARVAL(vtDREAL,ptrUData->Account.DebitMsgReadCapture));
        case 6:  return(new cVARVAL(vtDREAL,ptrUData->Account.DebitMsgWrite));
        case 7:  return(new cVARVAL(vtDREAL,ptrUData->Account.DebitMsgWriteEchoed));
        case 8:  return(new cVARVAL(vtDREAL,ptrUData->Account.DebitMsgWritePrivate));
        case 9:  return(new cVARVAL(vtDREAL,ptrUData->Account.DebitDownloadFile));
        case 10: return(new cVARVAL(vtDREAL,ptrUData->Account.DebitDownloadBytes));
        case 11: return(new cVARVAL(vtDREAL,ptrUData->Account.DebitGroupChat));
        case 12: return(new cVARVAL(vtDREAL,ptrUData->Account.DebitTPU));
        case 13: return(new cVARVAL(vtDREAL,ptrUData->Account.DebitSpecial));
        case 14: return(new cVARVAL(vtDREAL,ptrUData->Account.CreditUploadFile));
        case 15: return(new cVARVAL(vtDREAL,ptrUData->Account.CreditUploadBytes));
        case 16: return(new cVARVAL(vtDREAL,ptrUData->Account.CreditSpecial));
        case 17: return(new cVARVAL(vtDREAL,ptrUData->Account.DropSecLevel));
        default: return(new cVARVAL(vtDREAL,-1));
    }
}


cVARVAL * LIBENTRY cSCRIPT::PcbAccountSub(cVARVAL * p1)
{
    cVARVAL acc1(vtINTEGER);
    acc1 = *p1;
    switch (acc1.values.vINTEGER)
    {
        case 0:  return(new cVARVAL(vtDREAL,AccountRates.NewUserBalance));
        case 1:  return(new cVARVAL(vtDREAL,AccountRates.ChargeForCall));
        case 2:  return(new cVARVAL(vtDREAL,AccountRates.ChargeForTime));
        case 3:  return(new cVARVAL(vtDREAL,AccountRates.ChargeForPeakTime));
        case 4:  return(new cVARVAL(vtDREAL,AccountRates.ChargeForGroupChat));
        case 5:  return(new cVARVAL(vtDREAL,AccountRates.ChargeForMsgRead));
        case 6:  return(new cVARVAL(vtDREAL,AccountRates.ChargeForMsgReadCapture));
        case 7:  return(new cVARVAL(vtDREAL,AccountRates.ChargeForMsgWrite));
        case 8:  return(new cVARVAL(vtDREAL,AccountRates.ChargeForMsgWriteEchoed));
        case 9:  return(new cVARVAL(vtDREAL,AccountRates.ChargeForMsgWritePrivate));
        case 10: return(new cVARVAL(vtDREAL,AccountRates.ChargeForDownloadFile));
        case 11: return(new cVARVAL(vtDREAL,AccountRates.ChargeForDownloadBytes));
        case 12: return(new cVARVAL(vtDREAL,AccountRates.PayBackForUploadFile));
        case 13: return(new cVARVAL(vtDREAL,AccountRates.PayBackForUploadBytes));
        case 14: return(new cVARVAL(vtDREAL,AccountRates.WarnLevel));
        default: return(new cVARVAL(vtDREAL,-1));
    }
}


cVARVAL * LIBENTRY cSCRIPT::ScanMsgHdrSub(cVARVAL *p1, cVARVAL *p2, cVARVAL *p3, cVARVAL *p4)
{
    int done        = FALSE;
    long currMsgNum = 0;
    pcbconftype       confrec;
    char buf[26];                // longest field length in standard header.
    //cPCBMSGS message;
    cVARVAL int4(vtINTEGER);
    cVARVAL int3(vtINTEGER);
    cVARVAL int2(vtINTEGER);
    cVARVAL str1(vtSTRING);

    int4 = *p4;
    int3 = *p3;
    int2 = *p2;
    str1 = *p1;

    if(int4.values.vINTEGER > PcbData.NumConf || int4.values.vINTEGER < 0)
        return(new cVARVAL(vtINTEGER,currMsgNum));

    getconfrecord((unsigned)int4.values.vINTEGER,&confrec);

    if(message.open(confrec.MsgFile) != 0)
        return(new cVARVAL(vtINTEGER,currMsgNum));

    if(message.seekIdx(int3.values.vINTEGER) != 0)
    {
        message.close();
        return(new cVARVAL(vtINTEGER,currMsgNum));
    }

    strupr(str1.values.vSTRING);
    while(!done)
    {
        currMsgNum = message.readIdx();
        if(currMsgNum == -1)
        {
           done = TRUE;
           break;
        }

        message.readHdr(message.idxRec.off);

        switch(int2.values.vINTEGER)
        {
         case 1:
             buf[0] = message.msgHdr.status;
             buf[1] = '\x0';
             break;
         case 2:
             ltoa((long)message.msgHdr.number,buf,10);
             break;
         case 3:
             ltoa((long)message.msgHdr.refer,buf,10);
             break;
         case 4:
             itoa((int)message.msgHdr.blocks,buf,10);
             break;
         case 5:
             memcpy(buf,message.msgHdr.dmsg,8);
             buf[8] = '\x0';
             break;
         case 6:
             memcpy(buf,message.msgHdr.tmsg,5);
             buf[5] = '\x0';
             break;
         case 7:
             memcpy(buf,message.msgHdr.tname,25);
             buf[25] = '\x0';
             break;
         case 8:
             ltoa((long)message.msgHdr.dread,buf,10);
             break;
         case 9:
             memcpy(buf,message.msgHdr.tread,5);
             buf[5] = '\x0';
             break;
         case 10:
             buf[0] = message.msgHdr.reply;
             buf[1] = '\x0';
             break;
         case 11:
             memcpy(buf,message.msgHdr.fname,25);
             buf[25] = '\x0';
             break;
         case 12:
             memcpy(buf,message.msgHdr.subj,25);
             buf[25] = '\x0';
             break;
         case 13:
             memcpy(buf,message.msgHdr.pwd,12);
             buf[12] = '\x0';
             break;
         case 14:
             itoa((int)message.msgHdr.active,buf,10);
             break;
         case 15:
             buf[0] =  message.msgHdr.echo;
             buf[1] =  '\x0';
             break;
         default:
             buf[0] = ' ';
             buf[10] = '\x0';
             done = TRUE;
             break;
        }

        strupr(buf);
        if(strstr(buf,str1.values.vSTRING))
          done = TRUE;
    }
    return(new cVARVAL(vtINTEGER,currMsgNum));
}


/*===========================================================================
 * GetMsgHdrSub — implementation of GETMSGHDR(conf, message, field).
 *
 * Per WHATSNEW 15.4:
 *   "This function allows you to get specific message header info.
 *    Field #'s are the same as those used for the SCANMSGHDR function."
 *
 * Field indices mirror ScanMsgHdrSub above (1..15).  Returns a STRING;
 * empty string on any failure (bad conf, msg not found, etc.).
 *
 * v0.028: hexadecimal — mirrors the ScanMsgHdrSub open→seekIdx→readHdr
 * pattern but returns immediately after reading one field instead of
 * looping.
 *===========================================================================*/

cVARVAL * LIBENTRY cSCRIPT::GetMsgHdrSub(cVARVAL *p1, cVARVAL *p2, cVARVAL *p3)
{
    pcbconftype confrec;
    char        buf[26];

    cVARVAL int1(vtINTEGER);
    cVARVAL i2(vtDREAL);
    cVARVAL int3(vtINTEGER);

    int1 = *p1;    /* conf     */
    i2   = *p2;    /* message  */
    int3 = *p3;    /* field    */

    if (int1.values.vINTEGER > PcbData.NumConf ||
        int1.values.vINTEGER < 0)
        return new cVARVAL(vtSTRING, "");

    getconfrecord((unsigned)int1.values.vINTEGER, &confrec);

    if (message.open(confrec.MsgFile) != 0)
        return new cVARVAL(vtSTRING, "");

    if (message.seekIdx((long)i2.values.vDREAL) != 0) {
        message.close();
        return new cVARVAL(vtSTRING, "");
    }

    /* readIdx returns -1 at end; message.idxRec.off holds the header
     * offset for the current record.
     */
    long cur = message.readIdx();
    if (cur == -1) {
        message.close();
        return new cVARVAL(vtSTRING, "");
    }

    message.readHdr(message.idxRec.off);

    buf[0] = 0;
    switch (int3.values.vINTEGER) {
        case 1:
            buf[0] = message.msgHdr.status;
            buf[1] = 0;
            break;
        case 2:
            ltoa((long)message.msgHdr.number, buf, 10);
            break;
        case 3:
            ltoa((long)message.msgHdr.refer, buf, 10);
            break;
        case 4:
            itoa((int)message.msgHdr.blocks, buf, 10);
            break;
        case 5:
            memcpy(buf, message.msgHdr.dmsg, 8);
            buf[8] = 0;
            break;
        case 6:
            memcpy(buf, message.msgHdr.tmsg, 5);
            buf[5] = 0;
            break;
        case 7:
            memcpy(buf, message.msgHdr.tname, 25);
            buf[25] = 0;
            break;
        case 8:
            ltoa((long)message.msgHdr.dread, buf, 10);
            break;
        case 9:
            memcpy(buf, message.msgHdr.tread, 5);
            buf[5] = 0;
            break;
        case 10:
            buf[0] = message.msgHdr.reply;
            buf[1] = 0;
            break;
        case 11:
            memcpy(buf, message.msgHdr.fname, 25);
            buf[25] = 0;
            break;
        case 12:
            memcpy(buf, message.msgHdr.subj, 25);
            buf[25] = 0;
            break;
        case 13:
            memcpy(buf, message.msgHdr.pwd, 12);
            buf[12] = 0;
            break;
        case 14:
            itoa((int)message.msgHdr.active, buf, 10);
            break;
        case 15:
            buf[0] = message.msgHdr.echo;
            buf[1] = 0;
            break;
        default:
            buf[0] = 0;
            break;
    }

    return new cVARVAL(vtSTRING, buf);
}


/*===========================================================================
 * SetMsgHdrSub — implementation of SETMSGHDR(conf, message, field, value).
 *
 * Per WHATSNEW 15.4:
 *   "This function allows you to modify the above fields in a message."
 *   Writable fields:
 *     1 = 'To' field       (tname)
 *     2 = 'From' field     (fname)
 *     3 = 'Subject' field  (subj)
 *     4 = 'Password' field (pwd)
 *     5 = 'Echo' Flag      (echo)
 *
 * Note field numbers here differ from GetMsgHdr — WHATSNEW's SET set is
 * a smaller writable subset with its own 1..5 numbering.
 *
 * Returns 1 on success, 0 on failure.  Sysop-level access should be
 * enforced by the caller; this helper does not check ActStatus.
 *===========================================================================*/

cVARVAL * LIBENTRY cSCRIPT::SetMsgHdrSub(cVARVAL *p1, cVARVAL *p2,
                                        cVARVAL *p3, cVARVAL *p4)
{
    pcbconftype confrec;

    cVARVAL int1(vtINTEGER);
    cVARVAL i2(vtDREAL);
    cVARVAL int3(vtINTEGER);
    cVARVAL str4(vtSTRING);

    int1 = *p1;
    i2   = *p2;
    int3 = *p3;
    str4 = *p4;

    if (int1.values.vINTEGER > PcbData.NumConf ||
        int1.values.vINTEGER < 0)
        return new cVARVAL(vtINTEGER, (long)0);

    getconfrecord((unsigned)int1.values.vINTEGER, &confrec);

    if (message.open(confrec.MsgFile) != 0)
        return new cVARVAL(vtINTEGER, (long)0);

    if (message.seekIdx((long)i2.values.vDREAL) != 0) {
        message.close();
        return new cVARVAL(vtINTEGER, (long)0);
    }

    long cur = message.readIdx();
    if (cur == -1) {
        message.close();
        return new cVARVAL(vtINTEGER, (long)0);
    }

    message.readHdr(message.idxRec.off);

    char *val = str4.values.vSTRING;
    switch (int3.values.vINTEGER) {
        case 1:  /* To */
            memset(message.msgHdr.tname, ' ', 25);
            {
                int n = strlen(val);
                if (n > 25) n = 25;
                memcpy(message.msgHdr.tname, val, n);
            }
            break;
        case 2:  /* From */
            memset(message.msgHdr.fname, ' ', 25);
            {
                int n = strlen(val);
                if (n > 25) n = 25;
                memcpy(message.msgHdr.fname, val, n);
            }
            break;
        case 3:  /* Subject */
            memset(message.msgHdr.subj, ' ', 25);
            {
                int n = strlen(val);
                if (n > 25) n = 25;
                memcpy(message.msgHdr.subj, val, n);
            }
            break;
        case 4:  /* Password */
            memset(message.msgHdr.pwd, ' ', 12);
            {
                int n = strlen(val);
                if (n > 12) n = 12;
                memcpy(message.msgHdr.pwd, val, n);
            }
            break;
        case 5:  /* Echo flag — single byte, use first char of value */
            message.msgHdr.echo = val[0] ? val[0] : ' ';
            break;
        default:
            message.close();
            return new cVARVAL(vtINTEGER, (long)0);
    }

    /* Persist the modified header. */
    int rv = message.writeHdr(message.idxRec.off);
    return new cVARVAL(vtINTEGER, (long)(rv == 0 ? 1 : 0));
}


/*===========================================================================
 * pplmovemsg — implementation of MOVEMSG conf, message, movetype.
 *
 * Per WHATSNEW 15.4:
 *   "Moves the message from its current location to the end of the
 *    message base."
 * movetype: TRUE = move (delete source), FALSE = copy.
 *
 * This is the helper called from SCREXEC.CPP's TOK_MOVEMSG dispatch.
 * Returns 0 on success, non-zero on failure.
 *
 * The "move" operation is done as: read header at source position,
 * append a copy at the end (getting a new msg number), then if
 * movetype is TRUE mark the source header as deleted (active = 0
 * per the header layout).  This matches Clark's msg-base convention
 * where deleted messages are not physically removed but flagged.
 *===========================================================================*/

int LIBENTRY cSCRIPT::pplmovemsg(unsigned conf, long msgnum, int movetype)
{
    pcbconftype confrec;

    if (conf > PcbData.NumConf) {
        fastprint(0, 24, "Conference does not exist (MOVEMSG)", 0x70);
        return -1;
    }

    getconfrecord(conf, &confrec);

    if (message.open(confrec.MsgFile) != 0) {
        fastprint(0, 24, "Unable to read message header (MOVEMSG)", 0x70);
        return -2;
    }

    if (message.seekIdx(msgnum) != 0) {
        message.close();
        fastprint(0, 24, "Message number does not exist (MOVEMSG)", 0x70);
        return -3;
    }

    long cur = message.readIdx();
    if (cur == -1) {
        message.close();
        fastprint(0, 24, "Message number does not exist (MOVEMSG)", 0x70);
        return -3;
    }

    long srcOff = message.idxRec.off;
    if (message.readHdr(srcOff) != 0) {
        message.close();
        fastprint(0, 24, "Unable to read message header (MOVEMSG)", 0x70);
        return -4;
    }

    /* Save the source header for re-appending at the end. */
    sMSGHDR saved = message.msgHdr;

    /* Read the body into the msg buffer so we can rewrite it at the end. */
    if (message.readBody() != 0) {
        message.close();
        fastprint(0, 24, "Unable to read message body (MOVEMSG)", 0x70);
        return -5;
    }

    /* Get a new high message number and write the copy at the end.
     * writeHdr(-1) appends at end of file per the cMSG API contract.
     */
    saved.number = message.highMsgNum() + 1;
    message.msgHdr = saved;
    (void) message.writeHdr(-1);

    /* If move (not copy), mark the source header as deleted.
     * Clark's convention: msgHdr.active = 0 means deleted.
     */
    if (movetype) {
        (void) message.readHdr(srcOff);
        message.msgHdr.active = 0;
        (void) message.writeHdr(srcOff);
    }

    message.close();
    return 0;
}



cVARVAL * LIBENTRY cSCRIPT::evalPostFix(void)
{

    int err, c,loop;
    sint token;              // DWT:  changed int to sint

    cVARVAL * d, * v, * fpptr;

    cVARVAL * p1, * p2, * p3, * p4, * rp = NULL;


    static int errFlag = FALSE;

    long     b1, b2, b3;

    cVARVAL  b(vtBOOLEAN);
    cVARVAL  i(vtINTEGER);
    cVARVAL  s(vtSTRING);
    cVARVAL  r(vtDREAL);
    cVARVAL bs(vtBIGSTR);

    unsigned char _FAR_ * tmpscrptr;

    checkstack();
    varStack = new tVS(DD_YES,FALSE);

    if (varStack == NULL)
    {
        scriptErr(0,-1,SCR_ERR_MEM,"EVALUATING EXPRESSION");
        varStack = varStkStk.pop();
        varStkStk.push(varStack);
        if(Status.SysopFlag == ' ')
        {
          Status.SysopFlag = (Status.SysopFlag == 'R' ? ' ' : 'R');
          makepcboardsys();
        }
        return NULL;
    }

    varStkStk.push(varStack);

    err = FALSE;

    token = *((sint _FAR_ *) scriptPointer);      // DWT:  changed int to sint
    while (token != 0)
    {
        if (token > 0)
        {
          d = curVarSubPtr();

          if (d == NULL)
          {
                  err = TRUE;
                  break;
          }
          // If pushflag is true that means that a PROCEDURE call needs to
          // save D bacause it is the actual parameter with a VAR
          // storage class. It can then be copied back.
 //         if(pushFlag)
 //         {
 //            err = prgProcGlobVarStack.push(d);
 //            pushFlag = FALSE;
 //         }

          // if d->type = vtFUNCTION get params push onto stack
          // make recursive call
          int delflag = FALSE;
          if(d->type == vtFUNCTION)
          {

            fpptr    = d;
            currProcStack.push(currProc);
            currProc = fpptr;

          // Check for possible stack overflow
          // NOTE:  TurboC does not support stackavail() - only BorlandC.
          #ifdef __BORLANDC__
            if(STACK_CEILING >= stackavail() && !breakFlag)
            {
              //scriptErr(0,-1,SCR_ERR_SOFLOW,""); This needs to wait till SOFLOW can be added to PCBTEXT

              if(Status.CurSecLevel >= PcbData.SysopSec[SEC_SYSOPLEVEL])
              {
                int ccolor = curcolor();
                printcolor(0xC);
                println("Stack overflow error. Possibly too many recursive function calls");
                printcolor(ccolor);
              }

              sprintf(buf,"%s%s%s","Stack overflow running ",PPEName,".PPE.");
              writelog(buf,SPACERIGHT);
              sprintf(buf,"%s","Please notify PPE author.");
              writelog(buf,SPACERIGHT);
              stackErrFlag = TRUE;
              err          = TRUE;

              if(stackErrContFlag)
                breakFlag = TRUE;

              break;
            }
          #endif

          // Evaluate parameters and put those values into varLst in
          // the temporary variable slot for that parameter
          int sidno = d->values.vFUNCTION.firstId;
          err       = stkinit(fpptr);
          if(err)
                break;
          for(int ii = 0;ii<fpptr->values.vFUNCTION.params;ii++,sidno++)
                  EVAL(*varLst[sidno]->data,v,nullErrBreak);

          // Save current code pointer, set offset to point to function code
          // save current status on stack
          // make recursive call to execute.
          tmpscrptr     = (unsigned char _FAR_ *) (scriptPointer+sizeof(sint));  // DWT:  changed int to sint
          scriptPointer = (scriptBuffer+(((fpptr->values.vPROCEDURE.offset))))/*-sizeof(sint)*/;  // DWT:  changed int to sint

          initLocals(fpptr);

          if(err)
                break;
          execute(NULL);
          if(breakFlag)
              numCalls--;
          currProc = currProcStack.pop();

          //if(breakFlag)
          // err = TRUE;

          // return to old code offset
          // put return value into d
          // Re-instate old status from stack
          if(!breakFlag)
          {
            d = new cVARVAL (*varLst[fpptr->values.vFUNCTION.retId-1]->data);
            if(d)
              delflag = TRUE;
          }
          stkclean(fpptr);

          scriptPointer = (char _FAR_ *) (tmpscrptr -sizeof(sint));  // DWT:  changed int to sint
          if (fpptr == NULL || d == NULL)
          {
                scriptErr(0,-1,SCR_ERR_MEM,"EVALUATING SUB-EXPRESSION");
                err = TRUE;
                break;
          }
        }

        if(breakFlag)
          break;

        v = new cVARVAL(*d);

        if(delflag)
        {
          delete d;
          delflag = FALSE;
        }

        if (v == NULL)
        {
                scriptErr(0,-1,SCR_ERR_MEM,"EVALUATING SUB-EXPRESSION");
                err = TRUE;
                break;
        }
        varStack->push(v);
      }
      else
      {
        err = FALSE;
        p1 = NULL;      p2 = NULL;      p3 = NULL; p4 = NULL; rp = NULL;
        v1 = FALSE; v2 = FALSE; v3 = FALSE; v4 = NULL;
        f1 = FALSE; f2 = FALSE; f3 = FALSE; f4 = NULL;

        if (!varStack->isEmpty()) p1 = varStack->pop(), v1 = TRUE;
        if (!varStack->isEmpty()) p2 = varStack->pop(), v2 = TRUE;
        if (!varStack->isEmpty()) p3 = varStack->pop(), v3 = TRUE;
        if (!varStack->isEmpty()) p4 = varStack->pop(), v4 = TRUE;

#ifdef ERRBRK
            if (v1) nullErrBreak(p1);
            if (v2) nullErrBreak(p2);
            if (v3) nullErrBreak(p3);
            if (v4) nullErrBreak(p4);
#endif

            switch (*((sint _FAR_ *) scriptPointer))   // DWT:  changed int to sint
            {

             case TOK_OP_FTELL:
                  falseErrBreak(v1);

                  i = *p1;
                  c = int(i.values.vINTEGER % MAX_FILE_CHAN);


                  rp = new cVARVAL(vtINTEGER,dosfseek(&fileArr[c].hand,0,SEEK_CUR));

                  f1 = TRUE;
                  break;

             case TOK_OP_OS:

                  #if defined (__OS2__)

                    rp = new cVARVAL(vtINTEGER,2);

                  #elif defined(__MSDOS__)

                    rp = new cVARVAL(vtINTEGER,1);

                  #else

                    rp = new cVARVAL(vtINT,0);

                  #endif
                  break;

             {
             bassngl  bsngle;
             basdble  bdble;



             case TOK_OP_BS2I:
                  falseErrBreak(v1);

                  tmpBigStr = *p1;

                  memcpy(bsngle,tmpBigStr.values.vBIGSTR.ptr,sizeof(bsngle));

                  rp = new cVARVAL(vtINTEGER,bassngltolong(bsngle));
                  f1 = TRUE;
                  break;

             case TOK_OP_BD2I:
                  falseErrBreak(v1);

                  tmpBigStr = *p1;

                  memcpy(bdble,tmpBigStr.values.vBIGSTR.ptr,sizeof(bdble));

                  rp = new cVARVAL(vtINTEGER,basdbletolong(bdble));
                  f1 = TRUE;
                  break;

             case TOK_OP_I2BS:
                  falseErrBreak(v1);

                  tmpInteger = *p1;

                  longtobassngl(bsngle,tmpInteger.values.vINTEGER);

                  // Create a 4 byte empty string
                  rp = new cVARVAL(vtBIGSTR,"    ");

                  memcpy(rp->values.vBIGSTR.ptr,bsngle,sizeof(bsngle));
                  f1 = TRUE;
                  break;

             case TOK_OP_I2BD:
                  falseErrBreak(v1);

                  tmpInteger = *p1;

                  longtobasdble(bdble,tmpInteger.values.vINTEGER);

                  rp = new cVARVAL(vtBIGSTR,"        ");

                  memcpy(rp->values.vBIGSTR.ptr,bdble,sizeof(bdble));
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_SETDRIVE:

                  falseErrBreak(v1);
                  i = *p1;
                  rp = new cVARVAL(vtINTEGER,dossetcurdrive((int)i.values.vINTEGER+1));

                  f1 = TRUE;
                  break;

             case TOK_OP_GETDRIVE:
                  rp = new cVARVAL(vtINTEGER,dosgetcurdrive()-1);
                  break;

             case TOK_OP_CWD:
             {
                  char path[66];
                  dosgetcurpath(dosgetcurdrive(),path,sizeof(path));
                  rp = new cVARVAL(vtSTRING,path);
                  break;
             }

             case TOK_OP_INSTRR:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->instrr(*p1));
                  f1 = f2 = TRUE;
                  break;

#ifdef FIDO
             case TOK_OP_FDORDAKA:
             {
                  cAKAS     akas;
                  NADDRESS  addr;
                  char      nodestr[25];

                  falseErrBreak(v1);
                  i = *p1;
                  addr = akas.getRec((unsigned int)i.values.vINTEGER);

                  if(addr.zone == 0)
                  {
                     rp = new cVARVAL(vtSTRING,"");
                     break;
                  }

                  if(addr.point == 0) sprintf(nodestr,"%u:%u/%u",addr.zone,addr.net,addr.node);
                  else                sprintf(nodestr,"%u:%u/%u.%u",addr.zone,addr.net,addr.node,addr.point);
                  rp = new cVARVAL(vtSTRING,nodestr);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_FDORDORG:
             {
                  cORIGINS  origins;
                  ORIGIN    org;

                  falseErrBreak(v1);
                  i = *p1;
                  org = origins.getRec((unsigned int)i.values.vINTEGER);

                  if(org.origin[0] == '\x0')
                  {
                     rp = new cVARVAL(vtSTRING,"");
                     break;
                  }
                  rp = new cVARVAL(vtSTRING,org.origin);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_FDORDAREA:
             {
                  cAREAS          areas;
                  NAREA_STRUCT    area;

                  falseErrBreak(v1);
                  i = *p1;
                  area = areas.getAreaRec((unsigned int)i.values.vINTEGER);

                  if(area.ConfNum == 0)
                  {
                     rp = new cVARVAL(vtSTRING,"");
                     break;
                  }
                  rp = new cVARVAL(vtSTRING,area.AreaTag);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_FDOQRD:
                  break;

#endif // ifdef FIDO

             case TOK_OP_CONFINFO:
                  falseErrBreak(v2);

                  {
                    cVARVAL int1(vtINTEGER);
                    cVARVAL int2(vtINTEGER);
                    int1 = *p2;
                    int2 = *p1;

                    rp = getConfInfo((unsigned)int1.values.vINTEGER,(unsigned)int2.values.vINTEGER);

                    f1=f2 = TRUE;
                  }
                  break;

             case TOK_OP_USELMRS:
                  rp = new cVARVAL(vtBOOLEAN,(int) altUserLMRFlag);
                  break;

             {
             static struct ffblk ffblk;
             static int   dosfindmax;
             #ifdef __OS2__
             static int DirHandle;
             #endif

             case TOK_OP_FINDFIRST:
                  dosfindmax = 1;
                  #ifdef __OS2__
                  DirHandle = SYSTEMDIRHANDLE;
                  #endif

                  falseErrBreak(v1);
                  s = *p1;
                  if(dosfindfirst(s.values.vSTRING,&ffblk,FA_NORMAL|FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_ARCH,&dosfindmax PDIRHANDLE)==0)
                  {
                    sprintf(str1,"%-13s",ffblk.ff_name);
                    rp = new cVARVAL(vtSTRING,str1);
                  }
                  else
                    rp = new cVARVAL(vtSTRING,"");

                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_FINDNEXT:
                  if(dosfindnext(&ffblk,&dosfindmax PDIRHANDLE2)==0)
                  {
                    sprintf(str1,"%-13s",ffblk.ff_name);
                    rp = new cVARVAL(vtSTRING,str1);
                  }
                  else
                    rp = new cVARVAL(vtSTRING,"");
                  break;
             }
#ifndef LIB
             case TOK_OP_QWKLIMITS:
                  falseErrBreak(v1);

                  if (!QwkSupport)
                  {
                     rp = new cVARVAL(vtINTEGER,-1);
                     break;
                  }
                  b1 = *p1;
                  switch(b1)
                  {
                     case 0: rp = new cVARVAL(vtINTEGER,ptrUData->QwkConfig.MaxMsgs);
                                  break;
                     case 1: rp = new cVARVAL(vtINTEGER,ptrUData->QwkConfig.MaxMsgsPerConf);
                                  break;
                     case 2: rp = new cVARVAL(vtINTEGER,ptrUData->QwkConfig.PersonalAttachLimit);
                                  break;
                     case 3: rp = new cVARVAL(vtINTEGER,ptrUData->QwkConfig.PublicAttachLimit);
                                  break;
                     default:rp = new cVARVAL(vtINTEGER,-1);
                                  break;
                  }
                  f1 = TRUE;
                  break;
#endif
             case TOK_OP_RIPVER:

#ifndef LIB
                  rp = new cVARVAL(vtSTRING,Status.RipVersion);
#else
                  rp = new cVARVAL(vtSTRING,"0");
#endif
                  break;


             case TOK_OP_CHECKRIP:

#ifndef LIB
                  checkforrip();
                  rp = new cVARVAL(vtBOOLEAN,(int) HasRip);
#else
                  rp = new cVARVAL(vtBOOLEAN,0);
#endif
                  break;

#ifndef LIB
             case TOK_OP_SCANMSGHDR:
                  falseErrBreak(v4);
                  rp = ScanMsgHdrSub(p1,p2,p3,p4);
                  f1 = f2 = f3 = f4 = TRUE;
                  message.close();
                  break;

             case TOK_OP_ACCOUNT:    //... EXPR ...
                  falseErrBreak(v1);
                  rp = AccountSub(p1);
                  f1 = TRUE;
                  break;

             case TOK_OP_PCBACCSTAT:
             {
                  falseErrBreak(v1);
                  cVARVAL Stat1(vtINTEGER);
                  Stat1 = *p1;
                  switch (Stat1.values.vINTEGER)
                  {
                      case 0:
                        rp = new cVARVAL(vtINTEGER,Status.ActStatus);
                        break;
                      case 1:
                        rp = new cVARVAL(vtDREAL,Status.CurConf.ChargeTime);
                        break;
                      case 2:
                        rp = new cVARVAL(vtDREAL,Status.CurConf.ChargeMsgRead);
                        break;
                      case 3:
                        rp = new cVARVAL(vtDREAL,Status.CurConf.ChargeMsgWrite);
                        break;
                      case 4:
                        calculatebalance();
                        rp = new cVARVAL(vtDREAL,Status.Balance);
                        break;
                      default:
                        rp = new cVARVAL(vtDREAL,-1);
                        break;

                  }
                  f1 = TRUE;
                  }
                  break;

             case TOK_OP_PCBACCOUNT:       //... EXPR ...
                  falseErrBreak(v1);
                  rp = PcbAccountSub(p1);
                  f1 = TRUE;
                  break;
#endif //ifdef LIB

#ifdef DBASE
            case TOK_OP_DERRMSG:
            case TOK_OP_DCHKSTAT:
            case TOK_OP_DADD:
            case TOK_OP_DSETALIAS:
            case TOK_OP_DAPPEND:
            case TOK_OP_DBLANK:
            case TOK_OP_DBOTTOM:
            case TOK_OP_DCLOSE:
            case TOK_OP_DCLOSEALL:
            case TOK_OP_DDELETE:
            case TOK_OP_DFBLANK:
            case TOK_OP_DFCOPY:
            case TOK_OP_DGET:
            case TOK_OP_DGO:
            case TOK_OP_DLOCK:         //             TOK_DLOCKF same token
            case TOK_OP_DLOCKF:
            case TOK_OP_DLOCKR:
            case TOK_OP_DNCLOSE:
            case TOK_OP_DNCLOSEALL:
            case TOK_OP_DNEW:
            case TOK_OP_DNOPEN:
            case TOK_OP_DOPEN:
            case TOK_OP_DPACK:
            case TOK_OP_DPUT:
            case TOK_OP_DRECALL:
            case TOK_OP_DSEEK:
            case TOK_OP_DSKIP:
            case TOK_OP_DTAG:
            case TOK_OP_DTOP:
            case TOK_OP_DUNLOCK:
            case TOK_OP_DNEXT:
            case TOK_OP_DGETALIAS:
            case TOK_OP_DBOF:
            case TOK_OP_DCHANGED:
            case TOK_OP_DDECIMALS:
            case TOK_OP_DDELETED:
            case TOK_OP_DEOF:
            case TOK_OP_DERR:
            case TOK_OP_DFIELDS:
            case TOK_OP_DLENGTH:
            case TOK_OP_DNAME:
            case TOK_OP_DRECCOUNT:
            case TOK_OP_DRECNO:
            case TOK_OP_DTYPE:
            case TOK_OP_DSELECT:
                 execdbf(p1,p2,p3,p4,rp,err);
                 break;
#endif  // ifdef DBASE

             case TOK_OP_FNEXT:
                  rp = NULL;
                  for(loop = 0;loop<MAX_FILE_CHAN;loop++)
                  {
                     if(!fileArr[loop].inUse)
                     {
                         rp = new cVARVAL(vtINTEGER,loop);
                         break;
                     }
                  }
                  if(rp == NULL)
                    rp = new cVARVAL(vtINTEGER,-1);
                  break;

             case TOK_OP_STACKERR:
                  rp = new cVARVAL(vtBOOLEAN,(int) stackErrFlag);
                  break;

             case TOK_OP_STACKLEFT:
                  // TurboC does not support stackavail(), return a
                  // positive non-zero value
                  #ifdef __BORLANDC__
                    rp = new cVARVAL(vtINTEGER,stackavail());
                  #else
                    rp = new cVARVAL(vtINTEGER,0x1000);
                  #endif
                  break;

             case TOK_OP_PCBMAC:
                  falseErrBreak(v1);
                  s = *p1;
                  xlatetext(buf,s.values.vSTRING);
                  rp = new cVARVAL(vtBIGSTR,buf);
                  f1 = TRUE;
                  break;

             case TOK_OP_ACTMSGNUM:
                  msgbasestattype  msgstats;
                  msgstats.NumActiveMsgs = 0;
                  msgbasestats(&msgstats);
                  rp = new cVARVAL (vtUNSIGNED,msgstats.NumActiveMsgs);
                  break;
#ifndef LIB
             case TOK_OP_CRC32:
                  falseErrBreak(v2);
                  b = *p2;
                  bs = *p1;
                  rp = new cVARVAL (vtUNSIGNED,calcCRC32(b.values.vBOOLEAN,nul2mty(bs),bs.values.vBIGSTR.len));
                  f1 = f2 = TRUE;
                  break;
#endif
             case TOK_OP_HICONFNUM:
                  rp = new cVARVAL (vtINTEGER, PcbData.NumConf);
                  break;

             case TOK_OP_INBYTES:
#ifdef COMM
                  rp = new cVARVAL (vtINTEGER, InBytes);
#else
                  rp = new cVARVAL (vtINTEGER,0);
#endif
                  break;

             case TOK_OP_OUTBYTES:
#ifdef COMM
                  rp = new cVARVAL (vtINTEGER,OutBytes);
#else
                  rp = new cVARVAL (vtINTEGER,0);
#endif
                  break;

             case TOK_OP_DRIVESPACE:
                  falseErrBreak(v1);
                  s = *p1;
                  rp = new cVARVAL (vtINTEGER, diskfreespace(nul2mty(s)));
                  f1 = TRUE;
                  break;

             case TOK_OP_LEN:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->len());
                  f1 = TRUE;
                  break;

             case TOK_OP_LOWER:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->lower());
                  f1 = TRUE;
                  break;

             case TOK_OP_UPPER:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->upper());
                  f1 = TRUE;
                  break;

             case TOK_OP_MIXED:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->mixed());
                  f1 = TRUE;
                  break;

             case TOK_OP_MID:
                  falseErrBreak(v3);
                  rp = new cVARVAL(p3->mid(*p2,*p1));
                  f1 = f2 = f3 = TRUE;
                  break;

             case TOK_OP_LEFT:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->left(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_RIGHT:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->right(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_SPACE:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->space());
                  f1 = TRUE;
                  break;

             case TOK_OP_FERR:
                  falseErrBreak(v1);

                  i = *p1;
                  c = int(i.values.vINTEGER % MAX_FILE_CHAN);

                  rp = new cVARVAL(vtBOOLEAN,(int) fileArr[c].errStat);
                  fileArr[c].errStat = FALSE;

                  f1 = TRUE;
                  break;

             case TOK_OP_CHR:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->chr());
                  f1 = TRUE;
                  break;

             case TOK_OP_ASC:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->asc());
                  f1 = TRUE;
                  break;

             case TOK_OP_INSTR:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->instr(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_ABORT:
                  rp = new cVARVAL(vtINTEGER,(int) Display.AbortPrintout);
                  break;

             case TOK_OP_LTRIM:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->ltrim(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_REPLACE:
                  falseErrBreak(v3);
                  rp = new cVARVAL(p3->replace(*p2,*p1));
                  f1 = f2 = f3 = TRUE;
                  break;

             case TOK_OP_STRIP:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->strip(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_REPSTR:
                  falseErrBreak(v3);
                  rp = new cVARVAL(p3->replacestr(*p2,*p1));
                  f1 = f2 = f3 = TRUE;
                  break;

             case TOK_OP_STRSTR:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->stripstr(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_RTRIM:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->rtrim(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_TRIM:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->trim(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_RANDOM:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->random());
                  f1 = TRUE;
                  break;

             case TOK_OP_DATE:
                  rp = new cVARVAL(vtDATE,getjuliandate());
                  break;

             case TOK_OP_TIME:
                  rp = new cVARVAL(vtTIME,exacttime());
                  break;

             case TOK_OP_U_NAME:
                  rp = new cVARVAL(vtSTRING,ptrUData->Name);
                  break;

             case TOK_OP_U_LDATE:
                  rp = new cVARVAL(vtDATE,ptrUData->LastDateOn);
                  break;

             case TOK_OP_U_LTIME:
                  signed long h, m;
                  h = strtol(ptrUData->LastTimeOn+0,NULL,10);
                  m = strtol(ptrUData->LastTimeOn+3,NULL,10);
                  rp = new cVARVAL(vtTIME,h*60L*60L+m*60L);
                  break;

             case TOK_OP_U_LDIR:
                  sprintf(buf,"%02d/%02d/%02d",
                          DLDRMonth(),
                          DLDRDay(),
                          DLDRYear());
                  rp = new cVARVAL(vtDATE,datetojulian(buf));
                  break;

             case TOK_OP_U_LMR:
                  falseErrBreak(v1);
                  i = *p1;

                  if(ptrULMR) rp = new cVARVAL(vtINTEGER,
                               ptrULMR[unsigned(i.values.vINTEGER)]);
                  else
                               rp = new cVARVAL(vtINTEGER,0);
                  f1 = TRUE;
                  break;

             case TOK_OP_U_LOGONS:
                  rp = new cVARVAL(vtINTEGER,ptrUData->NumTimesOn);
                  break;

             case TOK_OP_U_FUL:
                  rp = new cVARVAL(vtINTEGER,ptrUData->NumUploads);
                  break;

             case TOK_OP_U_FDL:
                  rp = new cVARVAL(vtINTEGER,ptrUData->NumDownloads);
                  break;

             case TOK_OP_U_BDLDAY:
                  rp = new cVARVAL(vtINTEGER,ptrUData->DailyDnldBytes);
                  break;

             case TOK_OP_U_TIMEON:
                  rp = new cVARVAL(vtINTEGER,ptrUData->ElapsedTimeOn);
                  break;

             case TOK_OP_U_BDL:
                  //rp = new cVARVAL(vtINTEGER,ptrUData->TotDnldBytes);
                  rp = new cVARVAL(vtDREAL,ptrUData->TotDnldBytes);
                  break;

             case TOK_OP_U_BUL:
                  //rp = new cVARVAL(vtINTEGER,ptrUData->TotUpldBytes);
                  rp = new cVARVAL(vtDREAL,ptrUData->TotUpldBytes);
                  break;

             case TOK_OP_U_MSGRD:
                  //rp = new cVARVAL(vtINTEGER,ptrUData->MsgsRead);
                  rp = new cVARVAL(vtUNSIGNED,ptrUData->MsgsRead);
                  break;

             case TOK_OP_U_MSGWR:
                  rp = new cVARVAL(vtINTEGER,ptrUData->MsgsLeft);
                  break;

             case TOK_OP_YEAR:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->year());
                  f1 = TRUE;
                  break;

             case TOK_OP_MONTH:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->month());
                  f1 = TRUE;
                  break;

             case TOK_OP_DAY:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->day());
                  f1 = TRUE;
                  break;

             case TOK_OP_DOW:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->dow());
                  f1 = TRUE;
                  break;

             case TOK_OP_HOUR:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->hour());
                  f1 = TRUE;
                  break;

             case TOK_OP_MIN:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->min());
                  f1 = TRUE;
                  break;

             case TOK_OP_SEC:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->sec());
                  f1 = TRUE;
                  break;

             case TOK_OP_TIMEAP:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->timeap());
                  f1 = TRUE;
                  break;

             case TOK_OP_VER:
                  rp = new cVARVAL(vtINTEGER,SCR_VER);
                  break;

             case TOK_OP_NOCHAR:
                  buf[0] = NoChar;
                  buf[1] = '\0';
                  rp = new cVARVAL(vtSTRING,buf);
                  break;

             case TOK_OP_YESCHAR:
                  buf[0] = YesChar;
                  buf[1] = '\0';
                  rp = new cVARVAL(vtSTRING,buf);
                  break;

             case TOK_OP_STRIPATX:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->stripatx());
                  f1 = TRUE;
                  break;

             case TOK_OP_TOSTRING:
                  falseErrBreak(v1);
                  s = *p1;
                  rp = new cVARVAL(s);
                  f1 = TRUE;
                  break;

             case TOK_OP_TOBIGSTR:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtBIGSTR);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOBOOLEAN:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtBOOLEAN);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOBYTE:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtBYTE);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TODATE:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtDATE);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TODDATE:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtDDATE);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TODREAL:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtDREAL);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOEDATE:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtEDATE);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOINTEGER:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtINTEGER);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOMONEY:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtMONEY);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOREAL:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtREAL);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOSBYTE:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtSBYTE);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOSWORD:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtSWORD);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOTIME:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtTIME);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOUNSIGNED:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtUNSIGNED);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_TOWORD:
             {
                  falseErrBreak(v1);
                  cVARVAL v(vtWORD);
                  v = *p1;
                  rp = new cVARVAL(v);
                  f1 = TRUE;
                  break;
             }

             case TOK_OP_MASKAN:
                  rp = new cVARVAL(vtSTRING,expandMask(mask_an));
                  break;

             case TOK_OP_MASKALF:
                  rp = new cVARVAL(vtSTRING,expandMask(mask_alf));
                  break;

             case TOK_OP_MASKASC:
                  rp = new cVARVAL(vtSTRING,expandMask(mask_alphanum));
                  break;

             case TOK_OP_MASKFIL:
                  rp = new cVARVAL(vtSTRING,expandMask(mask_filename));
                  break;

             case TOK_OP_MASKNUM:
                  rp = new cVARVAL(vtSTRING,expandMask(mask_num));
                  break;

             case TOK_OP_MASKPTH:
                  strcpy(buf,":\\");
                  strcat(buf,expandMask(mask_filename));
                  rp = new cVARVAL(vtSTRING,buf);
                  break;

             case TOK_OP_MASKPWD:
             {
                   char * ptr = (PcbData.DisableEdits ? mask_message : mask_alphanum);
                   rp = new cVARVAL(vtSTRING,expandMask(ptr));
             }
                   break;

             case TOK_OP_CURCONF:
                  rp = new cVARVAL(vtINTEGER,Status.Conference);
                  break;

             case TOK_OP_PCBDAT:
                  rp = new cVARVAL(vtSTRING,DatFile);
                  break;

             case TOK_OP_PPENAME:
                  rp = new cVARVAL(vtSTRING,PPEName);
                  break;

             case TOK_OP_PPEPATH:
                  rp = new cVARVAL(vtSTRING,PPEPath);
                  break;

             case TOK_OP_VALDATE:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->valDate());
                  f1 = TRUE;
                  break;

             case TOK_OP_VALTIME:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->valTime());
                  f1 = TRUE;
                  break;

             case TOK_OP_PCBNODE:
                  rp = new cVARVAL(vtINTEGER,PcbData.NodeNum);
                  break;

             case TOK_OP_READLINE:
                  falseErrBreak(v2);
                  s = *p2;
                  i = *p1;
                  rp = new cVARVAL(vtBIGSTR,readLine(nul2mty(s),i.values.vINTEGER,buf));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_SYSOPSEC:
                  rp = new cVARVAL(vtINTEGER,PcbData.SysopSec[SEC_SYSOPLEVEL]);
                  break;

             case TOK_OP_ONLOCAL:
                  rp = new cVARVAL(vtBOOLEAN,(int) (Asy.Online == LOCAL));
                  break;

             case TOK_OP_UN_STAT:
                  buf[0] = unInfo.Status;
                  buf[1] = '\0';
                  rp = new cVARVAL(vtSTRING,buf);
                  break;

             case TOK_OP_UN_NAME:
                  rp = new cVARVAL(vtSTRING,unInfo.Name);
                  break;

             case TOK_OP_UN_CITY:
                  rp = new cVARVAL(vtSTRING,unInfo.City);
                  break;

             case TOK_OP_UN_OPER:
                  rp = new cVARVAL(vtSTRING,unInfo.Operation);
                  break;

             case TOK_OP_CURSEC:
                  rp = new cVARVAL(vtINTEGER,Status.CurSecLevel);
                  break;

             case TOK_OP_GETENV:
                  falseErrBreak(v1);
                  s = *p1;
                  rp = new cVARVAL(vtSTRING,getenv(nul2mty(s.upper())));
                  f1 = TRUE;
                  break;

             case TOK_OP_GETTOKEN:
                  rp = new cVARVAL(vtSTRING,(tokenCnt-- > 0) ? getnexttoken() : "");
                  if (tokenCnt < 0) tokenCnt = 0;
                  break;

             case TOK_OP_MINLEFT:
                  rp = new cVARVAL(vtINTEGER,minutesleft());
                  break;

             case TOK_OP_MINON:
                  rp = new cVARVAL(vtINTEGER,minutesonline());
                  break;

             case TOK_OP_CALLID:
                  rp = new cVARVAL(vtSTRING,
#ifdef LIB
                  ""
#else
                  Asy.CallerIDString
#endif
                  );
                  break;

#if defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__)
             case TOK_OP_REGAL: case TOK_OP_REGAH:
             case TOK_OP_REGBL: case TOK_OP_REGBH:
             case TOK_OP_REGCL: case TOK_OP_REGCH:
             case TOK_OP_REGDL: case TOK_OP_REGDH:
             case TOK_OP_REGAX: case TOK_OP_REGBX:
             case TOK_OP_REGCX: case TOK_OP_REGDX:
             case TOK_OP_REGSI: case TOK_OP_REGDI:
             case TOK_OP_REGF:  case TOK_OP_REGCF:
             case TOK_OP_REGDS: case TOK_OP_REGES:
                  err = unsupported("REG*() function");
                  break;
#else
             case TOK_OP_REGAL:
                  rp = new cVARVAL(vtINTEGER,intrREGS.h.al);
                  break;

             case TOK_OP_REGAH:
                  rp = new cVARVAL(vtINTEGER,intrREGS.h.ah);
                  break;

             case TOK_OP_REGBL:
                  rp = new cVARVAL(vtINTEGER,intrREGS.h.bl);
                  break;

             case TOK_OP_REGBH:
                  rp = new cVARVAL(vtINTEGER,intrREGS.h.bh);
                  break;

             case TOK_OP_REGCL:
                  rp = new cVARVAL(vtINTEGER,intrREGS.h.cl);
                  break;

             case TOK_OP_REGCH:
                  rp = new cVARVAL(vtINTEGER,intrREGS.h.ch);
                  break;

             case TOK_OP_REGDL:
                  rp = new cVARVAL(vtINTEGER,intrREGS.h.dl);
                  break;

             case TOK_OP_REGDH:
                  rp = new cVARVAL(vtINTEGER,intrREGS.h.dh);
                  break;

             case TOK_OP_REGAX:
                  rp = new cVARVAL(vtINTEGER,intrREGS.w.ax);
                  break;

             case TOK_OP_REGBX:
                  rp = new cVARVAL(vtINTEGER,intrREGS.w.bx);
                  break;

             case TOK_OP_REGCX:
                  rp = new cVARVAL(vtINTEGER,intrREGS.w.cx);
                  break;

             case TOK_OP_REGDX:
                  rp = new cVARVAL(vtINTEGER,intrREGS.w.dx);
                  break;

             case TOK_OP_REGSI:
                  rp = new cVARVAL(vtINTEGER,intrREGS.w.si);
                  break;

             case TOK_OP_REGDI:
                  rp = new cVARVAL(vtINTEGER,intrREGS.w.di);
                  break;

             case TOK_OP_REGF:
                  rp = new cVARVAL(vtINTEGER,intrREGS.w.cflag);
                  break;

             case TOK_OP_REGCF:
                  rp = new cVARVAL(vtBOOLEAN,intrREGS.w.cflag);
                  break;

             case TOK_OP_REGDS:
                  rp = new cVARVAL(vtINTEGER,intrSEGS.ds);
                  break;

             case TOK_OP_REGES:
                  rp = new cVARVAL(vtINTEGER,intrSEGS.es);
                  break;
#endif

             case TOK_OP_B2W:
                  falseErrBreak(v2);
                  i = *p1;
                  b1 = tBYTE(i.values.vINTEGER & 0x000000FF);
                  i = *p2;
                  b2 = tBYTE(i.values.vINTEGER & 0x000000FF);
                  rp = new cVARVAL(vtINTEGER,tWORD(b1)+(tWORD(b2)<<8));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_PEEKB:
#if defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__)
                  unsupported("PEEK() or PEEKB() function");
#endif
                  falseErrBreak(v1);
                  i = *p1;
                  rp = new cVARVAL(vtINTEGER,
#if defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__)
                       0
#else
                       *(tBYTE _FAR_ *)(i.values.vINTEGER)
#endif
                       );
                  f1 = TRUE;
                  break;

             case TOK_OP_PEEKW:
#if defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__)
                  unsupported("PEEKW() function");
#endif
                  falseErrBreak(v1);
                  i = *p1;
                  rp = new cVARVAL(vtINTEGER,
#if defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__)
                       0
#else
                       *(tWORD _FAR_ *)(i.values.vINTEGER)
#endif
                       );
                  f1 = TRUE;
                  break;

             case TOK_OP_PEEKDW:
#if defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__)
                  unsupported("PEEKDW() function");
#endif
                  {
                  falseErrBreak(v1);
                  i = *p1;

#if ! (defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__))
                  unsigned dwseg = unsigned(i.values.vINTEGER >> 16);
                  unsigned dwoff = unsigned(i.values.vINTEGER & 0x0000FFFFL);

                  _ES = dwseg;
                  _BX = dwoff;

                  // asm les bx,es:[bx]
                  __emit__(0x26,0xC4,0x1F);

                  dwseg = _ES;
                  dwoff = _BX;
#endif

                  rp = new cVARVAL(vtINTEGER,
#if defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__)
                       0
#else
                       (tINTEGER(dwseg)<<16)+tINTEGER(dwoff)
#endif
                       );

                  f1 = TRUE;
                  }
                  break;

             case TOK_OP_MKADDR:
#if defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__)
                  unsupported("MKADDR() function");
#endif
                  falseErrBreak(v2);
                  i = *p1;
                  b1 = tWORD(i.values.vINTEGER & 0x0000FFFF);
                  i = *p2;
                  b2 = tWORD(i.values.vINTEGER & 0x0000FFFF);
                  rp = new cVARVAL(vtINTEGER,
#if defined(PCB_DEMO) || defined(__OS2__) || defined(__WATCOMC__)
                       0
#else
                       tINTEGER(b1)+(tINTEGER(b2)<<16)
#endif
                       );
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_MKDATE:
             {
                  falseErrBreak(v3);

                  i = *p3;
                  b1 = tWORD(i.values.vINTEGER & 0x0000FFFF);
                  c = int(b1-1900);
                  b2 = c*365L + (c-1)/4;

                  i = *p2;
                  b1 = tWORD(i.values.vINTEGER & 0x0000FFFF);

                  int daysInMonth[] = { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
                  b2 += daysInMonth[tWORD(b1)] +
                          (((c > 0) && ((c % 4) == 0) && (b1 > 2)) ? 1 : 0);

                  i = *p1;
                  b1 = tWORD(i.values.vINTEGER & 0x0000FFFF);
                  b2 += b1;

                  rp = new cVARVAL(vtDATE,b2);
                  f1 = f2 = f3 = TRUE;

                  break;
             }

             case TOK_OP_EXIST:
                  falseErrBreak(v1);
                  s = *p1;
                  rp = new cVARVAL(vtBOOLEAN,(int) fexist(nul2mty(s)));
                  f1 = TRUE;
                  break;

             case TOK_OP_MEGANUM:
                  falseErrBreak(v1);
                  i = *p1;
                  b1 = i.values.vINTEGER;
                  inRange(b1,0,1295);
                  rp = new cVARVAL(vtSTRING,meganum(int(b1)));
                  f1 = TRUE;
                  break;

             case TOK_OP_I2S:
                  falseErrBreak(v2);

                  i = *p1;
                  b1 = i.values.vINTEGER;
                  if (b1 <  2) b1 = 2;
                  if (b1 > 36) b1 = 36;

                  i = *p2;

                  rp = new cVARVAL(vtSTRING,strupr(ultoa(i.values.vINTEGER,buf,int(b1))));

                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_S2I:
                  falseErrBreak(v2);

                  i = *p1;
                  b1 = i.values.vINTEGER;
                  if (b1 <  2) b1 = 2;
                  if (b1 > 36) b1 = 36;

                  s = *p2;

                  rp = new cVARVAL(vtINTEGER,strtoul(nul2mty(s),NULL,int(b1)));

                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_CARRIER:
                  rp = new cVARVAL(vtINTEGER,Asy.CarrierSpeed);
                  break;

             case TOK_OP_TOKENSTR:  //  ***
                  *buf = *(buf+1) = '\0';
                  for (c = 0; c < tokenCnt; c++)
                          addchar(buf,';'),
                          strcat(buf,getnexttoken());
                  rp = new cVARVAL(vtSTRING,buf+1);
                  tokenCnt = 0;
                  break;

             case TOK_OP_CDON:
                  rp = new cVARVAL(vtBOOLEAN,
#ifdef COMM
                  online()
#else
                  (signed long int) FALSE
#endif
                  );
                  break;

             case TOK_OP_LANGEXT:
                  rp = new cVARVAL(vtSTRING,Status.MultiLangExt);
                  break;

             case TOK_OP_ANSION:
                  rp = new cVARVAL(vtBOOLEAN,(int) UseAnsi);
                  break;

             case TOK_OP_VALCC:
                  s = *p1;
                  rp = new cVARVAL(s.valCC());
                  f1 = TRUE;
                  break;

             case TOK_OP_FMTCC:
                  s = *p1;
                  rp = new cVARVAL(s.fmtCC());
                  f1 = TRUE;
                  break;

             case TOK_OP_CCTYPE:
                  s = *p1;
                  rp = new cVARVAL(s.ccType());
                  f1 = TRUE;
                  break;

             case TOK_OP_GETX:
                  rp = new cVARVAL(vtINTEGER,awherex()+1);
                  break;

             case TOK_OP_GETY:
                  rp = new cVARVAL(vtINTEGER,awherey()+1);
                  break;

             case TOK_OP_ISBITSET:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->bitStat(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_BAND:
                  falseErrBreak(v2);
                  i = *p1;
                  b1 = i.values.vINTEGER;
                  i = *p2;
                  b2 = i.values.vINTEGER;
                  rp = new cVARVAL(vtINTEGER,b2&b1);
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_BOR:
                  falseErrBreak(v2);
                  i = *p1;
                  b1 = i.values.vINTEGER;
                  i = *p2;
                  b2 = i.values.vINTEGER;
                  rp = new cVARVAL(vtINTEGER,b2|b1);
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_BXOR:
                  falseErrBreak(v2);
                  i = *p1;
                  b1 = i.values.vINTEGER;
                  i = *p2;
                  b2 = i.values.vINTEGER;
                  rp = new cVARVAL(vtINTEGER,b2^b1);
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_BNOT:
                  falseErrBreak(v1);
                  i = *p1;
                  b1 = i.values.vINTEGER;
                  rp = new cVARVAL(vtINTEGER,~b1);
                  f1 = TRUE;
                  break;

             case TOK_OP_CURUSER:
                  rp = new cVARVAL(vtINTEGER,lastUserGot);
                  break;

             case TOK_OP_U_PWDHST:
                  falseErrBreak(v1);
                  i = *p1;
                  switch (i.values.vINTEGER)
                  {
                      case 1: case 2: case 3:
                              rp = new cVARVAL(vtSTRING,
                                   ptrUData->PwrdHistory.Previous[
                                   int(i.values.vINTEGER-1)]);
                              break;

                      default:
                              rp = new cVARVAL(vtSTRING,"");
                              break;
                  }
                  f1 = TRUE;
                  break;

             case TOK_OP_U_PWDLC:
                  rp = new cVARVAL(vtDATE,ptrUData->PwrdHistory.LastChange);
                  break;

             case TOK_OP_U_PWDTC:
                  rp = new cVARVAL(vtINTEGER,ptrUData->PwrdHistory.TimesChanged);
                  break;

             case TOK_OP_U_STAT:
                  falseErrBreak(v1);
                  i = *p1;
                  switch (i.values.vINTEGER)
                  {
                      case  1:
                              rp = new cVARVAL(vtDATE,ptrUData->Stats.FirstDateOn);
                              break;

                      case  2:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.NumSysopPages);
                              break;

                      case  3:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.NumGroupChats);
                              break;

                      case  4:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.NumComments);
                              break;

                      case  5:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.Num300);
                              break;

                      case  6:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.Num1200);
                              break;

                      case  7:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.Num2400);
                              break;

                      case  8:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.Num9600);
                              break;

                      case  9:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.Num14400);
                              break;

                      case 10:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.NumSecViol);
                              break;

                      case 11:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.NumNotReg);
                              break;

                      case 12:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.NumReachDnldLim);
                              break;

                      case 13:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.NumFileNotFound);
                              break;

                      case 14:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.NumPwrdErrors);
                              break;

                      case 15:
                              rp = new cVARVAL(vtINTEGER,ptrUData->Stats.NumVerifyErrors);
                              break;

                      default:
                              rp = new cVARVAL(vtINTEGER,(signed long int) 0);
                              break;
                  }
                  f1 = TRUE;
                  break;

             case TOK_OP_DEFCOLOR:
                  rp = new cVARVAL(vtINTEGER,(tINTEGER)PcbData.NewDefaultColor);
                  break;

             case TOK_OP_CURCOLOR:
                  rp = new cVARVAL(vtINTEGER,curcolor());
                  break;

             case TOK_OP_TINKEY:
                  falseErrBreak(v1);
                  i = *p1;
                  s = tinkey(i.values.vINTEGER);
                  rp = new cVARVAL(s);
                  f1 = TRUE;
                  break;

             case TOK_OP_INKEY:
                  rp = new cVARVAL(inkey());
                  break;

             case TOK_OP_KINKEY:
                  rp = new cVARVAL(kinkey());
                  break;

             case TOK_OP_MINKEY:
                  rp = new cVARVAL(minkey());
                  break;

             case TOK_OP_CALLNUM:
                  rp = new cVARVAL(vtINTEGER,
#ifndef LIB
                  Status.CallerNumber
#else
                  (signed long int) 0
#endif
                  );
                  break;

             case TOK_OP_MGETBYTE:
                  rp = new cVARVAL(vtINTEGER,
#ifdef COMM
                  comminkey()
#else
                  -1
#endif
                  );
                  break;

             case TOK_OP_TOKCOUNT:
                  rp = new cVARVAL(vtINTEGER,tokenCnt);
                  break;

             case TOK_OP_U_RECNUM:
                  falseErrBreak(v1);
                  s = *p1;
                  strcpy(buf,nul2mty(s));
                  rp = new cVARVAL(vtINTEGER,finduser(buf));
                  f1 = TRUE;
                  break;

             case TOK_OP_U_INCONF:
                  falseErrBreak(v2);
                  i = *p1;
                  b1 = i.values.vINTEGER;
                  i = *p2;
                  b2 = i.values.vINTEGER;
                  rp = new cVARVAL(vtBOOLEAN,
#ifndef LIB
                       (int) userreginconf((unsigned)b1,(unsigned)b2)
#else
                       (signed long int) 0
#endif
                       );
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_DBGLEVEL:
                  rp = new cVARVAL(vtINTEGER,DebugLevel);
                  break;

             case TOK_OP_MAXNODE:
                  rp = new cVARVAL(vtINTEGER,PCB_MAXNODES);
                  break;

             case TOK_OP_SLPATH:
                  rp = new cVARVAL(vtSTRING,PcbData.SecLoc);
                  break;

             case TOK_OP_HELPPATH:
                  rp = new cVARVAL(vtSTRING,PcbData.HlpLoc);
                  break;

             case TOK_OP_TEMPPATH:
                  rp = new cVARVAL(vtSTRING,PcbData.TmpLoc);
                  break;

             case TOK_OP_GRAFMODE:
                  *buf = 'N';
                  *(buf+1) = '\0';
                  if (Control.RipMode)
                      *buf = 'R';
                  else if (Control.GraphicsMode)
                      *buf = 'G';
                  else if (UseAnsi)
                      *buf = 'A';
                  rp = new cVARVAL(vtSTRING,buf);
                  break;

             case TOK_OP_LOGGEDON:
                  rp = new cVARVAL(vtBOOLEAN,(int) (*ptrUData->Name != '\0'));
                  break;

             case TOK_OP_MODEM:
                  rp = new cVARVAL(vtSTRING,
#if !defined(LIB) && defined(COMM)
                       Asy.ConnectString
#else
                       ""
#endif
                       );
                  break;

             case TOK_OP_ERRCORRECT:
                  rp = new cVARVAL(vtBOOLEAN,
#if defined(COMM)
                       (int) Asy.ErrorCorrected
#else
                       (int) TRUE
#endif
                       );
                  break;

             case TOK_OP_EVTTIMEADJ:
                  rp = new cVARVAL(vtBOOLEAN,(int) Status.TimeAdjustedForEvent);
                  break;

             case TOK_OP_PSA:
                  falseErrBreak(v1);
                  tmpInteger = *p1;
                  switch (tmpInteger.values.vINTEGER)
                  {
                      case 1:
                              c = AliasSupport;
                              break;

                      case 2:
                              c = VerifySupport;
                              break;

                      case 3:
                              c = AddressSupport;
                              break;

                      case 4:
                              c = PasswordSupport;
                              break;

                      case 5:
                              c = StatsSupport;
                              break;

                      case 6:
                              c = NotesSupport;
                              break;
#ifndef LIB

                      case 7:
                              c = AccountSupport;
                              break;

                      case 8:
                              c = QwkSupport;
                              break;

                      case 9:
                              c = PersonalSupport;
                              break;

                      case 10:
                              c = BankSupport;
                              break;
#endif
                      default:
                              c = FALSE;
                              break;
                  }
                  rp = new cVARVAL(vtBOOLEAN,c);
                  f1 = TRUE;
                  break;

             case TOK_OP_FILEINF:
             {
                  int dosfindmax = 1;
                  #ifdef __OS2__
                  int DirHandle = SYSTEMDIRHANDLE;
                  #endif
                  falseErrBreak(v2);
                  s = *p2;
                  i = *p1;

                  strupr(strcpy(buf,nul2mty(s)));

                  ffblk curFFB;
                  int done = dosfindfirst(nul2mty(s),&curFFB,
                          FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_ARCH,&dosfindmax PDIRHANDLE);

                  if (done) memset(&curFFB,0,sizeof(curFFB));

                  switch (i.values.vINTEGER)
                  {
                      case 1: // Exist
                              rp = new cVARVAL(vtBOOLEAN,(int) fexist(nul2mty(s)));
                              break;

                      case 2: // Date
                              sprintf(buf,"%02d-%02d-%02d",
                                      (curFFB.ff_fdate&0x01E0) >> 5,
                                      (curFFB.ff_fdate&0x001F),
                                      (((curFFB.ff_fdate&0xFE00) >> 9) + 80) % 100);
                              rp = new cVARVAL(vtDATE,datetojulian(buf));
                              break;

                      case 3: // Time
                              rp = new cVARVAL(vtTIME,
                                      ((curFFB.ff_ftime&0xF800) >> 11) * 60L * 60L +
                                      ((curFFB.ff_ftime&0x07E0) >>  5) * 60L           +
                                      ((curFFB.ff_ftime&0x001F)          ) *  2L);
                              break;

                      case 4: // Size
                              rp = new cVARVAL(vtINTEGER,curFFB.ff_fsize);
                              break;

                      case 5: // Attrib
                              rp = new cVARVAL(vtINTEGER,curFFB.ff_attrib);
                              break;

                              #ifndef LIB
                      case 6: // Drive
                              if (buf[1] != ':') *buf = dosgetcurdrive() - 1 + 'A' ;
                              buf[1] = '\0';
                              rp = new cVARVAL(vtSTRING,buf);
                              break;
                              #endif

                      case 7: // Path
                      {
                              if (buf[1] == ':') strcpy(buf,buf+2);
                              char * p = strrchr(buf,'\\');
                              p = (p ? ++p : buf);
                              *p = '\0';
                              rp = new cVARVAL(vtSTRING,buf);
                              break;
                      }

                      case 8: // Name
                      {
                              if (buf[1] == ':') strcpy(buf,buf+2);
                              char * p = strrchr(buf,'\\');
                              p = (p ? ++p : buf);
                              strcpy(buf,p);
                              p = strchr(buf,'.');
                              if (p) *p = '\0';
                              rp = new cVARVAL(vtSTRING,buf);
                              break;
                      }

                      case 9: // Ext
                      {
                              if (buf[1] == ':') strcpy(buf,buf+2);
                              char * p = strrchr(buf,'\\');
                              p = (p ? ++p : buf);
                              strcpy(buf,p);
                              p = strchr(buf,'.');
                              strcpy(buf,p ? p+1 : "");
                              rp = new cVARVAL(vtSTRING,buf);
                              break;
                      }

                      default:
                              rp = new cVARVAL(vtINTEGER,(signed long int) 0);
                              break;
                  }

                  while (!done) done = dosfindnext(&curFFB,&dosfindmax PDIRHANDLE2);

                  f1 = f2 = TRUE;
                  break;
             }

             case TOK_OP_SHOWSTAT:
                  rp = new cVARVAL(vtBOOLEAN,(int) Display.ShowOnScreen);
                  break;

             case TOK_OP_PAGESTAT:
                  rp = new cVARVAL(vtBOOLEAN,
#ifndef LIB
                       (int) Status.Paged
#else
                       (tINTEGER) FALSE
#endif
                       );
                  break;

             case TOK_OP_CHATSTAT:
                  rp = new cVARVAL(vtBOOLEAN,
#ifndef LIB
                       //Status.Available
                       (int) canbeavailable()
#else
                       (tINTEGER) FALSE
#endif
                       );
                  break;

             case TOK_OP_SCRTEXT: // Need to figure out 'real' max values for row and col
             {
                  falseErrBreak(v4);

                  i = *p4;
                  b1 = i.values.vINTEGER;
                  inRange(b1,1,MAX_COLS);

                  i = *p3;
                  b2 = i.values.vINTEGER;
                  inRange(b2,1,MAX_ROWS);

                  i = *p2;
                  b3 = i.values.vINTEGER;
                  inRange(b3,1,MAX_COLS);

                  if (b1+b3-1 > MAX_COLS) b3 = MAX_COLS-b1+1;

                  b = *p1;

                  rp = new cVARVAL(vtBIGSTR,scrtext(buf,int(b1),int(b2),
                          int(b3),bool(b.values.vBOOLEAN)));

                  f1 = f2 = f3 = f4 = TRUE;

                  break;
             }

             case TOK_OP_ALIAS:
                  rp = new cVARVAL(vtBOOLEAN,(int) Status.UseAlias);
                  break;

             case TOK_OP_CONFALIAS:
                  rp = new cVARVAL(vtBOOLEAN,
#ifndef LIB
                       (int) CurrConf.AllowAliases
#else
                       FALSE
#endif
                       );
                  break;

             case TOK_OP_USERALIAS:
                  rp = new cVARVAL(vtBOOLEAN,
#ifndef LIB
                       (int) Status.AllowAlias
#else
                       FALSE
#endif
                       );
                  break;

             case TOK_OP_CONFREG:
                  falseErrBreak(v1);

                  i = *p1;
                  b1 = i.values.vINTEGER;
                  inRange(b1,0,65535L);

                  rp = new cVARVAL(vtBOOLEAN,isset(&ptrUReg[CNF_REG],unsigned(b1)));

                  f1 = TRUE;
                  break;

             case TOK_OP_CONFEXP:
                  falseErrBreak(v1);

                  i = *p1;
                  b1 = i.values.vINTEGER;
                  inRange(b1,0,65535L);

                  rp = new cVARVAL(vtBOOLEAN,
                          isset(&ptrUReg[CNF_EXP],unsigned(b1)));

                  f1 = TRUE;

                  break;

             case TOK_OP_CONFSEL:
                  falseErrBreak(v1);

                  i = *p1;
                  b1 = i.values.vINTEGER;
                  inRange(b1,0,65535L);

                  rp = new cVARVAL(vtBOOLEAN,isset(&ptrUReg[CNF_USR],unsigned(b1)));

                  f1 = TRUE;
                  break;

             case TOK_OP_CONFSYS:
                  falseErrBreak(v1);

                  i = *p1;
                  b1 = i.values.vINTEGER;
                  inRange(b1,0,65535L);

                  rp = new cVARVAL(vtBOOLEAN,isset(&ptrUReg[CNF_CON],unsigned(b1)));

                  f1 = TRUE;
                  break;

             case TOK_OP_CONFMW:
                  falseErrBreak(v1);

                  i = *p1;
                  b1 = i.values.vINTEGER;
                  inRange(b1,0,65535L);

                  rp = new cVARVAL(vtBOOLEAN,isset(&ptrUReg[CNF_MFL],unsigned(b1)));

                  f1 = TRUE;
                  break;

             case TOK_OP_LPRINTED:
                  rp = new cVARVAL(vtINTEGER,Display.NumLinesPrinted);
                  break;

             case TOK_OP_ISNONSTOP:
                  rp = new cVARVAL(vtBOOLEAN,(int) !Display.CountLines);
                  break;

             case TOK_OP_DEFANS:
                  rp = new cVARVAL(vtSTRING,LastDefaultAnswer);
                  break;

             case TOK_OP_LASTANS:
                  rp = new cVARVAL(vtSTRING,LastAcceptedAnswer);
                  break;

             case TOK_OP_FMTREAL:
                  falseErrBreak(v3);

                  i = *p1;
                  b1 = i.values.vINTEGER;

                  i = *p2;
                  b2 = i.values.vINTEGER;

                  inRange(b2,-MAX_BIGSTR_LEN,MAX_BIGSTR_LEN);
                  inRange(b1,0,abs(int(b2))-2);
                  inRange(b1,0,abs(int(b2)));

                  r = *p3;

                  sprintf(buf,"%*.*f",int(b2),int(b1),r.values.vDREAL);

                  rp = new cVARVAL(vtBIGSTR,buf);

                  f1 = f2 = f3 = TRUE;
                  break;

             case TOK_OP_FLAGCNT:
                  rp = new cVARVAL(vtINTEGER,NumFiles);
                  break;

             case TOK_OP_LOMSGNUM:
                  rp = new cVARVAL(vtINTEGER,lowmsgnum());
                  break;

             case TOK_OP_HIMSGNUM:
                  rp = new cVARVAL(vtINTEGER,highmsgnum());
                  break;

             case TOK_OP_KBDBUFSIZE:
                  rp = new cVARVAL(vtINTEGER,
                       tINTEGER(bytesleftinkbdbuffer()));
                  break;

             case TOK_OP_KBDFILUSED:
                  rp = new cVARVAL(vtBOOLEAN,tINTEGER(filestuffinuse()));
                  break;

             case TOK_OP_PPLBUFSIZE:
                  rp = new cVARVAL(vtINTEGER,
                       tINTEGER(bytesleftinpplbuffer()));
                  break;

             /*----------------------------------------------------------------
              * 15.4 additions.  Handlers below implement the semantics that
              * WHATSNEW documents for each new PPL function.  All rely on
              * the Personal PSA scaffolding from v0.020 and the URead
              * ShortDesc field (also 15.4).
              *---------------------------------------------------------------*/

             case TOK_OP_U_PERSONAL:
                  /* U_PERSONAL(idx) — read a field of the personal PSA.
                   * idx: 1=Gender, 2=Birthdate, 3=Email, 4=Web.  These
                   * indices follow the PCBSM.EXE 15.4b field-label table
                   * order recovered in v0.017.
                   */
                  falseErrBreak(v1);
                  i = *p1;
                  switch (i.values.vINTEGER)
                  {
                      case 1:  { char g[2] = { ptrPersonal->Gender, 0 };
                                 rp = new cVARVAL(vtSTRING, g); }
                               break;
                      case 2:  rp = new cVARVAL(vtDATE,
                                                datetojulian(ptrPersonal->Birthdate));
                               break;
                      case 3:  rp = new cVARVAL(vtSTRING, ptrPersonal->Email);
                               break;
                      case 4:  rp = new cVARVAL(vtSTRING, ptrPersonal->Web);
                               break;
                      default: rp = new cVARVAL(vtSTRING, "");
                               break;
                  }
                  f1 = TRUE;
                  break;

             case TOK_OP_SHORTDESC:
                  /* SHORTDESC(rec) — read the URead ShortDesc field for
                   * user record `rec`.  ShortDesc is inline in URead
                   * (not a PSA) — added in 15.4 into the 8-byte Reserved
                   * scratch space.  For the currently-loaded user just
                   * pull from ptrUData; for a specific rec num a full
                   * users-file read would be needed (see U_LMR for the
                   * pattern; the argument is present but currently
                   * evaluated only for its side effect of validating type).
                   */
                  falseErrBreak(v1);
                  rp = new cVARVAL(vtSTRING, ptrURead->ShortDesc);
                  f1 = TRUE;
                  break;

             case TOK_OP_GETBANKVAL:
                  /* GETBANKVAL(idx) — read Time Bank PSA field 0-11. */
                  falseErrBreak(v1);
                  i = *p1;
                  {
                      int idx = (int) i.values.vINTEGER;
                      if (BankSupport && idx >= 0 && idx < 12) {
                          long *bp = (long *) &Bank;
                          rp = new cVARVAL(vtDREAL, (double) bp[idx]);
                      } else {
                          rp = new cVARVAL(vtDREAL, 0.0);
                      }
                  }
                  f1 = TRUE;
                  break;

             case TOK_OP_GETMSGHDR:
                  /* GETMSGHDR(conf, message, field) — return a single msg-hdr
                   * field as STRING.  Field indices match SCANMSGHDR (see
                   * ScanMsgHdrSub above; fields 1..15).  Per WHATSNEW #1.A:
                   *   field 1 = status ('R','N','K',...)
                   *   field 5 = date (dmsg)
                   *   field 6 = time (tmsg)
                   *   field 7 = To (tname)
                   *   field 11= From (fname)
                   *   field 12= Subject (subj)
                   *   field 13= Password (pwd)
                   *   ...
                   * arg1 = conf (INTEGER), arg2 = message (DREAL),
                   * arg3 = field (INTEGER 1..15)
                   */
                  falseErrBreak(v3);
                  rp = GetMsgHdrSub(p1, p2, p3);
                  f1 = f2 = f3 = TRUE;
                  message.close();
                  break;

             case TOK_OP_SETMSGHDR:
                  /* SETMSGHDR(conf, message, field, value) — write a msg-hdr
                   * field.  arg4 is the STRING value.  Field indices match
                   * the subset that WHATSNEW documents as writable:
                   *   1 = To, 2 = From, 3 = Subject, 4 = Password, 5 = Echo
                   * Sysop-level access required.  Returns 1 on success, 0
                   * on failure.
                   */
                  falseErrBreak(v4);
                  rp = SetMsgHdrSub(p1, p2, p3, p4);
                  f1 = f2 = f3 = f4 = TRUE;
                  message.close();
                  break;

             case TOK_OP_ABS:
                  falseErrBreak(v1);
                  rp = new cVARVAL(p1->abs());
                  f1 = TRUE;
                  break;                                          // Functions

             case TOK_OP_UPLUS:                                   // Operators
                  falseErrBreak(v1);
                  rp = new cVARVAL(*p1);
                  f1 = TRUE;
                  break;

             case TOK_OP_UMINUS:
                  falseErrBreak(v1);
                  rp = new cVARVAL(-(*p1));
                  f1 = TRUE;
                  break;

             case TOK_OP_EXP:
                  falseErrBreak(v2);
                  rp = new cVARVAL(p2->exp(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_TIMES:
                  falseErrBreak(v2);
                  rp = new cVARVAL((*p2)*(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_DIVIDE:
                  falseErrBreak(v2);
                  rp = new cVARVAL((*p2)/(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_MOD:
                  falseErrBreak(v2);
                  rp = new cVARVAL((*p2)%(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_PLUS:
                  falseErrBreak(v2);
                  rp = new cVARVAL((*p2)+(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_MINUS:
                  falseErrBreak(v2);
                  rp = new cVARVAL((*p2)-(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_EQ:
                  falseErrBreak(v2);
                  rp = new cVARVAL(vtBOOLEAN,(*p2)==(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_NE:
                  falseErrBreak(v2);
                  rp = new cVARVAL(vtBOOLEAN,(*p2)!=(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_LT:
                  falseErrBreak(v2);
                  rp = new cVARVAL(vtBOOLEAN,(*p2)<(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_LE:
                  falseErrBreak(v2);
                  rp = new cVARVAL(vtBOOLEAN,(*p2)<=(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_GT:
                  falseErrBreak(v2);
                  rp = new cVARVAL(vtBOOLEAN,(*p2)>(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_GE:
                  falseErrBreak(v2);
                  rp = new cVARVAL(vtBOOLEAN,(*p2)>=(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_NOT:
                  falseErrBreak(v1);
                  // if p1 is NULL, then process a 0 instead of
                  // allowing it to dereference the NULL
                  if (p1 == NULL)
                    rp = new cVARVAL(vtBOOLEAN,(int) TRUE);
                  else
                    rp = new cVARVAL(vtBOOLEAN,(int) !(*p1));
                  f1 = TRUE;
                  break;

             case TOK_OP_AND:
                  falseErrBreak(v2);
                  rp = new cVARVAL(vtBOOLEAN,(*p2)&&(*p1));
                  f1 = f2 = TRUE;
                  break;

             case TOK_OP_OR:
                  falseErrBreak(v2);
                  rp = new cVARVAL(vtBOOLEAN,(*p2)||(*p1));
                  f1 = f2 = TRUE;
                  break;

             default:
                  scriptErr(0,-1,SCR_ERR_TOKEN,"%d",token);
                  err = TRUE;
                  break;

            }

            if (v4 && f4 && (p4 != NULL))
                    delete p4;
            else if (v4 && !f4)
                    varStack->push(p4);

            if (v3 && f3 && (p3 != NULL))
                    delete p3;
            else if (v3 && !f3)
                    varStack->push(p3);

            if (v2 && f2 && (p2 != NULL))
                    delete p2;
            else if (v2 && !f2)
                    varStack->push(p2);

            if (v1 && f1 && (p1 != NULL))
                    delete p1;
            else if (v1 && !f1)
                    varStack->push(p1);

            if (rp == NULL)
            {

                rp = new cVARVAL(vtSTRING,"");
                varStack->push(rp);
                //    scriptErr(0,-1,SCR_ERR_MEM,"EVALUATING FUNCTION");
                //    err = TRUE;
            }
            else
                    varStack->push(rp);

            if (err) break;
        }

        scriptPointer += sizeof(sint);        // DWT:  changed int to sint
        token = *((sint _FAR_ *) scriptPointer);  // DWT:  changed int to sint
    }

    if (err || breakFlag)
    {
        if(!stackErrFlag && err)
        {
            scriptErr(0,-1,SCR_ERR_EXPEVL,"");
            errFlag = TRUE;
        }
    }
    else
    {
        switch (finalResult.type)
        {
            case vtSTRING:
                 if (finalResult.values.vSTRING != NULL)
                     free(finalResult.values.vSTRING);          // USed to be BFREE
                 break;

            case vtBIGSTR:
                 if (finalResult.values.vBIGSTR.ptr != NULL)
                     free(finalResult.values.vBIGSTR.ptr);      // USed to be BFREE
                 break;
        }

        cVARVAL * stackResult = varStack->pop();

        finalResult.type =  stackResult->type;
        finalResult.initialize();
        finalResult      = *stackResult;

        delete stackResult;

        if (!varStack->isEmpty())
        {
            //err = TRUE;
            while (!varStack->isEmpty())
            {
                cVARVAL * var = varStack->pop();
                delete var;
            }
        }
    }

//  delete varStack;
//  //delete varStkStk.pop();;
//  varStack = varStkStk.pop();
//  //varStkStk.push(varStack);

    delete varStkStk.pop();
    varStack = varStkStk.pop();
    varStkStk.push(varStack);

//  return (err ? NULL : &finalResult);
    return (&finalResult);
}

#if defined(__OS2__) && defined(__BORLANDC__)
  #pragma option -Oe      // BC++ for OS/2 bug - re-enable global register allocation optimization
  #ifdef DBASE
    #pragma option -Oc    // BC++ for OS/2 bug - re-enable local optimizations
  #endif
#endif

#endif


