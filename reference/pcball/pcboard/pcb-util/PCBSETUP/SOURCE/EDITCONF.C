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


#include <alloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
#include <dosfunc.h>
#include <misc.h>
#include <pcb.h>
#include <cnameidx.h>
#include <help.h>
#include "setup.h"
#include "setup.ext"
#include "fidocfg.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#define CONFMENU       1
#define MENUBOXLINE    8
#define MENUINPUTLINE  (MENUBOXLINE+2)
#define MENUFOUNDLINE  (MENUBOXLINE+5)
#define MENUINBOXLINE  (MENUBOXLINE+6)

       pcbconftype Conf;
static FldType     *ConfFields;
static FldType     *AddConfFields;
       unsigned    ConfSel;
static int         StartField;
static int         AddStartField;
static char        SaveName[80];
       bool        ConfNameChanged;


int checkname(int Before) {
  if (Before) {
    strcpy(SaveName,Conf.Name);
    if (Conf.Name[0] == 0) {
      confdefaults(ConfSel,&Conf);
      if (Conf.Name[0] != 0)
        return(1);
    }
  } else {
    stripright(Conf.Name,' ');
    if (strcmp(SaveName,Conf.Name) != 0)
      ConfNameChanged = TRUE;
  }
  return(0);
}

int jumpblt(int Before) {
  showeditmsg(Before,"BLT list");
  if (KeyFlags == FLAG3 && ! Before)
    editblt(Conf.BltNameLoc,"Enter a list of BLT Paths & Names");
  return(0);
}

int jumpscr(int Before) {
  showeditmsg(Before,"SCRIPT list");
  if (KeyFlags == FLAG3 && ! Before)
    editscript(Conf.ScrNameLoc);
  return(0);
}

int jumpdir(int Before) {
  showeditmsg(Before,"DIR list");
  if (KeyFlags == FLAG3 && ! Before)
    editdirlist(Conf.DirNameLoc);
  return(0);
}

int jumppath(int Before) {
  showeditmsg(Before,"DOWNLOAD PATH list");
  if (KeyFlags == FLAG3 && ! Before)
    editpath(Conf.PthNameLoc,0);
  return(0);
}

int jumpdoor(int Before) {
  showeditmsg(Before,"DOORS Security");
  if (! Before) {
    if (checkfiles8(FALSE))
      return(-1);
    if (KeyFlags == FLAG3)
      editdoor(Conf.DrsFile);
  }
  return(0);
}

int checkmsgblck(int Before) {
  if (Before)
    return(0);

  if ((Conf.MsgBlocks >= 1 && Conf.MsgBlocks <= 32) || ! Conf.OldIndex)
    return(0);

  beep();
  showhelp(ERRMSGBLCK);
  return(-1);
}

