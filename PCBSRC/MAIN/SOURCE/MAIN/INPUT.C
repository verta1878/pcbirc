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


#include "project.h"
#pragma hdrstop

#define WHITESPACE        " ,.;:!?"
#define iswhitespace(c)   (strchr(WHITESPACE,c) != NULL)

typedef enum { INPTDONE, INPTHELP, INPTREDO } inpttype;

char static OverFlow[80];
bool Redisplay = FALSE;

#ifndef LIB
static int SaveHelpNum;      /* used to hold the real help number when a PPL takes over the original inputfield() call */
char *LastDefaultAnswer;     /* a pointer to default answer for input */
char *LastAcceptedAnswer;    /* a pointer to the answer accepted */
#endif

char ExtKeyXlat[EXTHIGHKEY-EXTLOWKEY+1] = {
/* K_HOME   */ CTRL_W,
/* K_UP     */ CTRL_E,
/* K_PGUP   */ CTRL_R,
               0,
/* K_LEFT   */ CTRL_S,
               0,
/* K_RIGHT  */ CTRL_D,
               0,
/* K_END    */ CTRL_P,
/* K_DOWN   */ CTRL_X,
/* K_PGDN   */ CTRL_C,
/* K_INS    */ CTRL_V,
/* K_DEL    */ CTRL_G,
               0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/* K_CLEFT  */ CTRL_A,
/* K_CRIGHT */ CTRL_F,
/* K_CEND   */ CTRL_K};


static bool _NEAR_ LIBENTRY empty(char *Str) {
  while (*Str != 0) {
    if (*Str != ' ')
      return(FALSE);
    Str++;
  }
  return(TRUE);
}


static void _NEAR_ LIBENTRY makefilter(char *Filter, char *Mask, bool UpCase, bool Help, bool Stacked, bool FieldLen, bool HighAscii, bool YesNo) {
  int X;
  int Y;

  memset(Filter,0,256);
  Filter[8]   = CTRL_H;  /* BCKSPC */
  Filter[127] = CTRL_H;  /* BCKSPC */
  Filter[13]  = CTRL_M;  /* RET */

  Filter[CTRL_U] = CTRL_U;
  Filter[K_ESC]  = CTRL_U;

  if (FieldLen) {
    Filter[CTRL_A] = CTRL_A;   /* CTRL-LEFT */
    Filter[CTRL_S] = CTRL_S;   /* LEFT */
    Filter[CTRL_D] = CTRL_D;   /* RIGHT */
    Filter[CTRL_F] = CTRL_F;   /* CTRL-RIGHT */
    Filter[CTRL_G] = CTRL_G;   /* DEL */
    Filter[CTRL_H] = CTRL_H;   /* BKSPC */
    Filter[CTRL_I] = CTRL_I;   /* TAB */
    Filter[CTRL_K] = CTRL_K;   /* CTRL-END */
    Filter[CTRL_P] = CTRL_P;   /* END */
    Filter[CTRL_S] = CTRL_S;   /* LEFT */
    Filter[CTRL_V] = CTRL_V;   /* INSERT */
    Filter[CTRL_W] = CTRL_W;   /* HOME */
    Filter[127]    = CTRL_G;   /* change it from BCKSPC to DEL */
  }

  if (Help) {
    Filter['?'] = '?';
    Filter['H'] = 'H';
  }

  if (YesNo) {
    Filter[YesChar] = YesChar;
    Filter[NoChar] = NoChar;
  }

  if (Stacked) {
    Filter[' '] = ' ';
    Filter[';'] = ';';
  }

  for (X = 1; X <= Mask[0]; X++) {
    if (Mask[X] == 0) {
      for (Y = Mask[X+1]; Y <= Mask[X+2]; Y++)
        Filter[Y] = (char) Y;
      X += 2;
    } else
      Filter[Mask[X]] = Mask[X];
  }

  /* put the lowercase letters in so that the user can type them -    */
  /* then the input routine will convert them to uppercase afterwards */

  if (UpCase) {
    for (X = 'a'; X <= 'z'; X++) {
      if (Filter[X-32] != 0)
        Filter[X] = (char) X;
    }
  }

  if (HighAscii)
    for (X = 128; X <= 255; X++)
      Filter[X] = (char) X;
}


