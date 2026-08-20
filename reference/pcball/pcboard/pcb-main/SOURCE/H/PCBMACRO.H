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


enum {SKIP,ALIAS,AUTOMORE,BEEP,BICPS,BOARDNAME,BPS,BYTECREDIT,BYTELIMIT,
  BYTERATIO,BYTESLEFT,CARRIER,CITY,CLREOL,CLS,CONFNAME,CONFNUM,CREDLEFT,CREDNOW,
  CREDSTART,CREDUSED,CURMSGNUM,DATAPHONE,DAYBYTES,DELAY,DIRNAME,DIRNUM,DLBYTES,
  DLFILES,ENV,EVENT,EXPDATE,EXPDAYS,FBYTES,FFILES,FILECREDIT,FILERATIO,FIRSTU,
  FIRST,FNUM,FREESPACE,HOMEPHONE,HIGHMSGNUM,INAME,INCONF,KBLEFT,KBLIMIT,
  LASTCALLERNODE,LASTCALLERSYSTEM,LASTDATEON,LASTTIMEON,LMR,LOGDATE,LOGTIME,
  LOWMSGNUM,MAXBYTES,MAXFILES,MINLEFT,MORE,MSGLEFT,MSGREAD,NOCHAR,NODE,NUMBLT,
  NUMCALLS,NUMCONF,NUMDIR,NUMTIMESON,OFFHOURS,OPTEXT,PAUSE,POFF,PON,POS,PROLTR,
  PRODESC,PWXDATE,PWXDAYS,QOFF,QON,RATIOBYTES,RATIOFILES,RBYTES,RCPS,REALNAME,
  RFILES,SBYTES,SCPS,SECLEVEL,SFILES,SYSDATE,SYSOPIN,SYSOPOUT,SYSTIME,
  TIMELIMIT,TIMELEFT,TIMEUSED,TOTALTIME,UPBYTES,UPFILES,USERNAME,WAIT,WHO,
  XOFF,XON,YESCHAR,XCOLORS};

extern int           FindTokenStart;
extern int           FindTokenEnd;
extern unsigned char FindTokenColor;
extern unsigned char FindTokenAttr;

#ifdef __OS2__
  int LIBENTRY findtoken(char *Str);
#else
  #ifdef __cplusplus
    extern "C" {
  #endif

  int LIBENTRY findtoken(char *Str);

  #ifdef __cplusplus
    }
  #endif
#endif
