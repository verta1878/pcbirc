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
#include <screen.h>
#include <dosfunc.h>
#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

#define HELP_LEFT   6
#define HELP_TOP    6
#define HELP_RIGHT  73
#define HELP_BOTTOM 22
#define HELP_LINES  10
#define HELP_PAGELN 8
#define HELP_TOPLN  11
#define HELP_MARGIN 8


int  GeneralHelpNum;


/********************************************************************
 *
 *  Function:  blankfill()
 *
 *  Returns a str which is equal to S[] but has been blank filled from the end
 *  of S[] on to a maximum length of Num.
 */

char * _NEAR_ LIBENTRY blankfill(char *S, int Num) {
  int L;
  static char RetStr[80];

  strcpy(RetStr,S);
  L = strlen(RetStr);
  if (L < Num) {
    memset(&RetStr[L],' ',Num-L);
    RetStr[Num] = 0;
  }
  return(RetStr);
}


/********************************************************************
 *
 *  Function:  center()
 *
 *  Returns a string equal to S[] but which has been centered inside of a field
 *  Num characters long with the character C filling in both the front and back
 *  ends of the string.
 */

char * _NEAR_ LIBENTRY center(char *S, int Num, char C) {
  int X,L;
  static char RetStr[80];

  X = strlen(S);
  L = (Num >> 1) - (X >> 1);
  if (L > 0) {
    memset(RetStr,C,Num);
    RetStr[Num] = 0;
    memcpy(&RetStr[L],S,X);
    return(RetStr);
  } else
    return(S);
}


/********************************************************************
 *
 *  Function:  showhelp()
 *
 *  Takes as input the number of the help screen to display.  It then opens up
 *  a window and displays the appropriate text from the help file.  It allows
 *  the user to page back and forth according to the capabilities of the help
 *  file.  When done, the screen is restored to the way it was before the help
 *  screen was opened up.
 */

void LIBENTRY showhelp(int HelpNum) {
  char static    PageUp[3] = {2,'B','E'};
  char static    PageDn[3] = {2,'F','E'};
  char           Ch;
  char           Ext;
  bool           ExitHelp;
  int            X;
  int            TextLen;
  int            SaveCursor;
  char          *p;
  char          *HelpLine;
  char           UnderLine[80];
  char           OutLine[80];
  HelpTableType  HelpTable;
  HelpTitleType  HelpTitle;
  HelpHeaderType HelpHeader;
  HelpBodyType   HelpBody;
  savescrntype   ScrnBuf;
  #ifdef __OS2__
  os2errtype     Os2Error;
  #endif

  if (HelpNum) {
    savescreen(&ScrnBuf);
    boxcls(HELP_LEFT-1,HELP_TOP-2,HELP_RIGHT+1,HELP_BOTTOM+1,Colors[HELPBOX],HDOUBLE);
    SaveCursor = getcursor();
    setcursor(CUR_BLANK);

    if (HelpFile <= 0) {
      fastcenter(HELP_TOP,(HelpFile == 0 ?
                 "The PCBSM.HLP file is missing!" :
                 "The PCBSM.HLP file is the wrong version!"),
                 Colors[HELPTITLE]);
      goto getkey;
    }

    HelpNum--;
    memset(UnderLine,'Ä',HELPLINEWIDTH);
    fastprint(59,HELP_PAGELN,"page:",Colors[HELPBOX]);

    do {
      memset(&HelpBody,0,sizeof(HelpBodyType));
      doslseek(HelpFile,HelpNum * sizeof(HelpTableType),SEEK_SET);
      dosread(HelpFile,&HelpTable,sizeof(HelpTableType) POS2ERROR);

      doslseek(HelpFile,HelpTable.OffsetToTitle,SEEK_SET);
      dosread(HelpFile,&HelpTitle,sizeof(HelpTitleType) POS2ERROR);

      doslseek(HelpFile,HelpTable.OffsetToHeader,SEEK_SET);
      dosread(HelpFile,&HelpHeader,sizeof(HelpHeaderType) POS2ERROR);
      dosread(HelpFile,&HelpBody,sizeof(HelpBodyType) POS2ERROR);

      switch (HelpHeader.ForwBack) {
        case 'N': HelpLine = "           press";             break;
        case 'F': HelpLine = "press  PGDN  to go forward, "; break;
        case 'B': HelpLine = "press  PGUP  to go backward,"; break;
        case 'E': HelpLine = "PGDN Í forward, PGUP Í back,"; break;
      }
      strcpy(OutLine,"   ");
      strcpy(&OutLine[3] ,HelpLine);
      strcat(OutLine," F1 Í general help,   ESC to exit    ");
      fastprint(HELP_LEFT,HELP_BOTTOM,blankfill(OutLine,68),Colors[HELPDESC]);

      TextLen = strlen(HelpTitle.Title);
      UnderLine[TextLen] = 0;
      fastprint(HELP_MARGIN,HELP_TOP,center(HelpTitle.Title,65,' '),Colors[HELPTITLE]);
      fastprint(HELP_MARGIN,HELP_TOP+1,center(UnderLine,65,' '),Colors[HELPTITLE]);
      UnderLine[TextLen] = 196;

      sprintf(OutLine,"%02d of %02d",HelpHeader.PageNum,HelpHeader.NumPages);
      fastprint(65,HELP_PAGELN,OutLine,Colors[HELPBOX]);

      p = HelpBody.Text;
      fastprint(HELP_MARGIN,HELP_TOP+3,center(p,65,' '),Colors[HELPSUB]);

      for (X = 0, p += strlen(p)+1; X < HELP_LINES; X++, p += strlen(p)+1) {
        if (X >= HelpHeader.NumLines)  /* if we go over then EMPTY the string */
          *p = 0;
        fastprint(HELP_MARGIN,HELP_TOPLN+X,blankfill(p,65),Colors[HELPTEXT]);
      }

getkey:
      Ch = inkey(&Ext,ShowClock);
      ExitHelp = (bool) (HelpFile <= 0 || (!Ext && Ch == 27));

      if (! ExitHelp && Ext) {
        switch (Ch) {
          case 59: HelpNum = GeneralHelpNum - 1; break;
          case 73: if (charinmask(HelpHeader.ForwBack,PageUp))
                     HelpNum--;
                   break;
          case 81: if (charinmask(HelpHeader.ForwBack,PageDn))
                     HelpNum++;
                   break;
        }
      }

    } while (! ExitHelp);

goback:
    restorescreen(&ScrnBuf);
    setcursor(SaveCursor);
    showtime();
    showkeystatus();
  }
}