int jumpusrmenu(int Before) {
  showeditmsg2(Before,"BRDM");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

int jumpsysmenu(int Before) {
  showeditmsg2(Before,"BRDS");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

int jumpnewfile(int Before) {
  showeditmsg2(Before,"NEWS");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

int jumpintrofile(int Before) {
  showeditmsg2(Before,"INTRO");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

int jumpdrsmenu(int Before) {
  showeditmsg2(Before,"DOORS");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

int jumpbltmenu(int Before) {
  showeditmsg2(Before,"BLT");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

int jumpscrmenu(int Before) {
  showeditmsg2(Before,"SCRIPT");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}

int jumpdirmenu(int Before) {
  showeditmsg2(Before,"DIR");
  if (Before)
    return(0);
  return(externaledit(EDIT_CHECK7|EDIT_GRAPHICS));
}


static void near pascal saveconfrecord(void) {
  stripright(Conf.Name,' ');
  stripall(Conf.MsgFile,' ');
  stripall(Conf.UserMenu,' ');
  stripall(Conf.SysopMenu,' ');
  stripall(Conf.NewsFile,' ');
  stripall(Conf.UpldDir,' ');
  stripall(Conf.PubUpldLoc,' ');
  stripall(Conf.PrivDir,' ');
  stripall(Conf.PrvUpldLoc,' ');
  stripall(Conf.DrsMenu,' ');
  stripall(Conf.DrsFile,' ');
  stripall(Conf.BltMenu,' ');
  stripall(Conf.BltNameLoc,' ');
  stripall(Conf.ScrMenu,' ');
  stripall(Conf.ScrNameLoc,' ');
  stripall(Conf.DirMenu,' ');
  stripall(Conf.DirNameLoc,' ');
  stripall(Conf.PthNameLoc,' ');

  stripall(Conf.Password,' ');
  stripall(Conf.Intro,' ');
  stripall(Conf.AttachLoc,' ');
  stripall(Conf.RegFlags,' ');
  stripall(Conf.CmdLst,' ');

  putconfrecord(ConfSel,&Conf);
}


/********************************************************************
*
*  Function: initconffields()
*
*  Desc    : Creates a table of fields to be used for editing the users file
*            when performing the readscrn() function.
*/

static void near pascal initconffields(FldType *P) {
  addquest(P, 0,vSTR    ,CNAMESHELP+ 0,ALLCONF, 2, 1,60,Questions[FLDCONFRNCE+ 0], Conf.Name       ,CLEAR     ,checkname);
  addquest(P, 1,vBOOL   ,CNAMESHELP+ 1,YESNO  , 2, 2, 1,Questions[FLDCONFRNCE+ 1],&Conf.PublicConf ,NOCLEARFLD,NULL);
  addquest(P, 2,vINT    ,CNAMESHELP+ 2,ALLNUM ,45, 2, 3,Questions[FLDCONFRNCE+ 2],&Conf.ReqSecLevel,CLEAR     ,NULL);
  addquest(P, 3,vUPSTR  ,CNAMESHELP+ 4,ALLCHAR, 2, 3,12,Questions[FLDCONFRNCE+ 3], Conf.Password   ,CLEAR     ,NULL);
  addquest(P, 4,vUPSTR  ,CNAMESHELP+ 5,RXS    ,45, 3, 3,Questions[FLDCONFRNCE+ 4], Conf.RegFlags   ,NOCLEARFLD,NULL);

  addquest(P, 5,vBYTE   ,CNAMESHELP+ 6,ALLNUM , 2, 5, 2,Questions[FLDCONFRNCE+ 5],&Conf.MsgBlocks  ,CLEAR     ,checkmsgblck);
  addquest(P, 6,vUPSTR  ,CNAMESHELP+ 8,ALLFILE, 2, 6,31,Questions[FLDCONFRNCE+ 6], Conf.MsgFile    ,NOCLEARFLD,checkfiles8n);
  addquest(P, 7,vUPSTR  ,CNAMESHELP+ 9,ALLFILE, 2, 7,31,Questions[FLDCONFRNCE+ 7], Conf.UserMenu   ,NOCLEARFLD,jumpusrmenu);
  addquest(P, 8,vUPSTR  ,CNAMESHELP+10,ALLFILE, 2, 8,31,Questions[FLDCONFRNCE+ 8], Conf.SysopMenu  ,NOCLEARFLD,jumpsysmenu);
  addquest(P, 9,vUPSTR  ,CNAMESHELP+11,ALLFILE, 2, 9,31,Questions[FLDCONFRNCE+ 9], Conf.NewsFile   ,NOCLEARFLD,jumpnewfile);
  addquest(P,10,vUPSTR  ,CNAMESHELP+12,ALLFILE, 2,10,31,Questions[FLDCONFRNCE+10], Conf.Intro      ,NOCLEARFLD,jumpintrofile);
  addquest(P,11,vUPSTR  ,CNAMESHELP+13,ALLFILE, 2,11,31,Questions[FLDCONFRNCE+11], Conf.AttachLoc  ,NOCLEARFLD,backslash);

  addquest(P,12,vBYTE   ,CNAMESHELP+14,ALLNUM ,13,14, 1,"",&Conf.PubUpldSort,NOCLEARFLD,NULL);
  addquest(P,13,vUPSTR  ,CNAMESHELP+15,ALLFILE,19,14,28,"", Conf.UpldDir    ,NOCLEARFLD,checkfiles8n);
  addquest(P,14,vUPSTR  ,CNAMESHELP+16,ALLFILE,50,14,25,"", Conf.PubUpldLoc ,NOCLEARFLD,backslash);
  addquest(P,15,vBYTE   ,CNAMESHELP+17,ALLNUM ,13,15, 1,"",&Conf.PrvUpldSort,NOCLEARFLD,NULL);
  addquest(P,16,vUPSTR  ,CNAMESHELP+18,ALLFILE,19,15,28,"", Conf.PrivDir    ,NOCLEARFLD,checkfiles8n);
  addquest(P,17,vUPSTR  ,CNAMESHELP+19,ALLFILE,50,15,25,"", Conf.PrvUpldLoc ,NOCLEARFLD,backslash);

  addquest(P,18,vUPSTR  ,CNAMESHELP+20,ALLFILE,13,18,28,"", Conf.DrsMenu    ,NOCLEARFLD,jumpdrsmenu);
  addquest(P,19,vUPSTR  ,CNAMESHELP+21,ALLFILE,44,18,32,"", Conf.DrsFile    ,NOCLEARFLD,jumpdoor);
  addquest(P,20,vUPSTR  ,CNAMESHELP+22,ALLFILE,13,19,28,"", Conf.BltMenu    ,NOCLEARFLD,jumpbltmenu);
  addquest(P,21,vUPSTR  ,CNAMESHELP+23,ALLFILE,44,19,32,"", Conf.BltNameLoc ,NOCLEARFLD,jumpblt);
  addquest(P,22,vUPSTR  ,CNAMESHELP+24,ALLFILE,13,20,28,"", Conf.ScrMenu    ,NOCLEARFLD,jumpscrmenu);
  addquest(P,23,vUPSTR  ,CNAMESHELP+25,ALLFILE,44,20,32,"", Conf.ScrNameLoc ,NOCLEARFLD,jumpscr);
  addquest(P,24,vUPSTR  ,CNAMESHELP+26,ALLFILE,13,21,28,"", Conf.DirMenu    ,NOCLEARFLD,jumpdirmenu);
  addquest(P,25,vUPSTR  ,CNAMESHELP+27,ALLFILE,44,21,32,"", Conf.DirNameLoc ,NOCLEARFLD,jumpdir);
  addquest(P,26,vUPSTR  ,CNAMESHELP+28,ALLFILE,44,22,32,"", Conf.PthNameLoc ,NOCLEARFLD,jumppath);
}


static void near pascal initaddconffields(FldType *P) {
  addquest(P, 0,vBOOL   ,CNAMESHELP+29,YESNO  , 2, 3, 1,Questions[FLDADDCONF+ 0],&Conf.AutoRejoin      ,NOCLEARFLD,NULL);
  addquest(P, 1,vBOOL   ,CNAMESHELP+30,YESNO  , 2, 4, 1,Questions[FLDADDCONF+ 1],&Conf.ViewMembers     ,NOCLEARFLD,NULL);
  addquest(P, 2,vBOOL   ,CNAMESHELP+31,YESNO  , 2, 5, 1,Questions[FLDADDCONF+ 2],&Conf.PrivUplds       ,NOCLEARFLD,NULL);
  addquest(P, 3,vBOOL   ,CNAMESHELP+32,YESNO  , 2, 6, 1,Questions[FLDADDCONF+ 3],&Conf.PrivMsgs        ,NOCLEARFLD,NULL);
  addquest(P, 4,vBOOL   ,CNAMESHELP+33,YESNO  , 2, 7, 1,Questions[FLDADDCONF+ 4],&Conf.EchoMail        ,NOCLEARFLD,NULL);
  addquest(P, 5,vBOOL   ,CNAMESHELP+34,YESNO  , 2, 8, 1,Questions[FLDADDCONF+ 5],&Conf.ForceEcho       ,NOCLEARFLD,NULL);
  addquest(P, 6,vBYTE   ,CNAMESHELP+35,ALLNUM , 2, 9, 1,Questions[FLDADDCONF+ 6],&Conf.ConfType        ,NOCLEARFLD,NULL);
  addquest(P, 7,vBOOL   ,CNAMESHELP+36,YESNO  , 2,10, 1,Questions[FLDADDCONF+ 7],&Conf.LongToNames     ,NOCLEARFLD,NULL);
  addquest(P, 8,vBOOL   ,CNAMESHELP+37,YESNO  , 2,11, 1,Questions[FLDADDCONF+ 8],&Conf.ReadOnly        ,NOCLEARFLD,NULL);
  addquest(P, 9,vBOOL   ,CNAMESHELP+38,YESNO  , 2,12, 1,Questions[FLDADDCONF+ 9],&Conf.NoPrivateMsgs   ,NOCLEARFLD,NULL);
  addquest(P,10,vBOOL   ,CNAMESHELP+39,YESNO  , 2,13, 1,Questions[FLDADDCONF+10],&Conf.RecordOrigin    ,NOCLEARFLD,NULL);
  addquest(P,11,vBOOL   ,CNAMESHELP+40,YESNO  , 2,14, 1,Questions[FLDADDCONF+11],&Conf.PromptForRouting,NOCLEARFLD,NULL);
  addquest(P,12,vBOOL   ,CNAMESHELP+41,YESNO  , 2,15, 1,Questions[FLDADDCONF+12],&Conf.AllowAliases    ,NOCLEARFLD,NULL);
  addquest(P,13,vBOOL   ,CNAMESHELP+42,YESNO  , 2,16, 1,Questions[FLDADDCONF+13],&Conf.ShowIntroOnRA   ,NOCLEARFLD,NULL);
  addquest(P,14,vBOOL   ,CNAMESHELP+43,YESNO  , 2,17, 1,Questions[FLDADDCONF+14],&Conf.OldIndex        ,NOCLEARFLD,NULL);
  addquest(P,15,vUPSTR  ,CNAMESHELP+44,ALLFILE, 2,18,30,Questions[FLDADDCONF+15], Conf.CmdLst          ,NOCLEARFLD,jumpcmdfile);

  addquest(P,16,vINT    ,CNAMESHELP+45,ALLNUM ,40, 3, 3,Questions[FLDADDCONF+16],&Conf.AddSec          ,CLEAR     ,NULL);
  addquest(P,17,vINT    ,CNAMESHELP+46,ALLNUM ,40, 4, 3,Questions[FLDADDCONF+17],&Conf.AddTime         ,CLEAR     ,NULL);
  addquest(P,18,vBYTE   ,CNAMESHELP+47,ALLNUM ,40, 5, 3,Questions[FLDADDCONF+18],&Conf.AttachLevel     ,CLEAR     ,NULL);
  addquest(P,19,vBYTE   ,CNAMESHELP+48,ALLNUM ,40, 6, 3,Questions[FLDADDCONF+19],&Conf.ReqLevelToEnter ,CLEAR     ,NULL);
  addquest(P,20,vBYTE   ,CNAMESHELP+49,ALLNUM ,40, 7, 3,Questions[FLDADDCONF+20],&Conf.RetReceiptLevel ,CLEAR     ,NULL);
  addquest(P,21,vBYTE   ,CNAMESHELP+50,ALLNUM ,40, 8, 3,Questions[FLDADDCONF+21],&Conf.CarbonLevel     ,CLEAR     ,NULL);
  addquest(P,22,vBYTE   ,CNAMESHELP+51,ALLNUM ,40, 9, 3,Questions[FLDADDCONF+22],&Conf.CarbonLimit     ,CLEAR     ,NULL);

  addquest(P,23,vFLOAT  ,CNAMESHELP+52,ALLNUM ,40,10, 8,Questions[FLDADDCONF+23],&Conf.ChargeTime      ,CLEAR     ,NULL);
  addquest(P,24,vFLOAT  ,CNAMESHELP+53,ALLNUM ,40,11, 8,Questions[FLDADDCONF+24],&Conf.ChargeMsgRead   ,CLEAR     ,NULL);
  addquest(P,25,vFLOAT  ,CNAMESHELP+54,ALLNUM ,40,12, 8,Questions[FLDADDCONF+25],&Conf.ChargeMsgWrite  ,CLEAR     ,NULL);
  addquest(P,26,vLONG   ,CNAMESHELP+55,ALLNUM ,40,13, 8,Questions[FLDADDCONF+26],&Conf.ExportPtr       ,CLEAR     ,NULL);
}


static void _NEAR_ LIBENTRY showrec(int LineNum, cnamesidxtype *IdxRec, bool Highlight) {
  char Temp[80];

  sprintf(Temp,"%5u) %s",IdxRec->Num,IdxRec->Name);
  padstr(Temp,' ',66);
  fastprint(7,LineNum,Temp,Colors[(Highlight ? MENUBAR : MENUSELECT)]);
}


static void _NEAR_ LIBENTRY showconfmenu(unsigned Rec, cnamesidxtype *IdxRec, int Select) {
  int  X;
  bool Found;

  for (X = MENUINBOXLINE+1, Found = TRUE; X <= MENUINBOXLINE+6; X++) {
    if (Found) {
      showrec(X,IdxRec,(bool) (Select == (X - (MENUINBOXLINE+1))));
      if (getconfbyrec(++Rec,IdxRec) == -1)
        Found = FALSE;
    } else
      clsbox(7,X,72,X,Colors[MENUSELECT]);
  }
}


void LIBENTRY confnamesearch(char *Str) {
  unsigned      Rec;
  cnamesidxtype IdxRec;

  if ((Rec = findconfbyname(Str,&IdxRec)) != (unsigned) -1)
    showconfmenu(Rec,&IdxRec,0);
}


static unsigned _NEAR_ LIBENTRY confnamemenu(unsigned TopRec, int First) {
  bool           Extended;
  int            Key;
  int            X,Y;
  int            Select;
  cnamesidxtype  IdxRec;

  X = wherex();
  Y = wherey();
  Select = 0;
  if (First != Select) {
    setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUSELECT]);
    Select = First;
    setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUBAR]);
  }
  gotoxy(14,MENUINBOXLINE+Select+1);

  while (1) {
    Key = inkey(&Extended,CLOCK);
    if (Extended) {
      switch (Key) {
        case 72: if (Select == 0) {
                   if (TopRec > 0) {
                     if (getconfbyrec(TopRec-1,&IdxRec) != -1) {
                       TopRec--;
                       showconfmenu(TopRec,&IdxRec,0);
                     }
                   }
                 } else {
                   setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUSELECT]);
                   Select--;
                   setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUBAR]);
                   gotoxy(14,MENUINBOXLINE+Select+1);
                 }
                 break;
        case 80: if (Select < 5) {
                   setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUSELECT]);
                   Select++;
                   setatt(7,MENUINBOXLINE+Select+1,72,MENUINBOXLINE+Select+1,Colors[MENUBAR]);
                   gotoxy(14,MENUINBOXLINE+Select+1);
                 } else {
                   if (getconfbyrec(TopRec+1,&IdxRec) != -1) {
                     TopRec++;
                     showconfmenu(TopRec,&IdxRec,5);
                   }
                 }
                 break;
      }
    } else {
      switch (Key) {
        case  9: // tab or shift-tab, return -1
        case 15: gotoxy(X,Y);
                 return((unsigned) -1);
        case 27: return(0);
        case 13: getconfbyrec(TopRec+Select,&IdxRec);
                 return(IdxRec.Num);
      }
    }
  }
}


