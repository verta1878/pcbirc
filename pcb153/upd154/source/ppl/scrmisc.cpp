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
/*                                SCRMISC.CPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*   Misc data and functions used by the PCBoard Programming Language (PPL)   */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

// Pragmas

/******************************************************************************/

// Included Files

#include    <stdarg.h>
#include    <stdio.h>
#include    "scrmisc.hpp"


#ifdef DEBUGSCR
#include    <memcheck.h>
#endif

/******************************************************************************/

// Defined Macros

#define MAX_SCR_STK     16

/******************************************************************************/

// Types

/******************************************************************************/

// Constants

#ifdef LIB

char *errText[] =
{
#ifdef PCBTEXT
    "Errors encountered, PPE aborted...",               // *** PCBTEXT
#else
    "Error(s) encountered, compile aborted...",         // *** Compiler Version
#endif

#ifdef PCBTEXT
    "Unable to allocate memory (@OPTEXT@).",            // *** PCBTEXT
#else
    "Fatal Error!  Unable to allocate memory "
        "(@OPTEXT@).",                                  // *** Compiler Version
#endif

    "Invalid token encountered (@OPTEXT@).",            // *** PCBTEXT
    "Closing quote not found (@OPTEXT@)",
    "Closing parenthesis not found (@OPTEXT@)",
    "Too many closing parenthesis (@OPTEXT@)",
    "Illegal label (@OPTEXT@)",
    "Label already used (@OPTEXT@)",
    "Error occurred compiling PPE (@OPTEXT@).",         // *** PCBTEXT
    "Error occurred executing PPE (@OPTEXT@).",         // *** PCBTEXT
    "Error in @OPTEXT@",
    "Illegal variable name (@OPTEXT@)",
    "Variable name already used (@OPTEXT@)",
    "An error occurred evaluating an expression.",      // *** PCBTEXT

#ifdef PCB_DEMO
    "PPE buffer overflow, PPE too large for the DEMO version of PPLC",
#else
    "Fatal Error!  PPE buffer overflow, PPE too large",
#endif

    "Not enough arguments passed (@OPTEXT@)",
    "Variable not found (@OPTEXT@)",
    "No expression to evaluate",
    "IF/WHILE/SELECT requires a conditional expression to evaluate",
    "IF/WHILE/SELECT requires a statement after the conditional expression",
    "Label not found (@OPTEXT@)",
    "Invalid/Missing Variable/Constant in expression",
    "Missing label",
    "Missing variable name(s)",
    "Error saving compiled PPE (@OPTEXT@) to disk",
    "Error loading PPE (@OPTEXT@) from disk.",          // *** PCBTEXT
    "Too many arguments passed (@OPTEXT@)",
    "Error evaluating constant expression (@OPTEXT@)",
    "@OPTEXT@ statement only allowed in a WHILE or FOR loop",
    "Expression may not end with an operator (@OPTEXT@)",
    "Invalid character found in constant expression (@OPTEXT@)",
    "Error appending @OPTEXT@ to answer file.",         // *** PCBTEXT
    "Error deleting file (@OPTEXT@).",                  // *** PCBTEXT
    "Reserved constant name (@OPTEXT@)",
    "No end found for block control statement (IF/WHILE/FOR/SELECT)",
    "Block start (IF/WHILE/FOR/SELECT) must come before block end statement",
    "Bad structure end statement (@OPTEXT@)",
    "Invalid/Missing Operator in expression",
    "Warning in @OPTEXT@",
    "Invalid/Missing Include File (@OPTEXT@)",
    "FOR statement missing index variable and/or start and/or end expressions",
    "SORT requires an INTEGER array as the second parameter, not @OPTEXT@",
    "SORT arguments should be one (1) dimensional arrays (@OPTEXT@)",
    "Error in SELECT CASE expression (@OPTEXT@)",
    "Error in CASE expression (@OPTEXT@)",
    "Error in FUNCTION/PROCEDURE declaration(@OPTEXT@)",
    "Illegal FUNCTION/PROCEDURE call to (@OPTEXT@)",
    "Mismatch in number of parameters in call to (@OPTEXT@)",
    "Error in FUNCTION/PROCEDURE definition:  (@OPTEXT@)",
    "FUNCTION/PROCEDURE has not been declared: (@OPTEXT@)",
    "VAR parameters are not allowed in FUNCTIONS",
    "Cannot pass constant/expression as a VAR parameter in call to (@OPTEXT@)",
    "Stack error. Stack space exausted. Possibly too many recursive function calls.",   // *** PCBTEXT ???
    "Too many var parameters passed in call to (@OPTEXT@). Limited to the first 16 parameters",
    "Too many parameters passed in call to (@OPTEXT@)",
    "Missing FUNCTION/PROCEDURE definition. (@OPTEXT@)"



};

