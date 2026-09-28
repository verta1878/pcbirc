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
#include <misc.h>
#include <newdata.h>
#include <pcb.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include "pcbfiles.h"
#include "pcbfiles.ext"
#ifdef DEBUG
#include <memcheck.h>
#endif

bool BatchMode;

int pascal intparam(char *Param) {
  if ((Param = strchr(Param,':')) != NULL)
    return(atoi(++Param));
  return(0);
}

unsigned pascal dateparam(char *Param) {
  unsigned Date;

  if ((Param = strchr(Param,':')) != NULL) {
    Date = datetojulian(++Param);
    if (Date != 0 && Date < DateLimit)
      Date = DateLimit;
  } else
    Date = DateLimit;
  return(Date);
}

void pascal checkforretry(int argc, char *argv[]) {
  for (argc--; argc > 0; argc--) {
    if (argv[argc][0] == '/') {
      strupr(argv[argc]);
      if (findfour("RETR",&argv[argc][1]) == 1)
        MaxRetryCount = intparam(&argv[argc][1])+3;
    }
  }
}

void pascal batch(char *Str) {
  char *p;                    /* 1   2   3   4   5   6   7   8   9   10  11  12  13  14*/
  static char BatchCommands[] = "PACKSORTINDEADJUGROUEXPISTANCOPYUPGRADDTSECUEXPSPURGEDIT";
  char CommandLine[120];

  if (Str[0] == '/') {
    strupr(Str);
    maxstrcpy(CommandLine,&Str[1],sizeof(CommandLine));
    p = parsepaths(CommandLine);
    switch(findfour(BatchCommands,p)) {
      case  0:                              break;
      case  1: MenuSelection = 2;
               pack();
               break;
      case  2: sortall();                   break;
      case  3: createallindexes();          break;
      #ifndef DEMO
      case 11: /* adjust security  -  fall thru on both of these because */
      case 12: /* adjust expired      the manual was printed wrong       */

               maxstrcpy(&CommandLine[1],Str,sizeof(CommandLine)-1);
               CommandLine[0]='X';      /* force an dummy first parameter    */
               CommandLine[1]=';';      /* with a separator to the next then */
               parsepaths(CommandLine); /* re-parse it so that the routine   */  //lint !e534
                                        /* can find it as the second parm    */
               /* fall thru */
      case  4: adjustsecurity();            break;
      #endif
      case  5: adjustconf();                break;
      case  6: expdate();                   break;
      case  7: standardizephone();          break;
      #ifndef DEMO
      case  8: copyexpiredlevel();          break;
      case  9: createuserinfo();            break;
      case 10: updatetpa();                 break;
      case 13: MenuSelection = 3;
               pack();
               break;
      case 14: BatchMode = FALSE;
               if ((p = strchr(Str,':')) != NULL)
                 editusername(p+1);
               break;
      #endif
    }
  }
}