int LIBENTRY wordbackward(char *Str, int Pos) {
  int Save;

  if (Pos > 0) {
    Save = Pos;
    if (Pos > 1  && iswhitespace(Str[Pos-1]))
      Pos--;
    while (iswhitespace(Str[Pos]) && Pos >= 0) Pos--;
    while (! iswhitespace(Str[Pos]) && Pos >= 0) Pos--;
    Pos++;
    if (Pos == Save)
      return(Pos);
    backup(Save-Pos);
    updatelinesnow();
  }
  return(Pos);
}


int LIBENTRY wordforward(char *Str, int Pos, int FieldLen) {
  char Temp;
  int  Save;
  int  FirstSpace;

  /* Pos is 1-based - meaning the first position is #1 instead of #0   */
  /* so subtract one from the field length to avoid comparision errors */
  FieldLen--;

  if (Pos < FieldLen) {
    Temp = (char) lastcharinstr(Str,' ');
    if (Temp > Pos) {
      Save = Pos;
      while (! iswhitespace(Str[Pos]) && Pos < FieldLen) Pos++;
      FirstSpace = Pos;
      while (iswhitespace(Str[Pos]) && Pos < FieldLen) Pos++;
      if (Str[Pos] == 0)
        return(Save);
      if (Pos == FieldLen && iswhitespace(Str[Pos]))
        Pos = FirstSpace;
      if (UseAnsi)
        forward(Pos-Save);
      else {
        Temp = Str[Pos];
        Str[Pos] = 0;
        print(&Str[Save]);
        Str[Pos] = Temp;
      }
      updatelinesnow();
    }
  }
  return(Pos);
}


/********************************************************************
*
*  Function:  checkfornodechat()
*
*  Desc    :  This function checks the USERNET.DAT file *IF* it is time and
*             we're clear to do so - to see if someone wants to chat with us
*
*  Returns :  TRUE if a chat request has been sent, FALSE otherwise
*/

#ifndef LIB
static bool _NEAR_ LIBENTRY checkfornodechat(void) {
  if (Status.FileXfer || Status.EnteringMessage)
    return(FALSE);

  #ifdef __OS2__
    if (needtoscanusernet())
      return(scanusernet());
  #else
    if (timerexpired(7)) {
      settimer(7,Control.NetTimer);
      return(scanusernet());
    }
  #endif
  return(FALSE);
}
#endif


static void _NEAR_ LIBENTRY printdots(char *Str) {
  for (; *Str; Str++)
    printnow(*Str == ' ' ? " " : ".");
}


