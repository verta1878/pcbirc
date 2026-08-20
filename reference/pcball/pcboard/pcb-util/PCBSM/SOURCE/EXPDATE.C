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
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <dosfunc.h>
#include <help.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define NUMEXPFIELDS 6

#define DODATE    1
#define DODAYS    2
#define BLANKDATE 29220          /* julian date for 01/01/80 */

typedef struct {
  char     Base;
  int      HiLimit;
  int      LoLimit;
  unsigned Date;
  unsigned Days;
  bool     Printer;
  bool     Type;
  char     DateStr[9];
  char     StoreDate[6];
} ExpType;

static ExpType Exp;

static FldType ExpFields[NUMEXPFIELDS] = {
  {vCHAR,NORE   ,CHGEXPDATE+0, 3, 7, 1,NOCLEARFLD,"Base Security Level Criteria on NORMAL or EXPIRED Level (N/E)","",&Exp.Base   ,NULL},
  {vINT ,ALLNUM ,CHGEXPDATE+1, 3, 8, 3,CLEAR     ,"Adjust Expiration Date if level is Greater than or equal to  ","",&Exp.LoLimit,NULL},
  {vINT ,ALLNUM ,CHGEXPDATE+1, 3, 9, 3,CLEAR     ,"Adjust Expiration Date if level is Less than or equal to     ","",&Exp.HiLimit,NULL},
  {vDATE,ALLDATE,CHGEXPDATE+2, 3,15, 8,CLEAR     ,"New Expiration Date (01/01/80 is ignored)"                    ,"",&Exp.Date   ,NULL},
  {vINT ,ALLNUM ,CHGEXPDATE+3, 3,16, 4,CLEAR     ,"Current Date in record plus XXXX days    "                    ,"",&Exp.Days   ,NULL},
  {vBOOL,YESNO  ,0           , 3,19, 1,NOCLEARFLD,"Print Changed User Records on the Printer"                    ,"",&Exp.Printer,NULL}
};


/********************************************************************
*
*  Function:
*
*  Desc    :
*
*  Returns :
*/

static int pascal expdatesub(URead *p) {
  char Tmp[100];
  char Temp[80];
  char Name[26];
  char TempDate[9];
  int  OldDate;
  int  Level;

  switch (Exp.Base) {
    case 'N': Level = p->SecurityLevel;    break;
    case 'E': Level = p->ExpSecurityLevel; break;
    default : return(-1);
  }

  if (Level >= Exp.LoLimit && Level <= Exp.HiLimit) {
    yymmddtostr(TempDate,p->RegExpDate);
    OldDate = datetojulian(TempDate);
    switch(Exp.Type) {
      case DODATE: memcpy(p->RegExpDate,Exp.StoreDate,6); break;
      case DODAYS: if (OldDate != 0) {
                     OldDate += Exp.Days;
                     strcpy(Exp.DateStr,juliantodate(OldDate));
                     strtoyymmdd(p->RegExpDate,Exp.DateStr);
                   } else return(0);
                   break;
    }

    memcpy(Name,p->Name,sizeof(p->Name)); Name[25] = 0;
    fastprint(5,19,Name,Colors[QUESTION]);

    sprintf(Temp,"Sec: %3d, Date was %s, now %s",Level,TempDate,Exp.DateStr);
    fastprint(29,19,Temp,Colors[STATUS]);
    scrollup(5,10,74,19,Colors[OUTBOX]);
    if (Exp.Printer) {
      LineNum++;
      sprintf(Tmp,"%s %s\r\n",Name,Temp);
      if (dosfputs(Tmp,&prn) == -1)
        return(-1);
      if (LineNum > 63)
        if (printheading(FALSE) == -1)
          return(-1);
    }
  }
  return(0);
}



/********************************************************************
*
*  Function: expdate()
*
*  Desc    :
*
*/

void pascal expdate(void) {
  long   Recs;
  char   *p;                  /* 1   2   3   4   5   6   7   */
  char static BatchCommands[] = "NORMEXPILOWSHIGHDATEDAYSPRIN";


  PrintHeadStr = "Change Expiration Date";
  clscolor(Colors[OUTBOX]);
  generalscreen(MainHead1,PrintHeadStr);

  Exp.Base    = 'N';
  Exp.LoLimit = 0;
  Exp.HiLimit = 0;
  Exp.Date    = BLANKDATE;
  Exp.Days    = 0;
  Exp.Printer = FALSE;

  if (BatchMode) {
    while ((p = parsepaths(NULL)) != NULL) {
      switch(findfour(BatchCommands,p)) {
        case 0: break;  /* skip if not found */
        case 1: Exp.Base    = 'N';          break;
        case 2: Exp.Base    = 'E';          break;
        case 3: Exp.LoLimit = intparam(p);  break;
        case 4: Exp.HiLimit = intparam(p);  break;
        case 5: Exp.Date    = dateparam(p); break;
        case 6: Exp.Days    = intparam(p);  break;
        case 7: Exp.Printer = TRUE;         break;
      }
    }
  } else {
    initquest(ExpFields,NUMEXPFIELDS);
    fastprint(3, 5,"Security Level Range",Colors[DISPLAY]);
    fastprint(3, 6,"컴컴컴컴컴컴컴컴컴컴",Colors[DISPLAY]);
    fastprint(3,13,"Change Expiration Date To:",Colors[DISPLAY]);
    fastprint(3,14,"컴컴컴컴컴컴컴컴컴컴컴컴컴",Colors[DISPLAY]);
    fastcenter(22," Press PGDN to begin, or press ESC to abort ",Colors[DESC]);
    readscrn(ExpFields,NUMEXPFIELDS-1,0,MainHead1,PrintHeadStr,1,NOCLEARFLD);  //lint !e534
    freeanswers(ExpFields,NUMEXPFIELDS);
  }

  if (KeyFlags != ESC && (Exp.Date != BLANKDATE || Exp.Days != 0)) {
    if ((Recs = lockusersfile(PrintHeadStr,NONBUFFERED)) != -1) {
      if (Exp.Date != BLANKDATE) {
        Exp.Type = DODATE;
        strcpy(Exp.DateStr,juliantodate(Exp.Date));
        strtoyymmdd(Exp.StoreDate,Exp.DateStr);
      } else {
        Exp.Type = DODAYS;
      }
      setcursor(CUR_BLANK);
      boxcls(3,9,75,20,Colors[OUTBOX],SINGLE);
      if (Exp.Printer)
        if (printheading(TRUE) == -1)
          return;

      processusersfile(Recs,FALSE,Exp.Printer,FALSE,NONBUFFERED,expdatesub);
    }
  }
}
