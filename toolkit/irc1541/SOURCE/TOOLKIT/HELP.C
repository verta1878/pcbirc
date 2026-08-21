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


#include "pcbtools.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define TXT_MOREHELP_ENTER     340
#define TXT_MOREHELP_YES       341
#define TXT_MOREHELP_NO        342
#define TXT_MOREHELP_NONSTOP   343

void LIBENTRY customhelp(int HelpNum);

int LIBENTRY displayhelpfile(int HelpNum) {
  bool Temp;

  if (HelpNum == NOHELP)
    return(0);

  Temp = Display.CountLines;
  startdisplay(NOCHANGE);

  if (HelpNum == HLP_MORE) {
    displaypcbtext(TXT_MOREHELP_ENTER  ,NEWLINE|LFBEFORE);
    displaypcbtext(TXT_MOREHELP_YES    ,NEWLINE);
    displaypcbtext(TXT_MOREHELP_NO     ,NEWLINE);
    displaypcbtext(TXT_MOREHELP_NONSTOP,NEWLINE);
  } else {
    startdisplay(FORCECOUNTLINES);
    customhelp(HelpNum);
  }

  Display.CountLines = Temp;
  return(0);
}