/********************************************************************
*
*  Function:  promptuserforconfname()
*
*/

int LIBENTRY promptuserforconfname(int Ch) {
  char           X,Y;
  int            CursorStatus;
  uint           Num;
  uint           Pos;
  cnamesidxtype  Rec;
  char           Temp[61];
  savescrntype   Scrn;

  if (Ch < ' ')
    return(0);

  savescreen(&Scrn);

  CursorStatus = getcursor();
  X = wherex();
  Y = wherey();

  opencnamesidx(FALSE);

// 16  --
// 17  |
// 18  | Begin
// 19  |                                                                |
// 20  |                                                                |
// 21  | Found:
// 22  |
// 23  ---

  boxcls(5,MENUBOXLINE,74,MENUBOXLINE+15,Colors[OUTBOX],DOUBLE);
  fastprint(7,MENUINPUTLINE,"Enter Conference Name (press TAB or DOWN for menu)",Colors[DISPLAY]);
  fastprint(7,MENUFOUNDLINE,"Conferences Found:",Colors[DISPLAY]);
  boxcls(6,MENUINBOXLINE,73,MENUINBOXLINE+8,Colors[MENUBOX],SINGLE);
  fastprint(7,MENUINBOXLINE+8,"  Use arrow keys to move bar, press ENTER to select, ESC to exit  ",Colors[MENUDESC]);

  Input.OrgX     = 7;
  Input.CurX     = 7;
  Input.OrgY     = MENUINPUTLINE+1;
  Input.AnsLimit = 60;
  Input.ScrLimit = 60;
  Input.Old      = Temp;
  Input.Ans      = Temp;
  Input.Mask     = ALLCONF;
  Input.Keyed    = TRUE;
  Input.HelpNum  = 0;

  if (Ch != ' ') {
    Temp[0] = (char) Ch;
    Temp[1] = 0;
    Input.CurX++;
  } else
    Temp[0] = 0;

  confnamesearch(Temp);
  setcursor(CUR_NORMAL);

top:
  inputcallback = confnamesearch;
  inputall();
  inputcallback = NULL;

  Num = 0;
  if (KeyFlags != ESC) {
    Pos = findconfbyname(Input.Ans,&Rec);
    if (KeyFlags == TAB || KeyFlags == DN) {
      if ((Num = confnamemenu(Pos,(KeyFlags == DN ? 1 : 0))) == (unsigned) -1)
        goto top;
    } else {
      if (Pos != (unsigned) -1)
        Num = Rec.Num;
    }
  }

  closecnamesidx();

  restorescreen(&Scrn);
  setcursor(CursorStatus);
  gotoxy(X,Y);
  return(KeyFlags == ESC ? 0 : Num);
}


