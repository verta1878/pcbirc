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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <pcb.h>
#include <misc.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif


char static ACFO[] = {4,'A','C','F','O'};
char static Driver;
char static PortNum;
char static PortAddress[5];

int checkport(int Before) {
  if (Before)
    return(0);

  if (PortNum <= 2 || Driver == 'C' || Driver == 'F' || Driver == 'O')
    return(0);

  if (PortNum >= 3 && PortNum <= 8) {
    boxcls(44,7,77,9,Colors[OUTBOX],SINGLE);
    inputnum(46,8,2,"IRQ",&PcbData.IrqNum,vBYTE,MODEMSETUP+12);
    inputstr(56,8,4,"BASE",PortAddress,PortAddress,HEXNUM,INPUT_CAPS,MODEMSETUP+13);
    return(0);
  }

  beep();
  showhelp(ERRPORT);
  return(-1);
}

int checkspeed(int Before) {
  if (Before)
    return(0);

  if (PcbData.ModemSpeed <= 57600U) {
    switch((unsigned) PcbData.ModemSpeed) {
      case    300:
      case   1200:
      case   2400:
      case   4800:
      case   9600:
      case  19200:
      case 38400U:
      case 57600U: return(0);
    }
  } else if (PcbData.ModemSpeed == 115200L) {
    return(0);
  }

  beep();
  showhelp(ERRSPEED);
  return(-1);
}


