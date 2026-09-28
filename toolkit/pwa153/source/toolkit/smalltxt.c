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


#ifdef _MSC_VER
#include <malloc.h>
#else
#include <alloc.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcb.h>
#include <misc.h>
#include <screen.h>
#include "ansi.h"
#include "pcboard.h"
#include "pcboard.ext"
#include "pcbtext.h"
#include "users.h"
#ifdef DEBUG
#include <memcheck.h>
#endif


void LIBENTRY closepcbtext(void) {
}


static int _NEAR_ LIBENTRY processtext(pcbtexttype *Buf, int TextNum) {
  char *p;

  switch (TextNum) {
    case TXT_HELPLINE1        : p = "Alt-> F=Out I=In N=Next X=DOS P=Printer T=FmFeed F1/F2=Time F9/F10=Security";
                                Buf->Color = 0;
                                break;
    case TXT_HELPLINE2        : p = "1=SyPrv 2=LkOut 3=Print 4=Bell 5=SHELL 6=Reg 7=Alarm 8=HngUp 9=Screen 10=Chat";
                                Buf->Color = 0;
                                break;
    case TXT_NOTFOUNDONDISK   : p = "(@OPTEXT@) not found on disk!";
                                Buf->Color = 0x0C;
                                break;
    case TXT_DISKFULL         : p = "Disk Full - System presently unavailable!";
                                Buf->Color = 0x0C;
                                break;
    case TXT_NETWORKDELAY     : p = "Network Delay - Please Wait, @FIRST@ ...";
                                Buf->Color = 0x0C;
                                break;
    case TXT_MOREPROMPT       : p = "(@TIMELEFT@ min left), (H)elp, More";
                                Buf->Color = 0x0E;
                                break;
    case TXT_PRESSENTER       : p = "Press (Enter) to continue";
                                Buf->Color = 0x0A;
                                break;
    case TXT_AUTODISCONNECTNOW: p = "Automatic Disconnect Completed!";
                                Buf->Color = 0x0C;
                                break;
    case TXT_MOREHELP_ENTER   : p = "  (Enter) continues on with display     ";
                                Buf->Color = 0;
                                break;
    case TXT_MOREHELP_YES     : p = "  (Y) yes, continue on with display     ";
                                Buf->Color = 0;
                                break;
    case TXT_MOREHELP_NO      : p = "  (N) no, stop displaying this text     ";
                                Buf->Color = 0;
                                break;
    case TXT_MOREHELP_NONSTOP : p = "  (NS) continue reading in non-stop mode";
                                Buf->Color = 0;
                                break;
    case TXT_THANKSFORCALLING : p = "Thanks for calling, @FIRST@!";
                                Buf->Color = 0x0A;
                                break;
  }

  strcpy(Buf->Str,p);
  return(strlen(Buf->Str)+2);        /* add 2:  1 for color, 1 for NULL */
}


#pragma warn -par
int LIBENTRY readpcbtextfile(char *Extension, callfromtype CalledFrom) {
  return(0);
}
#pragma warn +par


bool LIBENTRY getpcbtext(int PcbTextNum, pcbtexttype *Buffer) {
  processtext(Buffer,PcbTextNum);
  return(substitute(Buffer->Str,"@OPTEXT@",Status.DisplayText,sizeof(Buffer->Str)));
}


bool LIBENTRY displaypcbtext(int PcbTextNum, DISPLAYTYPE Display) {
  pcbtexttype Buffer;

  getpcbtext(PcbTextNum,&Buffer);

  if (Display & BELL)
    bell();

  if (Display & LFBEFORE)
    newline();

  if (Control.GraphicsMode && Buffer.Color != 0)
    printcolor(Buffer.Color);

  printxlated(Buffer.Str);

  if (Display & NEWLINE)
    newline();
  if (Display & LFAFTER)
    newline();

  if (Display & LOGIT)
    writelog(Buffer.Str,SPACERIGHT);
  else if (Display & LOGITLEFT)
    writelog(Buffer.Str,LEFTJUSTIFY);

  return(FALSE);
}


#pragma warn -par
bool LIBENTRY pcbtextspaces(int TextNum) {
  return(FALSE);
}
#pragma warn +par

void LIBENTRY smalltext(void) {
}