/********************************************************************
*
*  Function:  promptuserforconfnum()
*
*/

int LIBENTRY promptuserforconfnum(int Ch) {
  char Temp[6];
  int  CursorStatus;
  int  Num;
  char X,Y;

  Temp[0] = (char) Ch;
  Temp[1] = 0;

  CursorStatus = getcursor();
  X = wherex();
  Y = wherey();

  fastprint(49,5,"Conference Number ?",Colors[QUESTION]);
  Input.OrgX     = 69;
  Input.CurX     = 70;
  Input.OrgY     = 5;
  Input.AnsLimit = 5;
  Input.ScrLimit = 5;
  Input.Old      = Temp;
  Input.Ans      = Temp;
  Input.Mask     = ALLNUM;
  Input.Keyed    = TRUE;
  Input.HelpNum  = 0;
  setcursor(CUR_NORMAL);
  inputall();

  Num = atoi(Temp);
  setcursor(CursorStatus);
  clsbox(49,5,73,5,Colors[MENUBOX]);
  gotoxy(X,Y);
  return(KeyFlags == ESC ? 0 : Num);
}


void pascal conffunc(void) {
  char    Head[20];
  int     First;
  int     Which;
  int     NumFields;
  FldType *p;

  ConfSel = MenuSelection + 1;
  if (ConfSel > PcbData.NumConf) {
    beep();
    KeyFlags = ESC;
    return;
  }

//if (ConfSel == 0) {
//  p = &ConfFields[5];
//  NumFields = NUMCONFRNCE - 6;
//} else {
    p = ConfFields;
    NumFields = NUMCONFRNCE - 1;
//}

  First = Menu[CONFMENU].First;

#ifdef DEMO
  if (ConfSel > 1) {
    beep();
    memset(&MsgData,0,sizeof(MsgData));
    MsgData.Save    = TRUE;
    MsgData.AutoBox = TRUE;
    MsgData.Line1   = 18;
    MsgData.Msg1    = "In the DEMO version only the first conference is selectable";
    MsgData.Color1  = Colors[HEADING];
    showmessage();
    KeyFlags = ESC;
    return;
  }
#endif

  getconfrecord(ConfSel,&Conf);

  ExitKeyNum[2] = 60;  ExitKeyFlag[2] = FLAG3;  /*  F2  */
  ExitKeyNum[3] = 61;  ExitKeyFlag[3] = FLAG4;  /*  F3  */

  Which = (KeyFlags == PGUP ? 1 : 0);
  sprintf(Head,"(#%u)",ConfSel);
  sprintf(ConfFields[0].Question,"Name %-8.8s",Head);

  while (1) {
    if (UpdateBox)
      clsbox(1,1,78,23,Colors[OUTBOX]);
    else
      cls();

    switch (Which) {
      case 0: if (ConfSel == 0) {
                fastprint( 2,1,ConfFields[0].Question,Colors[QUESTION]);
                fastprint(16,1,":",Colors[ANSWER]);
                fastprint(18,1,Conf.Name,Colors[ANSWER]);
              }
              fastprint(15,13,"Sort   Name/Loc Upload DIR File       Location of Uploads",Colors[DISPLAY]);
              fastprint( 2,14,"Public  Upld",Colors[QUESTION]);
              fastprint( 2,15,"Private Upld",Colors[QUESTION]);
              fastprint(16,17,"Menu Listing",Colors[DISPLAY]);
              fastprint(49,17,"Path/Name List File",Colors[DISPLAY]);
              fastprint( 2,18,"Doors"      ,Colors[QUESTION]);
              fastprint( 2,19,"Bulletins"  ,Colors[QUESTION]);
              fastprint( 2,20,"Scripts"    ,Colors[QUESTION]);
              fastprint( 2,21,"Directories",Colors[QUESTION]);
              fastprint( 2,22,"Download Paths (listing only)",Colors[QUESTION]);
              StartField = readscrn(p,NumFields,StartField,"","",1,NOCLEARFLD);
              break;
      case 1: fastprint( 2,1,ConfFields[0].Question,Colors[QUESTION]);
              fastprint(16,1,":",Colors[ANSWER]);
              fastprint(18,1,Conf.Name,Colors[ANSWER]);

              fastprint( 2,20,"NetMail Conference Types:"    ,Colors[DISPLAY]);
              fastprint( 2,21,"0=Normal Conference"         ,Colors[DISPLAY]);
              fastprint( 2,22,"1=Internet E-mail"           ,Colors[DISPLAY]);
              fastprint(23,21,"2=Internet/Usenet Junk"      ,Colors[DISPLAY]);
              fastprint(23,22,"3=Usenet Moderated Newsgroup",Colors[DISPLAY]);
              fastprint(53,21,"4=Usenet Public Newsgroup"   ,Colors[DISPLAY]);
              fastprint(53,22,"5=Fido Conference"           ,Colors[DISPLAY]);

              AddStartField = readscrn(AddConfFields,NUMADDCONF-1,AddStartField,"","",1,NOCLEARFLD);
              break;
      #ifdef FIDO
      case 2: if (Conf.ConfType == 5)
                Configure_Fido();
              break;
      #endif
    }

    if (KeyFlags == ESC)
      break;
    if (KeyFlags == PGDN) {
      Which++;
      #ifdef FIDO
//    if (Which > 2 || (Which > 1 && ! PcbData.EnableFido))
      if (Which > 2 || (Which > 1 && Conf.ConfType != 5))
        break;
      #else
      if (Which > 1)
        break;
      #endif
    }
    if (KeyFlags == PGUP)
      if (--Which < 0)
        break;
  }

  ExitKeyNum[2] = 0;
  ExitKeyNum[3] = 0;

  saveconfrecord();

  if (ConfSel != 0)
    sprintf(MenuList[First+ConfSel-GroupOffset-1],"%u) %-13.13s",ConfSel,Conf.Name);
}