/********************************************************************
*
*  Function: initmodemfields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initfields(FldType *P) {
  addquest(P, 0,vBYTE   ,MODEMSETUP+ 0,ALLNUM , 3, 5, 2,Questions[FLDMODEM+ 0],&PcbData.Seconds       ,CLEAR     ,NULL);
  addquest(P, 1,vUPSTR  ,MODEMSETUP+ 1,ACFO   , 3, 6, 1,Questions[FLDMODEM+ 1],&Driver                ,NOCLEARFLD,NULL);
  addquest(P, 2,vBYTE   ,MODEMSETUP+ 2,ALLPORT, 3, 7, 2,Questions[FLDMODEM+ 2],&PortNum               ,CLEAR     ,checkport);
  addquest(P, 3,vLONG   ,MODEMSETUP+ 3,ALLBAUD, 3, 8, 6,Questions[FLDMODEM+ 3],&PcbData.ModemSpeed    ,CLEAR     ,checkspeed);
  addquest(P, 4,vBOOL   ,MODEMSETUP+ 4,YESNO  , 3, 9, 1,Questions[FLDMODEM+ 4],&PcbData.LockSpeed     ,NOCLEARFLD,NULL);
  addquest(P, 5,vUPSTR  ,MODEMSETUP+ 5,ALLCHAR, 3,10,40,Questions[FLDMODEM+ 5], PcbData.ModemInit     ,CLEAR     ,NULL);
  addquest(P, 6,vUPSTR  ,MODEMSETUP+ 5,ALLCHAR, 3,11,40,Questions[FLDMODEM+ 6], PcbData.ModemInit2    ,CLEAR     ,NULL);
  addquest(P, 7,vUPSTR  ,MODEMSETUP+ 7,ALLCHAR, 3,12,40,Questions[FLDMODEM+ 7], PcbData.ModemOff      ,CLEAR     ,NULL);
  addquest(P, 8,vUPSTR  ,MODEMSETUP+ 8,ALLCHAR, 3,13,40,Questions[FLDMODEM+ 8], PcbData.ModemAns      ,CLEAR     ,NULL);
  addquest(P, 9,vUPSTR  ,MODEMSETUP+ 9,ALLCHAR, 3,14,40,Questions[FLDMODEM+ 9], PcbData.ModemDial     ,CLEAR     ,NULL);
  addquest(P,10,vINT,MODEMSETUP+10,ALLNUM , 3,15, 4,Questions[FLDMODEM+10],&PcbData.NumRedials    ,CLEAR     ,NULL);
  addquest(P,11,vINT,MODEMSETUP+11,ALLNUM , 3,16, 4,Questions[FLDMODEM+11],&PcbData.MaxTries      ,CLEAR     ,NULL);
}


void pascal modem(void) {
  FldType  *ModemFields;
  char     *p;
  char     *q;
  int       LineNum;
  long      Baud;
  char      Str[20];

  if ((ModemFields = (FldType *) mallochk(NUMMODEM * sizeof(FldType))) == NULL)
    return;

  initfields(ModemFields);

  if (UpdateBox)
    clsbox(1,1,78,23,Colors[OUTBOX]);
  else
    cls();

  fastprint(40,6,"A=ASYNC, C=COMM-DRV, F=FOSSIL, O=OS/2",Colors[QUESTION]);

  LineNum = 18;
  if ((p = getenv("PCB")) != NULL) {
    strupr(p);
    if (strstr(p,"/OS") != NULL)
      fastprint(3,LineNum++,"* The Comm Driver to Use is overridden by the PCB= environment.",Colors[HEADING]);

    if ((q = strstr(p,"/BAUD:")) != NULL) {
      Baud = atol(q+6);
      sprintf(Str,"* Baud=%ld",Baud);
      fastprint(49,8,Str,Colors[HEADING]);
      fastprint(3,LineNum++,"* The Baud Rate is currently overridden by the PCB= environment.",Colors[HEADING]);
    }

    if ((q = strstr(p,"/COM")) != NULL) {
      PortNum = atoi(q+4);
      sprintf(Str,"* Port=%d",PortNum);
      fastprint(49,7,Str,Colors[HEADING]);
      fastprint(3,LineNum++,"* The Port number is currently overridden by the PCB= environment.",Colors[HEADING]);
    } else if ((q = strstr(p,"/PORT")) != NULL) {
      PortNum = atoi(q + (q[5] == ':' ? 6 : 5));
      if (strchr(q,'F') != NULL)
        fastprint(3,LineNum++,"* The Comm Driver to Use is overridden by the PCB= environment.",Colors[HEADING]);
      sprintf(Str,"* Port=%d",PortNum);
      fastprint(49,7,Str,Colors[HEADING]);
      fastprint(3,LineNum++,"* The Port number is currently overridden by the PCB= environment.",Colors[HEADING]);
    }

    if (strstr(p,"/INIT:") != NULL)
      fastprint(3,LineNum++,"* The Init String is currently overridden by the PCB= environment.",Colors[HEADING]);
    if (strstr(p,"/BASE:") != NULL)
      fastprint(3,LineNum++,"* The Base Address is currently overridden by the PCB= environment.",Colors[HEADING]);
    if (strstr(p,"/IRQ:") != NULL)
      fastprint(3,LineNum++,"* The IRQ is currently overridden by the PCB= environment.",Colors[HEADING]);
  }

  Driver = 'A';
  if (memcmp(PcbData.ModemPort,"COM",3) == 0) {
    PortNum = atoi(&PcbData.ModemPort[3]);
  } else if (memcmp(PcbData.ModemPort,"PORT",4) == 0) {
    PortNum = atoi(&PcbData.ModemPort[4]);
    if (PcbData.OS2Driver)
      Driver = 'O';
    else if (PcbData.ModemPort[strlen(PcbData.ModemPort)-1] == 'F')
      Driver = 'F';
    else
      Driver = 'C';
  } else
    PortNum = 0;

  if (Driver == 'A') {
    if (PortNum == 1) {
      PcbData.IrqNum = 4;
      PcbData.BaseAddress = 0x3F8;
    } else if (PortNum == 2) {
      PcbData.IrqNum = 3;
      PcbData.BaseAddress = 0x2F8;
    }
    sprintf(PortAddress,"%X",PcbData.BaseAddress);
  }

  readscrn(ModemFields,NUMMODEM-1,0,"Modem Information","Modem Setup",1,NOCLEARFLD);

  if (PortNum == 0)
    strcpy(PcbData.ModemPort,"NONE");
  else {
    PcbData.OS2Driver = FALSE;
    switch (Driver) {
      case 'O': PcbData.OS2Driver = TRUE; // fall thru
      case 'C': sprintf(PcbData.ModemPort,"PORT%d",PortNum);   break;
      case 'F': sprintf(PcbData.ModemPort,"PORT%dF",PortNum);  break;
      case 'A': sprintf(PcbData.ModemPort,"COM%d:",PortNum);
                if (PortNum == 1) {
                  PcbData.IrqNum = 4;
                  PcbData.BaseAddress = 0x3F8;
                } else if (PortNum == 2) {
                  PcbData.IrqNum = 3;
                  PcbData.BaseAddress = 0x2F8;
                } else
                  PcbData.BaseAddress = hextoint(PortAddress);
                break;
    }
  }

  freescrn(ModemFields,NUMMODEM-1);
}