#pragma warn -rvl
static inpttype _NEAR_ LIBENTRY inputter(char *Buffer, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum, char *Mask, bool ShowQuestMark) {
  bool UpCase;
  bool Keyed;
  bool Insert;
  bool OldKbdFlag;
  bool Auto;
  bool YesNo;
  bool NotBlank;
  #ifndef LIB
  bool Scan = FALSE;
  #endif
  #ifdef PCBCOMM
  onlinetype SaveOnline;
  #endif
  char Str[2];
  int  FieldLen;
  int  HighAscii;
  int  EchoDots;
  int  Stacked;
  int  BeforeWrapLen;
  int  Num;
  int  Len;
  int  Key;
  int  StrLen;
  int  Wrap;
  char *p;
  char Save[81];
  char Filter[256];

  if (Display.AbortPrintout)  /* catch any stray aborts */
    checkdisplaystatus();

  #ifndef LIB
    /* disable the show activity if it is currently active */
    showactivity(ACTSUSPEND);
  #endif

  /* are we in terse mode?  If so, are we in local mode?  If so this is a */
  /* special case where we were doing terse mode output but for whatever  */
  /* reason we now need to get some input (such as displaying a text file */
  /* for a built-in prompt and then needing to get a MORE? prompt answer) */

  #ifdef PCBCOMM
    SaveOnline = Asy.Online;
    if (Status.TerseMode && Asy.Online == LOCAL)
      Asy.Online = REMOTE;
  #endif

  if ((DisplayCtrl & YESNO) == YESNO) {
    /* YESNO is made up of WORDWRAP|ECHODOTS, remove it now so it */
    /* doesn't get used below force UPCASE if YESNO is used */
    DisplayCtrl &= ~YESNO;  /*lint !e64 special case for masking the enum'd value */
    YesNo  = TRUE;
    UpCase = TRUE;
  } else {
    YesNo  = FALSE;
    UpCase = (bool) (DisplayCtrl & UPCASE);
  }

  Wrap      = DisplayCtrl & WORDWRAP;
  EchoDots  = DisplayCtrl & ECHODOTS;
  Stacked   = DisplayCtrl & STACKED;
  HighAscii = (DisplayCtrl & HIGHASCII) && PcbData.DisableFilter;
  FieldLen  = (DisplayCtrl & FIELDLEN) && UseAnsi;

  #ifndef LIB
    if (HelpNum == NOHELP)
      HelpNum = SaveHelpNum;
  #endif

  makefilter(Filter,Mask,UpCase,(bool) (HelpNum != NOHELP),(bool) Stacked,(bool) FieldLen,(bool) HighAscii,YesNo);
  Display.NumLinesPrinted = 0;

  /* The keyboard clock may have been off during a file transfer or a  */
  /* while displaying something in non-stop mode.  Force it back on    */
  /* now and then restore it when finished inputting - this will avoid */
  /* "dead time" during an input routine and catch the sleepers!       */

  OldKbdFlag = Control.WatchKbdClock;
  turnkbdtimeron();

  if (Wrap) {
    MaxLen++;                     /* add one more for the WRAP area   */
    if (Buffer[0] == 0)           /* if our buffer is empty then copy */
      strcpy(Buffer,OverFlow);    /* the OverFlow buffer into it      */
  }
  OverFlow[0] = 0;

  Insert = FALSE;
  Len    = 0;
  Str[1] = 0;
  p      = Buffer;
  StrLen = strlen(p);
  NotBlank = (bool) (StrLen != 0);
  maxstrcpy(Save,Buffer,sizeof(Save));

  Keyed = (bool) ((DisplayCtrl & NOCLEAR) || MaxLen == 1 || empty(Buffer));

  if (ShowQuestMark) {
    print("?");

    /* Are field delimiters enabled? */
    if (FieldLen) {
      BeforeWrapLen = 79 - 3 - awherex();
      /* might the field need to wrap to the next line? */
      if (BeforeWrapLen < MaxLen) {
        if (BeforeWrapLen > MaxLen / 2) {
          /* more than half of the field length will be on the first line so */
          /* just ignore the problem */
        } else if (awherex() < 70) {
          /* if it's just a long field and we're not really that close to */
          /* the edge yet, then just turn off the field length delimiters */
          FieldLen = FALSE;
        } else {
          /* otherwise, we must be close to the edge, so go down to the next */
          /* line and leave the field length delimiters enabled */
          newline();
        }
      }
    }

    if (FieldLen) {
      print(" (");
      if (awherex() + 1 + MaxLen > 79) { /* don't let field spill onto next line */
        MaxLen = 79 - 1 - awherex();
        if (MaxLen < 1) {                /* if MaxLen is less than 1 then let's just get outta here! */
          #ifndef LIB
            /* enable the show activity if it was active */
            showactivity(ACTRESUME);
          #endif
          return(INPTDONE);
        }
        if (StrLen > MaxLen) {
          Buffer[MaxLen] = 0;            /* don't let buffer get to large either */
          StrLen = MaxLen;
        }
      }
      forward(MaxLen);
      print(")");
      backup(MaxLen+1);
      printdefcolor();
      if (StrLen != 0) {
        print(p);
        if (Wrap) {
          p = &Buffer[StrLen];
          Len += StrLen;
          Keyed = TRUE;        /* don't let the first key wipe it out! */
        } else backup(StrLen);
      }
      padstr(Buffer,' ',MaxLen);
    } else {
      print(" ");
      printdefcolor();
      if (Wrap) {
        print(p);
        p = &Buffer[StrLen];
        Len += StrLen;
      } else *p = 0;
      padstr(Buffer,0,MaxLen);
    }
  } else {
    printdefcolor();
    if (Wrap) {
      print(p);
      p = &Buffer[StrLen];
      Len += StrLen;
    } else *p = 0;
    padstr(Buffer,0,MaxLen);
  }

  #ifndef LIB
    showstatusline();
  #endif

  updatelinesnow();

  if (DisplayCtrl & AUTO) {
    Auto = TRUE;
    #ifdef COMM
      waitforempty(TWENTYSECONDS);
    #endif
    settimer(4,TWENTYSECONDS);
    #ifdef __OS2__
      releasekbdblock(TWENTYSECONDS);  // release the kbd block in 20 seconds
    #endif
  } else
    Auto = FALSE;

  while (1) {
    #ifdef __OS2__
      Key = waitforkey();
      if (Key == -1 || (Key == 0 && Status.KbdTimedOut))
        Key = CTRL_M;
    #else
      #ifdef COMM
        /* if data is waiting to be processed then avoid sending any more */
        /* data out the comm port while we process what we've already got */
        if (Asy.Online == REMOTE && InBytes != 0)
          commpause();

        /* if we're still sending data out the comm port then check to     */
        /* see if the input is a CTRL-S or CTRL-X/CTRL-K sequence          */
        /* - or -                                                          */
        /* if in terse mode and pcbcomm requests the prompt be redisplayed */
        #ifdef PCBCOMM
        if ((Asy.Online == REMOTE && OutBytes != 0 && checkcomm() != 0) || (Status.TerseMode && Redisplay)) {
        #else
        if (Asy.Online == REMOTE && OutBytes != 0 && checkcomm() != 0) {
        #endif
          checkstatus();                    /* take care of control codes */
          if (Display.AbortPrintout || Redisplay) { /* was it aborted?    */
            Redisplay = FALSE;
            #ifdef PCBCOMM
              Asy.Online = SaveOnline;      /* restore online setting     */
            #endif
            Display.AbortPrintout = FALSE;  /* reset it and then          */
            freshline();                    /* start on a fresh line then */
            return(INPTREDO);               /* re-display the prompt      */
          }
        }

        /* if we've lost carrier, force it to jump out now */
        if ((Key = commportinkey()) == -1) {
          Key = CTRL_M;
          goto preprocess;
        }

        if (Key != 0)
          goto preprocess;
      #endif

      /* if keyboard timer expired, force it to jump out now */
      if ((Key = kbdinkey()) == -1)
        Key = CTRL_M;
    #endif  /* ifdef __OS2__ */

preprocess:
    if (Key >= EXTLOWKEY && Key <= EXTHIGHKEY)
      Key = ExtKeyXlat[Key-EXTLOWKEY];

    #ifdef COMM
      #ifndef LIB
        if (Key == 141 && Status.DisplayName[0] == 0) {
          Key = K_RET;
          Asy.DataBits = 7;
        }
      #endif
    #endif

    if (Key < 256) {
      Key = Filter[Key];

process:
      switch (Key) {
        case       0:
                      if (Len == 0) {
                        if (Auto && timerexpired(4)) {
                          Key = CTRL_M;                 /* force a CAR RET   */
                          renewkbdtimer();
                          goto process;
                        }
                        #ifndef LIB
                          if (checkfornodechat()) {
                            // The Scan variable is used to cause the inputter
                            // to loop back without doing anything else, if it
                            // is set to true it loops, if set to false it
                            // returns back to the calling function.  In this
                            // case, if the caller is at the command prompt and
                            // he has mail, then we need to get back to the
                            // command prompt so that the caller can scan for
                            // and read his mail.
                            if (Status.CmdPrompt && Status.CheckForMail)
                              Scan = FALSE;
                            else
                              Scan = TRUE;
                            Key = CTRL_M;                    // cmd prompt, if at cmd
                            goto process;                    // prompt re-ask quest?
                          }
                        #endif
                        if (Control.WarnMinute != 0) {
                          warntime();
                          #ifndef LIB
                            Scan = TRUE;
                          #endif
                          Key = CTRL_M;
                          goto process;
                        }
                      }
                      break;
        case CTRL_A : Len = wordbackward(Buffer,Len);
        /* c-left */  p = &Buffer[Len];
                      Keyed = TRUE;
                      break;
        case CTRL_B : /* do nothing */ break;
        case CTRL_C : /* do nothing */ break;
        case CTRL_D : if (Len < MaxLen-1) {
        /* right */     printnow("[C");
                        Len++;
                        p++;
                        Keyed = TRUE;
                      }
                      break;
        case CTRL_E : /* do nothing */ break;
        case CTRL_F : Len = wordforward(Buffer,Len,MaxLen);
        /* c-right */ p = &Buffer[Len];
                      Keyed = TRUE;
                      break;
        case CTRL_G : Num = lastcharinstr(Buffer,' ');
        /* del */     if (Len < Num) {
                        memcpy(p,p+1,Num-Len);
                        Buffer[Num-1] = ' ';
                        Buffer[Num] = 0;
                        if (EchoDots)
                          printdots(p);
                        else
                          print(p);
                        backup(Num-Len);
                        updatelinesnow();
                        padstr(Buffer,(char) (FieldLen ? ' ' : 0),MaxLen);
                        Keyed = TRUE;
                        break;
                      }
                      /* fall thru to CTRL_H if we're at the end */
        case CTRL_H : if (Len > 0) {
        /* bkspce */    Num = lastcharinstr(Buffer,' ');
                        Len--;
                        p--;
                        if (Len >= Num) {
                          backupdestructive(1);
                          *p = (char) (FieldLen ? ' ' : 0);
                        } else {
                          printnow("");
                          Key = CTRL_G;
                          goto process;
                        }
                        Keyed = TRUE;
                      }
                      break;
        case CTRL_I : /* calculate the position of the LAST tab */
        /* tab */     Num = (MaxLen >> 3) << 3;
                      /* is the cursor to the left of the last tab? */
                      if (Len < Num) {
                        Num = 8 - (Len % 8);  /* calc num to move forward */
                        forward(Num);
                        Len += Num;
                        p += Num;
                        Keyed = TRUE;
                      }
                      break;
        case CTRL_J : /* do nothing */ break;
        case CTRL_K : Num = MaxLen - Len;
                      memset(p,' ',Num);  /* print spaces over */
                      p[Num] = 0;         /* top of existing   */
                      print(p);           /* letters only then */
                      backup(Num);        /* set the 0 back to */
                      p[Num] = ' ';       /* a space           */
                      p[Num] = 0;         /* not too long...   */
                      updatelinesnow();
                      Keyed = TRUE;
                      break;
        case CTRL_L : /* do nothing */ break;
        case CTRL_M : stripright(Buffer,' ');
        /* ret */     /* Check to see if the original input was NOT blank, */
                      /* and if the buffer is NOW blank, but nothing was   */
                      /* entered by the user.  If that is the case, then   */
                      /* restore the buffer.  Otherwise, go ahead and let  */
                      /* it be cleared.  These steps are needed for the    */
                      /* CTTY support which does not allow direct editing  */
                      /* of the input field.                               */
                      if (NotBlank && Len == 0 && strlen(Buffer) == 0 && ! Keyed) {
                        strcpy(Buffer,Save);
                        stripright(Buffer,' ');
                      }
                      if (UpCase)
                        strupr(Buffer);
                      if (DisplayCtrl & ERASELINE)
                        backupcleareol(awherex());
                      if (DisplayCtrl & NEWLINE)
                        newline();
                      if (DisplayCtrl & LFAFTER)
                        newline();
                      Control.WatchKbdClock = OldKbdFlag;
                      #ifdef PCBCOMM
                        Asy.Online = SaveOnline;  /* restore online setting */
                      #endif
                      #ifndef LIB
                        LastAcceptedAnswer = Buffer;
                      #endif
                      if (HelpNum != NOHELP && Buffer[1] == 0 && (Buffer[0] == 'H' || Buffer[0] == '?'))
                        return(INPTHELP);
                      #ifndef LIB
                        if (! Scan) {
                          showactivity(ACTRESUME);
                          return(INPTDONE);
                        }
                        return(INPTREDO);
                      #else
                        return(INPTDONE);
                      #endif
        case CTRL_N : /* do nothing */ break;
        case CTRL_O : /* do nothing */ break;
        case CTRL_P : for (p = &Buffer[MaxLen-1], Num = MaxLen-1; *p == ' ' && Num; p--, Num--);
        /* end */     if (Num != 0) {
                        Num++;
                        p++;
                      }
                      if (Num > Len)
                        forward(Num-Len);
                      else if (Num < Len)
                        backup(Len-Num);
                      Len = Num;
                      Keyed = TRUE;
                      break;
        case CTRL_Q : /* do nothing */ break;
        case CTRL_R : /* do nothing */ break;
        case CTRL_S : if (Len > 0) {
        /* left */      printnow("[D");
                        Len--;
                        p--;
                        Keyed = TRUE;
                      }
                      break;
        case CTRL_T : /* do nothing */ break;
        case CTRL_U : if (Len > 0 || (FieldLen && ! empty(Buffer))) {
                        if (FieldLen && Len < (StrLen = lastcharinstr(Buffer,' '))) {
                          forward(StrLen-Len);
                          Len = StrLen;
                        }
                        if (awherex() > Len) {
                          backupdestructive(Len);
                          memset(Buffer,(FieldLen ? ' ' : 0),Len);
                          p = Buffer;
                          Len = 0;
                        } else {
                          backupcleareol(awherex());
                          Buffer[0] = 0;
                          return(INPTREDO);
                        }
                      }
                      break;
        case CTRL_V : Insert = (bool) (! Insert);
                      break;
        case CTRL_W : if (Len > 0) {
        /* home */      backup(Len);
                        Len = 0;
                        p = Buffer;
                        Keyed = TRUE;
                      }
                      break;
        case K_SPACE:
        case K_SCLN : if (Stacked) {       /* can we stack commands? */
                        if (Len == 0)
                          break;
                      }                    /* else, fall thru */
        default     : if (Len < MaxLen) {
                        if (FieldLen && ! Keyed) {
                          Keyed = TRUE;
                          memset(Buffer,' ',StrLen);  /* print spaces over */
                          Buffer[StrLen] = 0;         /* top of existing   */
                          print(Buffer);              /* letters only then */
                          backup(StrLen);             /* set the 0 back to */
                          Buffer[StrLen] = ' ';       /* a space           */
                          Buffer[MaxLen] = 0;         /* not too long...   */
                          updatelinesnow();
                        }

                        Num = lastcharinstr(Buffer,' ');
                        if (Num > Len && Insert) {
                          if (Num >= MaxLen)
                            Num = MaxLen-1;
                          memmove(&Buffer[Len+1],&Buffer[Len],Num-Len+1);
                          Buffer[Num+1]  = 0;
                          Buffer[Len]    = (char) Key;
                          Buffer[MaxLen] = 0;
                          print(&Buffer[Len]);
                          backup(strlen(&Buffer[Len]));
                          updatelinesnow();
                          Buffer[Num+1] = ' ';
                          Buffer[MaxLen] = 0;
                        } else
                          *p = (char) Key;

                        if (Wrap && Len == MaxLen-1) {
                          p = strrchr(Buffer,' ');
                          if (p != NULL) {
                            if (p != &Buffer[Len]) {
                              *p = 0;
                              strcpy(OverFlow,p+1);
                              Num = Len-((int) (p-Buffer+1));
                              backup(Num);
                              memset(p,' ',Num+1);
                              *(p+Num+1) = 0;
                              printnow(p);
                            }
                          } else {
                            OverFlow[0] = Buffer[Len];
                            OverFlow[1] = 0;
                            Buffer[Len] = 0;
                            /*
                            backup(1);
                            Buffer[Len]   = ' ';
                            Buffer[Len+1] = 0;
                            printnow(&Buffer[Len]);
                            */
                          }
                          Key = K_RET;
                          goto process;
                        }
                        p++;
                        Len++;
                        if (EchoDots)
                          Key = '.';
                      } else break;
                      Str[0] = (char) Key;
                      printnow(Str);
                      break;
      }
    }
  }
}
#pragma warn +rvl


static void _NEAR_ LIBENTRY printold(char *Old, int MaxLen, DISPLAYTYPE DisplayCtrl) {
  char Str[80];

  Str[0] = '(';
  Str[MaxLen+1] = ')';
  Str[MaxLen+2] = 0;

  if ((DisplayCtrl & YESNO) == YESNO)
    DisplayCtrl &= ~YESNO;  /*lint !e64 special case for masking the enum'd value */

  if (Old[0] == 0 || (DisplayCtrl & ECHODOTS))
    memset(&Str[1],'-',MaxLen);
  else {
    memset(&Str[1],' ',MaxLen);
    memcpy(&Str[1],Old,strlen(Old));
  }
  println(Str);
}


void LIBENTRY inputfield(char *Buffer, int PcbTextNum, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum, char *Mask) {
  bool ShowOnScreen;
  bool ShowQuestMark;
  #ifndef LIB
  bool TypeAhead;
  #endif

  #ifdef COMM
/*  watchstatuswhilesending(); */
  #endif

  if ((ShowOnScreen = Display.ShowOnScreen) == FALSE)
    Display.ShowOnScreen = TRUE;

  if (Display.AbortPrintout)  /* catch any stray aborts */
    checkdisplaystatus();

  #ifndef LIB
  setcapture(FALSE);
  #endif

  Display.NumLinesPrinted = 0;

  #ifndef LIB
    checkfornodechat();      /*lint !e534 */
  #endif

  if (Control.WarnMinute != 0)
    warntime();

  ShowQuestMark = TRUE;

  while (1) {
    if (DisplayCtrl & LFBEFORE)
      newline();

    #ifdef PCBCOMM
    if (! UseAnsi && (DisplayCtrl & GUIDE) && ! Status.TerseMode) {
    #else
    if (! UseAnsi && (DisplayCtrl & GUIDE)) {
    #endif
      if (! pcbtextspaces(PcbTextNum))         /* don't print if stop char   */
        printold(Buffer,MaxLen,DisplayCtrl);   /* is found in pcbtext record */
    }

    #ifndef LIB
      TypeAhead = ppltypeahead();    /* do we have KBD input BEFORE       */
    #endif                           /* displaying the pcbtext record and */
                                     /* possibly running a .PPE file?     */

    #ifndef LIB
      LastDefaultAnswer = Buffer;
      SaveHelpNum = HelpNum;
    #endif

    Status.WatchForStopChar = TRUE;
    if (displaypcbtext(PcbTextNum,DEFAULTS)) { /* display text, check for stop character */
      DisplayCtrl &= ~FIELDLEN;  /*lint !e64 special case for masking the enum'd value */
      ShowQuestMark = FALSE;
    }
    Status.WatchForStopChar = FALSE;

    #ifndef LIB
      SaveHelpNum = NOHELP;
      if (ppltypeahead() && ! TypeAhead) { // ONLY IF: we did NOT have KBD input
        gettypeaheadstr(Buffer,MaxLen);    // up above and we DO NOW have input
        setcapture(TRUE);
        Display.ShowOnScreen = ShowOnScreen;
        return;
      }
    #endif

    switch(inputter(Buffer,MaxLen,DisplayCtrl,HelpNum,Mask,ShowQuestMark)) {
      case INPTDONE:
                     #ifndef LIB
                       setcapture(TRUE);
                     #endif
                     Display.ShowOnScreen = ShowOnScreen;
                     return;
      case INPTHELP: displayhelpfile(HelpNum);  /*lint !e534 */
                     Buffer[0] = 0;
                     break;
      case INPTREDO: break;
    }
  }

}


void LIBENTRY inputfieldstr(char *Buffer, char *Prompt, int Color, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum, char *Mask) {
  bool ShowOnScreen;
  bool StopChar;
  int  Len;
  char Str[256];

  #ifdef COMM
/*  watchstatuswhilesending(); */
  #endif

  if ((ShowOnScreen = Display.ShowOnScreen) == FALSE)
    Display.ShowOnScreen = TRUE;

  if (Display.AbortPrintout)  /* catch any stray aborts */
    checkdisplaystatus();

  #ifndef LIB
    setcapture(FALSE);
  #endif

  Display.NumLinesPrinted = 0;

  #ifndef LIB
    checkfornodechat();   /*lint !e534 */
  #endif

  if (Control.WarnMinute != 0)
    warntime();

  Len = strlen(Prompt);
  StopChar = FALSE;
  if (Len > 0 && Prompt[--Len] == TXT_STOPCHAR) {
    Prompt[Len] = 0;
    StopChar = TRUE;
    DisplayCtrl &= ~(FIELDLEN|GUIDE);  /*lint !e64 special case for masking the enum'd value */
  }

  while (1) {
    if (DisplayCtrl & LFBEFORE)
      newline();

    #if defined(COMM) && defined(PCBCOMM)
    if (Status.TerseMode && (DisplayCtrl & GUIDE))
      sendstr("TXT0",6);
    else
    #endif
    if (! UseAnsi && (DisplayCtrl & GUIDE)) {
      Len = strlenminuscolorcodes(Prompt) + 1;
      memset(Str,' ',Len);
      Str[Len] = 0;
      print(Str);
      if (Len + MaxLen + 1 > 78)
        MaxLen = 78 - Len - 1;
      printold(Buffer,MaxLen,DisplayCtrl);
    }

    printcolor(Color);

    #ifndef LIB
      if (Prompt[0] == '!' && Prompt[1] != '|') {
        if (! runscriptwithparams(&Prompt[1]))
          printxlated(Prompt);
      } else
        printxlated(Prompt);
    #else
      printxlated(Prompt);
    #endif

    if (StopChar)
      Prompt[Len] = TXT_STOPCHAR;

    #ifndef LIB
      LastDefaultAnswer = Buffer;
    #endif

    switch(inputter(Buffer,MaxLen,DisplayCtrl,HelpNum,Mask,(bool) !StopChar)) {
      case INPTDONE:
                     #ifndef LIB
                       setcapture(TRUE);
                     #endif
                     Display.ShowOnScreen = ShowOnScreen;
                     return;
      case INPTHELP:
                     #ifndef LIB
                       if (HelpNum == NOHELP)
                         HelpNum = SaveHelpNum;
                     #endif
                     displayhelpfile(HelpNum);  /*lint !e534 */
                     Buffer[0] = 0;
                     break;
      case INPTREDO: break;
    }
  }
}

#ifdef LIB
extern char mask_num[];
long LIBENTRY inputfieldlong(long Default, char *Prompt, int Color, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum) {
  char Str[80];

  lascii(Str,Default);
  inputfieldstr(Str,Prompt,Color,MaxLen,DisplayCtrl,HelpNum,mask_num);
  return(atol(Str));
}

int LIBENTRY inputfieldint(int Default, char *Prompt, int Color, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum) {
  return((int) inputfieldlong(Default,Prompt,Color,MaxLen,DisplayCtrl,HelpNum));
}
#endif


#ifndef LIB
inputtype LIBENTRY inputreqfield(char *Buffer, int PcbTextNum, int MaxLen, DISPLAYTYPE DisplayCtrl, int HelpNum, char *Mask) {
  int NumTries;

  if (Display.AbortPrintout)  /* catch any stray aborts */
    checkdisplaystatus();

  NumTries = 0;
  while (1) {
    NumTries++;
    if (NumTries > 3)
      return(INPUTREFUSEANSWER);

    Buffer[0] = 0;
    inputfield(Buffer,PcbTextNum,MaxLen,DisplayCtrl|FIELDLEN|GUIDE|NEWLINE|LFBEFORE,HelpNum,Mask);
    if (Buffer[0] == 0)
      displaypcbtext(TXT_RESPONSEREQUIRED,NEWLINE);
    else break;
  }
  return(INPUTOKAY);
}
#endif