/********************************************************************
*
*  Function: makeconfmenu()
*
*  Desc    :
*/

static void near pascal makeconfmenu(unsigned Offset) {
  char     *q;
  unsigned t;
  unsigned x;
  unsigned y;

  x = Menu[CONFMENU].First;
  y = x + 42;
  q = MenuList[x];

  for (t = Offset;  x < y; q+=36, x++, t++) {
    if (t <= PcbData.NumConf) {
      getconfrecord(t,&Conf);
      MenuAvail[x] = TRUE;
      sprintf(q,"%u) %-13.13s",t,Conf.Name);
    } else {
      MenuAvail[x] = FALSE;
      sprintf(q,"%u)",t);
    }
  }
}


int pascal boundschecking(unsigned Selection, unsigned *Offset) {
  if (Selection == 65535U)
    return(-1);

  if (Selection < *Offset || Selection > *Offset + 41) {
    *Offset = ((Selection / 3) * 3);
    makeconfmenu(*Offset + 1);
  }
  return(0);
}


void pascal mainbrd(void) {
  #ifdef DEBUG
    mc_register(&Conf,sizeof(Conf));
  #endif

  if ((ConfFields = (FldType *) mallochk(NUMCONFRNCE * sizeof(FldType))) == NULL) {
    KeyFlags = ESC;
    return;
  }

  if ((AddConfFields = (FldType *) mallochk(NUMADDCONF * sizeof(FldType))) == NULL) {
    free(ConfFields);
    KeyFlags = ESC;
    return;
  }

  initconffields(ConfFields);
  initaddconffields(AddConfFields);
  StartField = 0;
  MenuSelection = 0xFFFF;
  conffunc();


  freescrn(AddConfFields,NUMADDCONF-1);
  freescrn(ConfFields,NUMCONFRNCE-1);

  #ifdef DEBUG
    mc_unregister(&Conf);
  #endif
}


void pascal conferences(void) {
  if ((ConfFields = (FldType *) mallochk(NUMCONFRNCE * sizeof(FldType))) == NULL) {
    KeyFlags = ESC;
    return;
  }

  if ((AddConfFields = (FldType *) mallochk(NUMADDCONF * sizeof(FldType))) == NULL) {
    free(ConfFields);
    KeyFlags = ESC;
    return;
  }

  initconffields(ConfFields);
  initaddconffields(AddConfFields);
  StartField = 0;

  promptusernum = promptuserforconfnum;
  promptuserstr = promptuserforconfname;
  makeconfmenu(1);
  bigmenusel(CONFMENU,CNAMESMENU,0,conffunc,boundschecking,TRUE);
  promptusernum = NULL;
  promptuserstr = NULL;

  freescrn(AddConfFields,NUMADDCONF-1);
  freescrn(ConfFields,NUMCONFRNCE-1);
}