#endif


/******************************************************************************/

// Variables

cSCRIPT * scriptPtr = NULL;

cSCRIPT * scrStk [MAX_SCR_STK];

int       scrStkPtr = 0;

int scrCnt = 0;
/******************************************************************************/


// Function Prototypes

/******************************************************************************/

// Inline Functions

/******************************************************************************/

// Functions

char * LIBENTRY formattedLine(char * l)
{
    static char line[40*2+1];
    sprintf(line,"%-*.*s%s",(strlen(l) > 40) ? 37 : 40,
        (strlen(l) > 40) ? 37 : 40, l, (strlen(l) > 40) ? "..." : "");
    stripleft(line,' ');
    stripright(line,' ');
    return line;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY dispPcbText(int prompt, char * optext, int disp)
{
    strcpy(Status.DisplayText,optext);
    if (disp)
    {
#ifdef LIB
        newline();
        printxlated("@X00@X0C");
        printxlated(errText[prompt]);
        printxlated("@XFF  ");
#else
        displaypcbtext(prompt,LFBEFORE|LOGIT);
#endif
    }
#if !defined(LIB)
    else
    {
        logsystext(prompt,SPACERIGHT);
    }
#elif defined(___EXEC___)
    writelog(errText[prompt],SPACERIGHT);
#endif
}

    /*--------------------------------------------------------------------*/

#if defined(___EXEC___) || defined(___INFO___) || defined(___SCAN___)
#pragma argsused
#endif
void scriptErr(int handNum, int lineNum, int textNum, char * format, ...)
{
#ifdef  ___COMP___
    char warnFlag = FALSE;

    if (textNum < 0)
    {
        warnFlag = TRUE;
        textNum *= -1;
    }
#endif

    int dispFlag = TRUE;

#ifdef  ___EXEC___
    if ((textNum != SCR_ERR_CUSR) && (textNum != SCR_ERR_EUSR) &&
        (Status.CurSecLevel < PcbData.SysopSec[SEC_SYSOPLEVEL]))
        dispFlag = FALSE;
#endif

#ifdef  ___COMP___
    if (lineNum != -1)
    {
        if (handNum > 0)
        {
            char * p = strrchr(OpenFileNames[handNum],'\\');
            p = (p ? ++p : OpenFileNames[handNum]);
            scriptErr(0,-1,warnFlag ? SCR_WRN_LINNUM : SCR_ERR_LINNUM,
                "file %s, line %d",p,lineNum);
        }
        else
        {
            scriptErr(0,-1,warnFlag ? SCR_WRN_LINNUM : SCR_ERR_LINNUM,
                "line %d",lineNum);
        }
    }
    else
#endif
        newline();

    char    fmtBuf[80+1];
    va_list arglist;

    va_start(arglist,format);
    vsprintf(fmtBuf,format,arglist);
    va_end(arglist);

    dispPcbText(textNum,fmtBuf,dispFlag);
}

    /*--------------------------------------------------------------------*/

#ifdef ___COMP___
#pragma argsused
#endif
void LIBENTRY doScrSub(char * f, char * a, int & err)
{

#if !defined(___COMP___) && !defined(___EXEC___) && !defined(___INFO___) && !defined(___SCAN___) && !defined(___DCOM___)
#error define one of the following for compile
#error ___COMP___, ___EXEC___, ___INFO___, ___SCAN___, or ___DCOM___
#endif

    strupr(f);

#if defined(___EXEC___) || defined(___INFO___) || defined(___SCAN___)

    // perhaps we should enter a critical region here?
 // cVARVAL::fastNewDel = false;
    err = scriptPtr->load(f);
 // cVARVAL::fastNewDel = true;
    // perhaps we should exit  a critical region here?

    if (err)
    {
        scriptErr(0,-1,SCR_ERR_LUSR,"%s",formattedLine(f));
        newline();
        newline();
    }
#elif defined(___COMP___)
    err = scriptPtr->compile(f);
#endif

    if (!err)
    {
#if defined(___COMP___)
        err = scriptPtr->save(f);
        if (err)
        {
            scriptErr(0,-1,SCR_ERR_SUSR,"%s",formattedLine(f));
            newline();
            newline();
        }
#elif defined(___EXEC___) || defined(___INFO___) || defined(___SCAN___)
        err = scriptPtr->execute(a);
        if (err)
        {
            scriptErr(0,-1,SCR_ERR_EUSR,"%s",formattedLine(f));
            newline();
            newline();
        }
#endif
    }
}

    /*--------------------------------------------------------------------*/

bool LIBENTRY pushScript(void)
{
    if ((scriptPtr != NULL) && (scrStkPtr < MAX_SCR_STK))
    {
        scrStk[scrStkPtr++] = scriptPtr;
        scriptPtr = NULL;
    }
    else if (scriptPtr != NULL)
        return FALSE;                   // Return false if error

    return TRUE;                        // Return true if no error
}

    /*--------------------------------------------------------------------*/

void LIBENTRY popScript(void)
{
    if (scriptPtr != NULL)
    {
        delete scriptPtr;
        scriptPtr = NULL;
    }
    if (scrStkPtr > 0) scriptPtr = scrStk[--scrStkPtr];

}

    /*--------------------------------------------------------------------*/

#ifdef ___EXEC___

#ifdef LIB

       bool NoPPE; // Used to temporarily disable PPE execution

#else

extern bool NoPPE; // Used to temporarily disable PPE execution

#endif

#endif

int LIBENTRY doScript(char * f, char * a, int n)
{

#ifdef ___EXEC___

    if (NoPPE)
    {
        print("PPEs disabled:  ");
        println(f);
        return (a ? 1 : 0);
    }

#endif

    int err;
#ifdef  ___EXEC___
    int ansAp = 0;
#endif
    scrCnt++;

    if (!pushScript())
    {
      scrCnt--;
      return -2;
    }
    #ifdef ___EXEC___
    if(scrCnt == 1)
      showactivity(ACTSUSPEND);
    #endif

    scriptPtr = new cSCRIPT(n);
    if (scriptPtr != NULL)
    {
        #if defined(___EXEC___) && defined(__OS2__)
          if (scrCnt == 1)
            changeupdateinterval(UPDT_FASTINTERVAL);
        #endif

        doScrSub(f,a,err);

        #if defined(___EXEC___) && defined(__OS2__)
          if (scrCnt == 1)
            changeupdateinterval(UPDT_DFLTINTERVAL);
        #endif

#ifdef  ___EXEC___
        ansAp = scriptPtr->ansAppended;
#endif
    }
    else
    {
        err = TRUE;
        #ifdef __OS2__
          strcpy(Status.DisplayText,"SCRIPT");
        #else
          sprintf(Status.DisplayText,"SCRIPT (%u/%lu)",sizeof(cSCRIPT),coreleft());
        #endif
        newline();
#ifdef  ___EXEC___
        displaypcbtext(508,LOGIT);
#else
        print("Insufficient Memory for Operation ");
        println(Status.DisplayText);
#endif
        if(Status.SysopFlag == ' ')
        {
          Status.SysopFlag = (Status.SysopFlag == 'R' ? ' ' : 'R');
          #ifndef LIB
          makepcboardsys();
          #endif
        }
        newline();
        newline();
    }

    popScript();

    #ifdef ___EXEC___
    if(scrCnt == 1)
      showactivity(ACTRESUME);
    #endif

    scrCnt--;

    return (err ? -1 :
#ifdef  ___EXEC___
        ansAp
#else
        0
#endif
        );
}

    /*--------------------------------------------------------------------*/

void LIBENTRY cleanupScript(void)
{
    while (scrStkPtr > 0) popScript();
    popScript();
}

#ifdef ___EXEC___

/******************************************************************************/

cVARVAL * LIBENTRY cSCRIPT::getConfInfo(unsigned int confnum, unsigned int field)
{

 cVARVAL * rp = NULL;

#ifdef LIB
    if(field || confnum) field = field; // this is to avoid compiler warnings
    rp = new cVARVAL(vtINTEGER,-1);
    return rp;
#else
    pcbconftype   confrec;

    if(confnum > PcbData.NumConf)
    {

      rp = new cVARVAL(vtINTEGER,-1);
      return rp;
    }
    else
      getconfrecord(confnum,&confrec);

    switch(field)
    {
      case  1: rp = new cVARVAL(vtSTRING,confrec.Name)                    ;break;
      case  2: rp = new cVARVAL(vtBOOLEAN,(int) confrec.PublicConf)       ;break;
      case  3: rp = new cVARVAL(vtBOOLEAN,(int) confrec.AutoRejoin)       ;break;
      case  4: rp = new cVARVAL(vtBOOLEAN,(int) confrec.ViewMembers)      ;break;
      case  5: rp = new cVARVAL(vtBOOLEAN,(int) confrec.PrivUplds)        ;break;
      case  6: rp = new cVARVAL(vtBOOLEAN,(int) confrec.PrivMsgs)         ;break;
      case  7: rp = new cVARVAL(vtBOOLEAN,(int) confrec.EchoMail)         ;break;
      case  8: rp = new cVARVAL(vtINTEGER,      confrec.ReqSecLevel)      ;break;
      case  9: rp = new cVARVAL(vtINTEGER,      confrec.AddSec)           ;break;
      case 10: rp = new cVARVAL(vtINTEGER,      confrec.AddTime)          ;break;
      case 11: rp = new cVARVAL(vtINTEGER,      confrec.MsgBlocks)        ;break;
      case 12: rp = new cVARVAL(vtSTRING,       confrec.MsgFile)          ;break;
      case 13: rp = new cVARVAL(vtSTRING,       confrec.UserMenu)         ;break;
      case 14: rp = new cVARVAL(vtSTRING,       confrec.SysopMenu)        ;break;
      case 15: rp = new cVARVAL(vtSTRING,       confrec.NewsFile)         ;break;
      case 16: rp = new cVARVAL(vtINTEGER,      confrec.PubUpldSort)      ;break;
      case 17: rp = new cVARVAL(vtSTRING,       confrec.UpldDir)          ;break;
      case 18: rp = new cVARVAL(vtSTRING,       confrec.PubUpldLoc)       ;break;
      case 19: rp = new cVARVAL(vtINTEGER,      confrec.PrvUpldSort)      ;break;
      case 20: rp = new cVARVAL(vtSTRING,       confrec.PrivDir)          ;break;
      case 21: rp = new cVARVAL(vtSTRING,       confrec.PrvUpldLoc)       ;break;
      case 22: rp = new cVARVAL(vtSTRING,       confrec.DrsMenu)          ;break;
      case 23: rp = new cVARVAL(vtSTRING,       confrec.DrsFile)          ;break;
      case 24: rp = new cVARVAL(vtSTRING,       confrec.BltMenu)          ;break;
      case 25: rp = new cVARVAL(vtSTRING,       confrec.BltNameLoc)       ;break;
      case 26: rp = new cVARVAL(vtSTRING,       confrec.ScrMenu)          ;break;
      case 27: rp = new cVARVAL(vtSTRING,       confrec.ScrNameLoc)       ;break;
      case 28: rp = new cVARVAL(vtSTRING,       confrec.DirMenu)          ;break;
      case 29: rp = new cVARVAL(vtSTRING,       confrec.DirNameLoc)       ;break;
      case 30: rp = new cVARVAL(vtSTRING,       confrec.PthNameLoc)       ;break;
      case 31: rp = new cVARVAL(vtBOOLEAN,(int) confrec.ForceEcho)        ;break;
      case 32: rp = new cVARVAL(vtBOOLEAN,(int) confrec.ReadOnly)         ;break;
      case 33: rp = new cVARVAL(vtBOOLEAN,(int) confrec.NoPrivateMsgs)    ;break;
      case 34: rp = new cVARVAL(vtINTEGER,      confrec.RetReceiptLevel)  ;break;
      case 35: rp = new cVARVAL(vtBOOLEAN,(int) confrec.RecordOrigin)     ;break;
      case 36: rp = new cVARVAL(vtBOOLEAN,(int) confrec.PromptForRouting) ;break;
      case 37: rp = new cVARVAL(vtBOOLEAN,(int) confrec.AllowAliases)     ;break;
      case 38: rp = new cVARVAL(vtBOOLEAN,(int) confrec.ShowIntroOnRA)    ;break;
      case 39: rp = new cVARVAL(vtINTEGER,      confrec.ReqLevelToEnter)  ;break;
      case 40: rp = new cVARVAL(vtSTRING ,      confrec.Password)         ;break;
      case 41: rp = new cVARVAL(vtSTRING ,      confrec.Intro)            ;break;
      case 42: rp = new cVARVAL(vtSTRING ,      confrec.AttachLoc)        ;break;
      case 43: rp = new cVARVAL(vtSTRING ,      confrec.RegFlags)         ;break;
      case 44: rp = new cVARVAL(vtBYTE   ,      confrec.AttachLevel)      ;break;
      case 45: rp = new cVARVAL(vtBYTE   ,      confrec.CarbonLimit)      ;break;
      case 46: rp = new cVARVAL(vtSTRING ,      confrec.CmdLst)           ;break;
      case 47: rp = new cVARVAL(vtBOOLEAN,(int) confrec.OldIndex)         ;break;
      case 48: rp = new cVARVAL(vtBOOLEAN,(int) confrec.LongToNames)      ;break;
      case 49: rp = new cVARVAL(vtBYTE   ,      confrec.CarbonLevel)      ;break;
      case 50: rp = new cVARVAL(vtBYTE   ,      confrec.ConfType)         ;break;
      case 51: rp = new cVARVAL(vtINTEGER,      confrec.ExportPtr)        ;break;
      case 52: rp = new cVARVAL(vtDREAL  ,      confrec.ChargeTime)       ;break;
      case 53: rp = new cVARVAL(vtDREAL  ,      confrec.ChargeMsgRead)    ;break;
      case 54: rp = new cVARVAL(vtDREAL  ,      confrec.ChargeMsgWrite)   ;break;
      default: rp = new cVARVAL(vtINTEGER,-1)                             ;break;
    }
    return rp;
#endif

}


void LIBENTRY cSCRIPT::putConfInfo(unsigned int confnum, unsigned int field, cVARVAL * value)
{

    pcbconftype   confrec;

#ifdef LIB
    return;
#endif

    if(confnum > PcbData.NumConf)  return;
    else                           getconfrecord(confnum,&confrec);

    switch(field)
    {

      case  1: maxstrcpy(confrec.Name,value->values.vSTRING,sizeof(confrec.Name));break;
      case  2: confrec.PublicConf  = value->values.vBOOLEAN;break;
      case  3: confrec.AutoRejoin  = value->values.vBOOLEAN;break;
      case  4: confrec.ViewMembers = value->values.vBOOLEAN;break;
      case  5: confrec.PrivUplds   = value->values.vBOOLEAN;break;
      case  6: confrec.PrivMsgs    = value->values.vBOOLEAN;break;
      case  7: confrec.EchoMail    = value->values.vBOOLEAN;break;
      case  8: confrec.ReqSecLevel = (int) value->values.vINTEGER;break;
      case  9: confrec.AddSec      = (int) value->values.vINTEGER;break;
      case 10: confrec.AddTime     = (int) value->values.vINTEGER;break;
      case 11: confrec.MsgBlocks   = (int) value->values.vINTEGER;break;
      case 12: maxstrcpy(confrec.MsgFile,value->values.vSTRING,sizeof(confrec.MsgFile));break;
      case 13: maxstrcpy(confrec.UserMenu,value->values.vSTRING,sizeof(confrec.UserMenu));break;
      case 14: maxstrcpy(confrec.SysopMenu,value->values.vSTRING,sizeof(confrec.SysopMenu));break;
      case 15: maxstrcpy(confrec.NewsFile,value->values.vSTRING,sizeof(confrec.NewsFile));break;
      case 16: confrec.PubUpldSort = (char) value->values.vINTEGER;break;
      case 17: maxstrcpy(confrec.UpldDir,value->values.vSTRING,sizeof(confrec.UpldDir));break;
      case 18: maxstrcpy(confrec.PubUpldLoc,value->values.vSTRING,sizeof(confrec.PubUpldLoc));break;
      case 19: confrec.PrvUpldSort = (char) value->values.vINTEGER;break;
      case 20: maxstrcpy(confrec.PrivDir,value->values.vSTRING,sizeof(confrec.PrivDir));break;
      case 21: maxstrcpy(confrec.PrvUpldLoc,value->values.vSTRING,sizeof(confrec.PrvUpldLoc));break;
      case 22: maxstrcpy(confrec.DrsMenu,value->values.vSTRING,sizeof(confrec.DrsMenu));break;
      case 23: maxstrcpy(confrec.DrsFile,value->values.vSTRING,sizeof(confrec.DrsFile));break;
      case 24: maxstrcpy(confrec.BltMenu,value->values.vSTRING,sizeof(confrec.BltMenu));break;
      case 25: maxstrcpy(confrec.BltNameLoc,value->values.vSTRING,sizeof(confrec.BltNameLoc));break;
      case 26: maxstrcpy(confrec.ScrMenu,value->values.vSTRING,sizeof(confrec.ScrMenu));break;
      case 27: maxstrcpy(confrec.ScrNameLoc,value->values.vSTRING,sizeof(confrec.ScrNameLoc));break;
      case 28: maxstrcpy(confrec.DirMenu,value->values.vSTRING,sizeof(confrec.DirMenu));break;
      case 29: maxstrcpy(confrec.DirNameLoc,value->values.vSTRING,sizeof(confrec.DirNameLoc));break;
      case 30: maxstrcpy(confrec.PthNameLoc,value->values.vSTRING,sizeof(confrec.PthNameLoc));break;
      case 31: confrec.ForceEcho        = value->values.vBOOLEAN;break;
      case 32: confrec.ReadOnly         = value->values.vBOOLEAN;break;
      case 33: confrec.NoPrivateMsgs    = value->values.vBOOLEAN;break;
      case 34: confrec.RetReceiptLevel  = (int) value->values.vINTEGER;break;
      case 35: confrec.RecordOrigin     = value->values.vBOOLEAN;break;
      case 36: confrec.PromptForRouting = value->values.vBOOLEAN;break;
      case 37: confrec.AllowAliases     = value->values.vBOOLEAN;break;
      case 38: confrec.ShowIntroOnRA    = value->values.vBOOLEAN;break;
      case 39: confrec.ReqLevelToEnter  = (int) value->values.vINTEGER;break;
      case 40: maxstrcpy(confrec.Password,value->values.vSTRING,sizeof(confrec.Password));break;
      case 41: maxstrcpy(confrec.Intro,value->values.vSTRING,sizeof(confrec.Intro));break;
      case 42: maxstrcpy(confrec.AttachLoc,value->values.vSTRING,sizeof(confrec.AttachLoc));break;
      case 43: maxstrcpy(confrec.RegFlags,value->values.vSTRING,sizeof(confrec.RegFlags));break;
      case 44: confrec.AttachLevel = value->values.vBYTE;break;
      case 45: confrec.CarbonLimit = value->values.vBYTE;break;
      case 46: maxstrcpy(confrec.CmdLst,value->values.vSTRING,sizeof(confrec.CmdLst));break;
      case 47: confrec.OldIndex       = value->values.vBOOLEAN;break;
      case 48: confrec.LongToNames    = value->values.vBOOLEAN;break;
      case 49: confrec.CarbonLevel    = value->values.vBYTE;break;
      case 50: confrec.ConfType       = value->values.vBYTE;break;
      case 51: confrec.ExportPtr      = value->values.vINTEGER;break;
      case 52: confrec.ChargeTime     = value->values.vDREAL;break;
      case 53: confrec.ChargeMsgRead  = value->values.vDREAL;break;
      case 54: confrec.ChargeMsgWrite = value->values.vDREAL;break;
      default: ;break;

    }

    putconfrecord(confnum,&confrec);

}


long standardizeTicks(long ticks)
{
  // under OS/2 the settimer() call uses values measured as 1/100th of a second
  // under DOS the values are 1/18th of a second

  const unsigned long multiplier = 10000;

  if(ticks > (LONG_MAX/multiplier)) ticks = LONG_MAX/multiplier;


  return
  #ifdef __OS2__
   ( ((unsigned long)ticks * multiplier)/182);
  #else
   ticks;
  #endif
}

#endif
