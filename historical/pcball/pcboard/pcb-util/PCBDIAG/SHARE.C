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
#include <stdio.h>
#include <string.h>
#include <dosfunc.h>
#include <newdata.h>
#include <pcb.h>
#include <misc.h>
#include "pcbdiag.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define MSGS_UNABLE   0
#define MSGS_OPEN     1
#define MSGS_FAILOK   2
#define MSGS_SUCCOK   3
#define MSGS_FAILERR  4
#define MSGS_SUCCERR  5
#define MSGS_RECORD   6

char TestName[13] = "$$smtest";
char *Msgs[] = {
  "unable to perform test",
  "Open DENY %s / attempt to share file by second process ",
  "failed    - OKAY\r\n",
  "succeeded - OKAY\r\n",
  "failed    - ERROR!\r\n",
  "succeeded - ERROR!\r\n",
  "Attempt to %s record #1 by %s process "
  };


void pascal checkshare(void) {
  int  H1;
  int  H2;
  bool DosShareLoaded;
  char Buffer[10];
  char Temp[160];
  char Tmp[200];

  DosShareLoaded = shareloaded();
  sprintf(Tmp,"DOS's SHARE command is%sloaded\r\n",(DosShareLoaded ? " " : " NOT "));
  dosfputs(Tmp,&prn);

  if (! DosShareLoaded && ShareStatus) {
    sprintf(Tmp,"However, a preliminary test of SHARE functions indicates that it is functioning\r\n");
    dosfputs(Tmp,&prn);
  }

  if (PcbData.Network) {
    if (ShareStatus)
      strcpy(Temp,"and PCBoard is set to make network calls");
    else
      strcpy(Temp,"but is REQUIRED for PCBoard to make network calls");
  } else {
    if (ShareStatus)
      strcpy(Temp,"but");
    else
      strcpy(Temp,"and");
    strcat(Temp," is not needed for current PCBoard configuration");
  }
  dosfputs(Temp,&prn);
  dosfputs("\r\n\r\n",&prn);

  if (ShareStatus) {
    dosfputs("\r\nTest SHARE functions:\r\n",&prn);

    if ((H1 = doscreate(TestName,OPEN_RDWR,OPEN_NORMAL)) == -1) {
      sprintf(Tmp,_strerror(Msgs[MSGS_UNABLE]));
      dosfputs(Tmp,&prn);
      return;
    }
    doswrite(H1,"test",4);
    dosclose(H1);

    sprintf(Tmp,Msgs[MSGS_OPEN],"ALL ");
    dosfputs(Tmp,&prn);

    if ((H1 = dosopen(TestName,OPEN_RDWR|OPEN_DENYRDWR)) == -1) {
      sprintf(Tmp,_strerror(Msgs[MSGS_UNABLE]));
      dosfputs(Tmp,&prn);
      return;
    }

    if ((H2 = dosopen(TestName,OPEN_RDWR|OPEN_DENYNONE)) == -1) {
      sprintf(Tmp,Msgs[MSGS_FAILOK]);
      dosfputs(Tmp,&prn);
    } else {
      sprintf(Tmp,Msgs[MSGS_SUCCERR]);
      dosfputs(Tmp,&prn);
      dosclose(H2);
    }
    dosclose(H1);

    sprintf(Tmp,Msgs[MSGS_OPEN],"NONE");
    dosfputs(Tmp,&prn);

    if ((H1 = dosopen(TestName,OPEN_RDWR | OPEN_DENYNONE)) == -1) {
      sprintf(Tmp,_strerror(Msgs[MSGS_UNABLE]));
      dosfputs(Tmp,&prn);
      return;
    }

    if ((H2 = dosopen(TestName,OPEN_RDWR | OPEN_DENYNONE)) == -1) {
      sprintf(Tmp,Msgs[MSGS_FAILERR]);
    } else {
      sprintf(Tmp,Msgs[MSGS_SUCCOK]);
      dosclose(H2);
    }
    dosfputs(Tmp,&prn);

    sprintf(Tmp,Msgs[MSGS_RECORD],"lock","first ");
    dosfputs(Tmp,&prn);

    if (lock(H1,0,4) == -1)
      sprintf(Tmp,Msgs[MSGS_FAILERR]);
    else
      sprintf(Tmp,Msgs[MSGS_SUCCOK]);
    dosfputs(Tmp,&prn);

    sprintf(Tmp,Msgs[MSGS_RECORD],"lock","second");
    dosfputs(Tmp,&prn);

    if (lock(H2,0,4) == -1)
      sprintf(Tmp,Msgs[MSGS_FAILOK]);
    else
      sprintf(Tmp,Msgs[MSGS_SUCCERR]);
    dosfputs(Tmp,&prn);

    sprintf(Tmp,Msgs[MSGS_RECORD],"read","second");
    dosfputs(Tmp,&prn);

    if (dosread(H2,Buffer,4) != 4)
      sprintf(Tmp,Msgs[MSGS_FAILOK]);
    else
      sprintf(Tmp,Msgs[MSGS_SUCCERR]);
    dosfputs(Tmp,&prn);

    dosfputs("\r\n",&prn);
    dosclose(H1);
    unlink(TestName);
  }
}

