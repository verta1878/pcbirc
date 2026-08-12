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


#include <malloc.h>

#include "project.h"
#pragma hdrstop

#include "pcbmacro.h"
#include "messages.h"
#include <account.h>

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <io.h>
  #include <malloc.h>
  #include <borland.h>
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#ifndef __OS2__
  #define MINFREEMEM  6144+sizeof(msgheadertype)
#endif

#define MAXLINELEN    82  /* 74 */ /* + NULL + 1 EXTRA */
#define SWITCH        31

typedef enum { STAY, NEXTLINE, PREVLINE, QUOTE, SHOWSCRN, REPAINT, BREAKOUT, LOSTCD } rettype;
typedef enum { ABSOLUTE, RELATIVE } movetype;
typedef enum { NOCLEANUP, CLEANUP } cleanuptype;
typedef enum { FROMCALLER, FROMPPL } postedtype;

#ifdef __cplusplus
  #define max(a,b)    (((a) > (b)) ? (a) : (b))
  #define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

typedef struct {
  char Line[MAXLINELEN];
} msgbuffer;

typedef struct {
  int  MaxFieldLen;
  int  RightMargin;
  int  WrapPoint;
  int  LastTab;
  int  LeftMargin;
  bool ShowLineNums;
} settings;

settings static Settings;
settings static Narrow   = { 73, 72, 71, 63, 5, TRUE  };
settings static Wide     = { 79, 79, 78, 71, 0, FALSE };

char CharXlat[256];
char static FullScreenXlat[32] = {
/* 0 */  0,
/* A */  CTRL_A,   /* LEFT */
/* B */  CTRL_B,
/* C */  CTRL_C,   /* PGDN */
/* D */  CTRL_D,   /* RIGHT */
/* E */  CTRL_E,   /* UP */
/* F */  CTRL_F,   /* CTRL-RIGHT*/
/* G */  CTRL_G,   /* DEL */
/* H */  CTRL_H,   /* BKSPC */
/* I */  CTRL_I,   /* TAB */
/* J */  CTRL_J,
/* K */  CTRL_K,   /* CTRL-END */
/* L */  CTRL_L,
/* M */  CTRL_M,   /* RET */
/* N */  CTRL_N,
/* O */  CTRL_O,
/* P */  CTRL_P,   /* END */
/* Q */  CTRL_Q,
/* R */  CTRL_R,   /* PGUP */
/* S */  CTRL_S,   /* LEFT */
/* T */  CTRL_T,
/* U */  CTRL_U,   /* ESC */
/* V */  CTRL_V,   /* INS */
/* W */  CTRL_W,   /* HOME */
/* X */  CTRL_X,   /* DOWN */
/* Y */  CTRL_Y,
/* Z */  CTRL_Z,
/*ESC*/  CTRL_U,
/* 28*/  0,
/* 29*/  0,
/* 30*/  0,
/* 31*/  SWITCH};

char static LineEditorXlat[32] = {
/* 0 */  0,
/* A */  CTRL_A,
/* B */  0,
/* C */  0,
/* D */  CTRL_D,
/* E */  0,
/* F */  CTRL_F,
/* G */  CTRL_G,
/* H */  CTRL_H,
/* I */  CTRL_I,
/* J */  0,
/* K */  CTRL_K,
/* L */  0,
/* M */  CTRL_M,
/* N */  0,
/* O */  0,
/* P */  CTRL_P,
/* Q */  0,
/* R */  0,
/* S */  CTRL_S,
/* T */  CTRL_T,
/* U */  0,
/* V */  0,
/* W */  CTRL_W,
/* X */  0,
/* Y */  0,
/* Z */  0,
/*ESC*/  0,
/* 28*/  0,
/* 29*/  0,
/* 30*/  0,
/* 31*/  0};


bool      static Insert;       /* TRUE if we're in insert mode */
bool      static EditMsg;      /* TRUE if we're editing an EXISTING message */
bool      static EditHdr;      /* TRUE if we're editing a message header    */
bool      static HideSaveMsg;  /* TRUE if "saving msg #" should be hidden   */
bool      static MsgGrew;      /* TRUE if an edited message GREW in size    */
char      static QuoteStage;   /* Current Quoting stage */
char      static OldNumBlocks; /* Number of blocks in EXISTING message */
long      static OldMsgNumber; /* Number of the EXISTING message */
long      static OldOffset;    /* Offset of the EXISTING message */
long      static ToRecNum;     /* record number of user message is addressed to */
int       static Pos;          /* column position in the line the editor is on */
int       static LineNum;      /* line number the editor is currently on (base 0) */
int       static HighNum;      /* total number of lines in the line editor (starts with 1) */
int       static MsgLeftNum;   /* PcbText Number for Message/Comment Left */
int       static MsgSaveNum;   /* PcbText Number for Saving Message/Comment */
int       static TopLine;      /* top line shown on screen */
int       static BottomLine;   /* bottom line shown on screen */
int       static PageLen;      /* length of full screen editor workspace */
int       static MaxMsgLines;  /* maximum number of message lines in workspace */
long      static MovedNum;     /* the old message number of a message being moved */
postedtype static PostedBy;

msgbuffer     static *Msg = NULL;    /* dynamically alloc array for msg body */
msgheadertype static *Header = NULL; /* dynamically alloc msg header */

char *SysopName = "SYSOP                    ";

extern struct ffblk DTA;  /* declared in exist.c */


/********************************************************************
*
*  Function:  wsmovecursor()
*
*  Desc    :  Uses Ansi cursor position commands to place the cursor at the
*             appropriate location for the X,Y coordinates in the workspace
*/

static void _NEAR_ LIBENTRY wsmovecursor(int X, int Y, movetype Move) {
  checkstack();
  if (Move == RELATIVE) {
    Y += 3 - TopLine;
    X += Settings.LeftMargin + 1;
  }
  movecursor(X-1,Y-1);
  #ifdef __OS2__
    updatelinesnow();
  #endif
}


void LIBENTRY movebottom(void) {
  wsmovecursor(Settings.LeftMargin,PageLen+4,ABSOLUTE);
}


/********************************************************************
*
*  Function:  lastspace()
*
*  Desc    :  scans the string for the last space PRIOR to LastColumn
*
*  Returns :  returns a pointer to the space
*/

static char * _NEAR_ LIBENTRY lastspace(char *Str, int LastColumn) {
  int X;

  checkstack();
  for (X = LastColumn, Str += LastColumn; *Str != ' ' && X; X--, Str--);
  if (X == 0)
    return(NULL);
  return(Str);
}


/********************************************************************
*
*  Function:  lastline()
*
*  Desc    :  scans the message editor workspace for the last line with data
*
*  Returns :  the number of the last line (starting the first line as "1")
*/

static int _NEAR_ LIBENTRY lastline(void) {
  int       X;
  msgbuffer *p;

  checkstack();
  for (X = MaxMsgLines-1, p = &Msg[X]; p->Line[0] == 0 && X; p--, X--);
  return(X+1);
}


/********************************************************************
*
*  Function:  longlines()
*
*  Desc    :  Determines if any existing lines are longer than 72 characters
*
*  Returns :  TRUE if long lines are found, otherwise FALSE
*/

static bool _NEAR_ LIBENTRY longlines(void) {
  int       Num;
  msgbuffer *p;

  checkstack();
  for (Num = 0, p = Msg, HighNum = lastline(); Num < HighNum; Num++, p++) {
    if (strlen(p->Line) > 72)
      return(TRUE);
  }
  return(FALSE);
}


/********************************************************************
*
*  Function:  insertline()
*
*  Desc    :  Inserts a blank line at line number Num moving all subsequent
*             lines down one...
*
*  Returns :  TRUE if a line was inserted, FALSE otherwise
*/

static bool _NEAR_ LIBENTRY insertline(int Num) {
  checkstack();
  HighNum = lastline();
  if (HighNum < Num)
    return(FALSE);

  if (HighNum >= MaxMsgLines)
    return(FALSE);

  memmove(&Msg[Num+1],&Msg[Num],(HighNum-Num)*sizeof(msgbuffer));
  Msg[Num].Line[0] = 0;
  HighNum++;
  return(TRUE);
}


/********************************************************************
*
*  Function:  deleteline()
*
*  Desc    :  Deletes a line at line number Num moving all subsequent lines
*             up one...
*
*  Returns :  TRUE if a line was deleted, FALSE otherwise
*/

static bool _NEAR_ LIBENTRY deleteline(int Num) {
  checkstack();
  HighNum = lastline();
  if (HighNum >= Num) {
    if (HighNum-Num > 1) {
      memmove(&Msg[Num],&Msg[Num+1],(HighNum-Num-1)*sizeof(msgbuffer));
      memset(Msg[HighNum-1].Line,0,sizeof(msgbuffer));
    } else memset(Msg[Num].Line,0,sizeof(msgbuffer));
    if (HighNum > 0)
      HighNum--;
    return(TRUE);
  }
  return(FALSE);
}


/********************************************************************
*
*  Function:  wordwrap()
*
*  Desc    :  reformats the information in the message buffer beginning
*             with LineNum (the current line number)
*
*  Returns :  1) -1 if there was no room to (too many lines) to insert another
*             2) The line number of the last line that was reformatted
*             3) 0 if no lines were reformatted
*/

static int _NEAR_ LIBENTRY wordwrap(void) {
  int       Num;
  int       Last;
  int       LastLine;
  char      *Str;
  char      *Space;
  msgbuffer *p;
  char      OverFlow[256];

  checkstack();
  OverFlow[0] = 0;
  LastLine    = 0;
  HighNum     = lastline();

  if (HighNum >= MaxMsgLines)
    return(-1);

  for (Num = LineNum, p = &Msg[Num]; Num < HighNum || OverFlow[0] != 0; p++, Num++) {
    if (OverFlow[0] != 0) {
      if (Num >= HighNum) {
        HighNum = lastline();
        if (HighNum >= MaxMsgLines)
          break;
      }
      if (p->Line[0] == 0) {
        insertline(Num);      /*lint !e534 */
        LastLine = HighNum;
      }
      strcat(OverFlow,p->Line);
      Str = OverFlow;
    } else Str = p->Line;

    if ((Last = lastcharinstr(Str,' ')) > Settings.RightMargin) {
      if (Num > LastLine)
        LastLine = Num;
      Str[Last] = 0;
      if ((Space = lastspace(Str,Settings.WrapPoint)) != NULL) {
        *Space = 0;
        if (OverFlow[0] != 0) {
          if (Status.TaggedText != NULL) {
            buildstr(p->Line,"-> ",OverFlow,NULL);
          } else {
            maxstrcpy(p->Line,OverFlow,Settings.RightMargin+1);
            stripright(p->Line,' ');
          }
        }
        Str[Last] = ' ';
        Str[Last+1] = 0;
        maxstrcpy(OverFlow,Space+1,sizeof(OverFlow));
      } else Str[Settings.RightMargin] = 0;
    } else {
      if (OverFlow[0] != 0) {
        if (Num > LastLine)
          LastLine = Num;
        if (Status.TaggedText != NULL && strlen(OverFlow) < Settings.RightMargin-3) {  /*lint !e574 */
          buildstr(p->Line,"-> ",OverFlow,NULL);
        } else {
          maxstrcpy(p->Line,OverFlow,Settings.RightMargin+1);
          stripright(p->Line,' ');
        }
      }
      break;
    }
  }
  return(LastLine);
}


static void _NEAR_ LIBENTRY removeinsidespaces(char *Str) {
  char *p;
  char Ch;

  checkstack();
  stripleft(Str,' ');
  stripright(Str,' ');

  while (1) {
    if ((p = strstr(Str,"  ")) == NULL)
      break;

    if (p != Str) {
      Ch = *(p-1);
      if (Ch == '.' || Ch == ':' || Ch == '?' || Ch == '!') {
        Str = p + 1;
        continue;
      }
    }
    strcpy(p,p+1);
    Str = p;
  }
}


/********************************************************************
*
*  Function:  reformat()
*
*  Desc    :  reformats the current paragraph starting at StartLine or if
*             StartLine = 0 it scans for the top of the paragraph
*/

static int _NEAR_ LIBENTRY reformat(int StartLine) {
  bool      Quoted;
  int       Num;
  int       TopOfParagraph;
  int       Len;
  int       Index;
  char      *Space;
  msgbuffer *p;
  msgbuffer *q;
  msgbuffer *End;

  checkstack();
  Quoted  = FALSE;
  HighNum = lastline();

  if (StartLine != 0) {
    Num = StartLine++;
    p = &Msg[Num];
  } else {
    for (Num = LineNum, p = &Msg[Num]; Num >= 0; ) {
      stripright(p->Line,' ');
      if (p->Line[0] == 0)
        break;
      p--;
      Num--;
    }
/*  for (Num = LineNum, p = dMsg[Num]; p->Line[0] != 0 && Num >= 0; p--, Num--); */
    Num++;
    p++;
  }

  TopOfParagraph = Num;
  q = p + 1;
  End = &Msg[HighNum-1];

  while (1) {
    stripright(p->Line,' ');
    removeinsidespaces(p->Line);
    Len = strlen(p->Line);
    Index = 0;
    if (Len <= Settings.RightMargin && q <= End) {
      if (q->Line[0] == 0)
        break;
      if (q->Line[0] == '-' && q->Line[1] == '>' && (q->Line[2] == ' ' || q->Line[2] == 0)) {
        Quoted = TRUE;
        Index = (q->Line[2] == ' ' ? 3 : 2);
      }
      p->Line[Len++] = ' ';
      stripleft(q->Line,' ');
      removeinsidespaces(q->Line);
      memmove((char *) p + Len,(char *) q + Index,sizeof(msgbuffer));  /* use "(char*)" to fake out the DEBUG.H routines */
      q++;
      Len = strlen(p->Line);
    }
    if (Len > Settings.RightMargin) {
      Space = lastspace(p->Line,Settings.RightMargin);
      *Space++ = 0;
      p++;
      if (Quoted && strlen(Space) + 2 < Settings.RightMargin) {  /*lint !e574 */
        memmove((char *) p + 3,Space,strlen(Space)+1);  /* use "(char*)" to fake out the DEBUG.H routines */
        memcpy((char *) p,"-> ",3);
      } else {
        memmove((char *) p + Index,Space,strlen(Space)+1);  /* use "(char*)" to fake out the DEBUG.H routines */
      }
      Len = strlen((p-1)->Line);
      memset(&(p-1)->Line[Len],0,sizeof(msgbuffer)-Len);
    } else
      memset(&p->Line[Len],0,sizeof(msgbuffer)-Len); /* pad the end with 0's */
    if (q > End)
      break;
  }

  HighNum = lastline();
  for (p++, Num = (int) (p-Msg); p < q; p++)
    deleteline(Num);   /*lint !e534 */

  return(TopOfParagraph < TopLine ? TopLine : TopOfParagraph);
}


/********************************************************************
*
*  Function:  joinline()
*
*  Desc    :  joins the next line onto the current line
*
*  Returns :  the last line number that should be redrawn or "0" if no lines
*             need to be redrawn
*/

static int _NEAR_ LIBENTRY joinline(int Position) {
  int  Len1;
  int  Len2;
  char *Space;

  checkstack();
  if (LineNum >= MaxMsgLines)
    return(0);

  HighNum = lastline();
  if ((LineNum+1)+1 > HighNum) /* (LineNum+1)=next line, "+1" for 1-based arithmetic */
    return(0);

  Len1 = lastcharinstr(Msg[LineNum].Line,' ');
  Len2 = strlen(Msg[LineNum+1].Line);

  if (Position > Len1) {
    Len1 = Position;
    Msg[LineNum].Line[Len1] = 0;
  } else {
    stripright(Msg[LineNum].Line,' ');
    Len1 = strlen(Msg[LineNum].Line);
  }

  if (Len1 + Len2 > Settings.RightMargin) {
    if ((Space = lastspace(Msg[LineNum+1].Line,Settings.RightMargin-Len1)) != NULL) {
      *Space = 0;
      strcat(Msg[LineNum].Line,Msg[LineNum+1].Line);
      strcpy(Msg[LineNum+1].Line,Space+1);
    }
    return(LineNum+2);
  } else {
    strcat(Msg[LineNum].Line,Msg[LineNum+1].Line);
    deleteline(LineNum+1);   /*lint !e534 */
  }
  padstr(Msg[LineNum].Line,' ',Settings.MaxFieldLen);
  return(BottomLine);
}


/********************************************************************
*
*  Function:  printnoesc()
*
*  Desc    :  turns off escape code processing while printing the string so as
*             to avoid turning escape codes into actual colors
*/

static void _NEAR_ LIBENTRY printnoesc(char *Str) {
  checkstack();
  noesccodes();             /* turn off ESC code processing */
  print(Str);
  doesccodes();             /* turn on  ESC code processing */
}


/********************************************************************
*
*  Function:  redraw()
*
*  Desc    :  redraws the screen from StartLine thru LastLine
*/

static void _NEAR_ LIBENTRY redraw(int StartLine, int LastLine) {
  int       Num;
  msgbuffer *p;

  checkstack();
  if (LastLine > BottomLine-1)
    LastLine = BottomLine-1;

  for (Num = StartLine, p = &Msg[Num]; Num <= LastLine; p++, Num++) {
    wsmovecursor(0,Num,RELATIVE);
    printnoesc(p->Line);
    cleareol();
  }
}


/********************************************************************
*
*  Function:  showinsertmode()
*
*  Desc    :  shows the status of Insert Mode
*/

static void _NEAR_ LIBENTRY showinsertmode(void) {
  checkstack();
  wsmovecursor(48,PageLen+4,ABSOLUTE);
  if (Insert)
    displaypcbtext(TXT_INSFOROVERWRITE,DEFAULTS);
  else
    displaypcbtext(TXT_INSFORINSERT,DEFAULTS);
  cleareol();
  printdefcolor();
}

/*
static void _NEAR_ LIBENTRY stufftaggedtext(void) {
  char *p;
  char Buffer[128];

  if ((p = strchr(Status.TagPointer,13)) != NULL) {
    *p = 0;
    maxstrcpy(Buffer,Status.TagPointer,Settings.MaxFieldLen);
    Status.TagPointer = p + 1;
  } else {
    maxstrcpy(Buffer,Status.TagPointer,Settings.MaxFieldLen);
    bfree(Status.TaggedText);
    Status.TaggedText = NULL;
  }
  stuffbufferstr(Buffer);
  if (strlen(Buffer) != Settings.MaxFieldLen)
    stuffbuffer(13);
}
*/

/********************************************************************
*
*  Function:  inputline()
*
*  Desc    :  gets a line of data from the user and word wraps into OverFlow
*             if the user goes beyond position Settings.RightMargin
*/

#pragma warn -rvl
static rettype _NEAR_ LIBENTRY inputline(char *OverFlow) {
  char static Leader[] = {0,CTRL_X,CTRL_W,CTRL_N,CTRL_E,CTRL_N,CTRL_E,0};
  int  X;
  int  Y;
  int  Key;
  char *Line;
  char *p;
  char Str[80];

  checkstack();
  Line        = Msg[LineNum].Line;
  Str[1]      = 0;
  OverFlow[0] = 0;

  startdisplay(FORCENONSTOP);  /* force non-stop mode during message entry */
  turnkbdtimeron();            /* but turn the clock back on!              */

  padstr(Line,' ',Settings.MaxFieldLen);

  while (1) {
     if (Status.TaggedText != NULL) {
       if (QuoteStage < 6) {
         if (QuoteStage == 0) {
           if (lastcharinstr(Msg[LineNum].Line,' ') == 0) {
             if (lastcharinstr(Msg[LineNum+1].Line,' ') == 0)
               QuoteStage = 6;
             else
               QuoteStage = 4;
           }
         }
         QuoteStage++;
         Key = Leader[QuoteStage];
         goto process;
       }
       Key = *Status.TagPointer;
       if (Key == 0) {
         QuoteStage = 0;
         bfree(Status.TaggedText);
         Status.TaggedText = NULL;
         Key = CTRL_M;
         goto process2;
       }
       switch (Pos) {
         case 0: Key = '-';
                 goto process2;
         case 1: Key = '>';
                 goto process2;
         case 2: Key = ' ';
                 goto process2;
         case 3: if (Key == ' ' && *(Status.TagPointer+1) != ' ') {
                   Status.TagPointer++;
                   continue;
                 }
       }

       if (Key == 27)
         Key = 29;
       Status.TagPointer++;
       goto process;
     }

     #ifdef __OS2__
       Key = waitforkey();
     #else
       #ifdef COMM
         /* if data is waiting to be processed then avoid sending any more */
         /* data out the comm port while we process what we've already got */
         if (Asy.Online == REMOTE) {
           if (! (typeahead() || ppltypeahead())) {
             if (InBytes != 0)
               commpause();

             if ((Key = commportinkey()) != 0)
               goto process;
           }
         }
       #endif  /* ifdef COMM */

       Key = kbdinkey();
     #endif  /* ifdef __OS2__ */

process:
     // a -1 is returned by commportinkey() if carrier is lost, it is also
     // retunred by kbdinkey() if a keyboard timeout occurs, in either case,
     // it's time to exit the editor
     if (Key == -1)
       return(LOSTCD);

     if (Key >= EXTLOWKEY && Key <= EXTHIGHKEY)
       Key = ExtKeyXlat[Key-EXTLOWKEY]; /* filter out undefined keystrokes */

     if (Key > 255)  /* Key should be a char with all relevant "high #'s" */
       continue;     /* "low #'s" ..  if not then we don't want it!       */

     Key = CharXlat[Key];  /* filter out unwanted characters */

process2:
     switch (Key) {
       case       0: /* if (Status.TaggedText != NULL) */
                     /*  stufftaggedtext(); */
                     continue;
       case CTRL_A : Pos = wordbackward(Line,Pos);
       /* c-left */  break;
       case CTRL_B : stripright(Line,' ');
       /* reflow */  X = reformat(0);
                     redraw(X,BottomLine);
                     if (Pos > (X = strlen(Line)))
                       Pos = X;
                     wsmovecursor(Pos,LineNum,RELATIVE);
                     padstr(Line,' ',Settings.MaxFieldLen);
                     break;
       case CTRL_C : if (TopLine < MaxMsgLines-PageLen) {
       /* pgdn */      TopLine += PageLen-2;
                       /* if ((LineNum += PageLen-2) > MaxMsgLines-2) */
                       /*   LineNum = MaxMsgLines-2;                  */
                       if ((LineNum = TopLine + 2) > MaxMsgLines)
                         LineNum = MaxMsgLines;
                     }
                     stripright(Line,' ');
                     return(SHOWSCRN);
       case CTRL_D : if (Pos < Settings.WrapPoint) {
       /* right */     printnow("[C");
                       Pos++;
                     }
                     break;
       case CTRL_E : stripright(Line,' ');
       /* up */      return(PREVLINE);
       case CTRL_F : Pos = wordforward(Line,Pos,Settings.RightMargin);
       /* c-right */ break;
       case CTRL_G : X = lastcharinstr(Line,' ');
       /* del */     if (Pos >= X && Status.FullScreen) {
                       if ((X = joinline(Pos)) != 0) {
                         redraw(LineNum,X);
                         wsmovecursor(Pos,LineNum,RELATIVE);
                         break;
                       }
                       /* else fall thru to CTRL_H */
                     } else if (X > Pos) {
                       memcpy(&Line[Pos],&Line[Pos+1],X-Pos);
                       Line[X-1] = ' ';
                       Line[X] = 0;
                       print(&Line[Pos]);
                       backup(X-Pos);
                       updatelinesnow();
                       Line[X] = ' ';
                       break;
                     }
                     /* CTRL_G falls thru to CTRL_H if the cursor was on */
                     /* the last character of the last line of the msg,  */
                     /* thus simulating older versions of PCBoard        */
       case CTRL_H : if (Pos == 0) {
       /* bkspc */     if (Status.FullScreen && LineNum > 0) {
                         stripright(Line,' ');
                         LineNum--;
                         stripright(Msg[LineNum].Line,' ');
                         Pos = strlen(Msg[LineNum].Line);
                         if ((X = joinline(Pos)) != 0) {
                           if (TopLine > LineNum && TopLine != 0) {
                             LineNum++;
                             return(PREVLINE);
                           }
                           redraw(LineNum,X);
                           wsmovecursor(Pos,LineNum,RELATIVE);
                         }
                         if (TopLine > LineNum) {
                           LineNum++;
                           return(PREVLINE);
                         }
                         return(STAY);
                       }
                     } else {
                       print("");
                       X = lastcharinstr(Line,' ');
                       if (X <= Pos) {
                         print(" ");
                         Line[Pos-1] = ' ';
                       } else {
                         Line[X++] = ' ';
                         Line[X] = 0;
                         print(&Line[Pos]);
                         backup(X-Pos);
                         memcpy(&Line[Pos-1],&Line[Pos],X-Pos);
                         Line[X] = ' ';
                       }
                       updatelinesnow();
                       Pos--;
                     }
                     break;
       case CTRL_I : if (Pos < Settings.LastTab) {
       /* tab */       X = 8 - (Pos % 8);  /* calc num to move forward */
                       if (Status.FullScreen) {
                         /* are we in INSERT mode if so are we at the END or in the middle? */
                         if (Insert && (LineNum < HighNum-1 || Pos < lastcharinstr(Line,' '))) {
                           /* we're in the middle so insert 'X' number of spaces */
                           for (; X; X--)
                             stuffbuffer(' ');
                           break;  /* then get out before Pos += X */
                         } else forward(X);
                       } else {
                         memset(Str,' ',X);
                         memset(&Line[Pos],' ',X);
                         Str[X] = 0;
                         printnow(Str);
                         Str[1] = 0;
                       }
                       Pos += X;
                     } else if (Status.FullScreen) {
                       /* are we in INSERT mode if so are we at the END or in the middle? */
                       if (Insert && (LineNum < HighNum-1 || Pos < lastcharinstr(Line,' '))) {
                         /* we're in the middle so insert 'X' number of spaces */
                         for (X = Settings.RightMargin - lastcharinstr(Line,' ') + 1; X; X--)
                           stuffbuffer(' ');
                       } else {
                         stripright(Line,' ');
                         Pos = 0;
                         return(NEXTLINE);
                       }
                     }
                     break;
       case CTRL_J : if ((X = joinline(Pos)) != 0) {
       /* join */      redraw(LineNum,X);
                       wsmovecursor(Pos,LineNum,RELATIVE);
                     }
                     break;
       case CTRL_K : Line[Pos] = 0;
       /* c-end */   cleareol();
                     padstr(Line,' ',Settings.MaxFieldLen);
                     break;
       case CTRL_L : printcls();
                     stripright(Line,' ');
       /* redraw */  return(REPAINT);
       case CTRL_M : if (Status.FullScreen) {
       /* ret */       if (Insert) {
                         if (insertline(LineNum+1)) {
                           X = lastcharinstr(Line,' ');
                           stripright(Line,' ');
                           if (X > Pos) {
                             cleareol();
                             if (LineNum < MaxMsgLines-1)
                               strcpy(Msg[LineNum+1].Line,&Line[Pos]);
                             Line[Pos] = 0;
                             stripright(Line,' ');
                           }
                           redraw(LineNum+1,min(BottomLine,HighNum));
                         }
                       } else stripright(Line,' ');
                     } else {
                       stripright(Line,' ');
                       if (Pos == 0 && Line[0] == 0)
                         return(BREAKOUT);
                     }
                     Pos = 0;
                     #ifdef __OS2__
                       updatelinesnow();
                     #endif
                     return(NEXTLINE);
       case CTRL_N : if (insertline(LineNum+1)) {
       /* newline */   X = lastcharinstr(Line,' ');
                       stripright(Line,' ');
                       if (X > Pos) {
                         cleareol();
                         strcpy(Msg[LineNum+1].Line,&Line[Pos]);
                         Line[Pos] = 0;
                         stripright(Line,' ');
                       }
                       Pos = 0;
                       redraw(LineNum+1,min(BottomLine,HighNum));
                     }
                     #ifdef __OS2__
                       updatelinesnow();
                     #endif
                     return(NEXTLINE);
       case CTRL_O : /* Pos = 0; */
       /* quote */   stripright(Line,' ');
                     return(QUOTE);
       case CTRL_P : X = lastcharinstr(Line,' ');
       /* end */     if (X > Pos)
                       forward(X-Pos);
                     else if (X < Pos)
                       backup(Pos-X);
                     Pos = X;
                     updatelinesnow();
                     break;
       case CTRL_Q : /* do nothing */ break;
       case CTRL_R : if (TopLine > PageLen-2) {
       /* pgup */      TopLine -= PageLen-2;
                       /* LineNum -= PageLen-2; */
                       LineNum = TopLine + PageLen - 3;
                     } else if (TopLine > 0) {
                       TopLine = 0;
                       LineNum = PageLen - 3;
                     } else {
                       break;
                     }
                     stripright(Line,' ');
                     return(SHOWSCRN);
       case CTRL_S : if (Pos > 0) {
       /* left */      printnow("[D");
                       Pos--;
                     }
                     break;
       case CTRL_T : padstr(Line,' ',Settings.MaxFieldLen);
       /*del word*/  X = lastcharinstr(Line,' ');
                     if (Pos >= X && Status.FullScreen) {
                       if ((X = joinline(Pos)) != 0) {
                         redraw(LineNum,X);
                         wsmovecursor(Pos,LineNum,RELATIVE);
                       }
                     } else {
                       if (X > Pos) {
                         Y = Pos;
                         if (Line[Pos] == ' ')
                           while (Line[Y] == ' ') Y++;
                         else {
                           while (Line[Y] > ' ') Y++;
                           Y++;
                         }
                         memcpy(&Line[Pos],&Line[Y],X-Pos);
                         padstr(Line,' ',Settings.MaxFieldLen);
                         Line[X] = 0;
                         print(&Line[Pos]);
                         backup(X-Pos);
                         updatelinesnow();
                         Line[X] = ' ';
                       }
                     }
                     break;
       case CTRL_U : stripright(Line,' ');
       /* esc */     return(BREAKOUT);
       case CTRL_V : Insert = (bool) (! Insert);
       /* ins */     showinsertmode();
                     wsmovecursor(Pos,LineNum,RELATIVE);
                     break;
       case CTRL_W : if (Pos > 0) {
       /* home */      backup(Pos);
                       Pos = 0;
                       updatelinesnow();
                     }
                     break;
       case CTRL_X : stripright(Line,' ');
       /* down */    return(NEXTLINE);
       case CTRL_Y : if (deleteline(LineNum))
       /*del line*/    redraw(LineNum,BottomLine);
                     Pos = 0;
                     wsmovecursor(Pos,LineNum,RELATIVE);
                     return(STAY);
       case CTRL_Z : printcls();
       /* help */    if (displayhelpfile(HLP_FULLSCRN) != -1)
                       moreprompt(PRESSENTER);
                     stripright(Line,' ');
                     return(REPAINT);
       case SWITCH : stripright(Line,' ');
                     if (Settings.ShowLineNums)
                       Settings = Wide;
                     else {
                       if (! longlines())
                         Settings = Narrow;
                     }
                     return(REPAINT);
       case K_ESC  : Key = 29;  /* fall thru */
       default     : X = lastcharinstr(Line,' ');
                     if (X > Pos && Insert) {
                       memmove(&Line[Pos+1],&Line[Pos],X-Pos+1);
                       Line[Pos] = (char) Key;
                       if (Status.FullScreen && (X = wordwrap()) > LineNum) {
                         if (strlen(Line) > Pos) {  /*lint !e574 */
                           print(&Line[Pos]);
                           cleareol();
                           padstr(Line,' ',Settings.MaxFieldLen);
                           redraw(LineNum+1,X);
                           wsmovecursor(Pos+1,LineNum,RELATIVE);
                         } else {
                           backupcleareol(Pos-strlen(Line));
                           redraw(LineNum+1,X);
                           Pos -= strlen(Line);
                           return(NEXTLINE);
                         }
                       } else {
/*                       Old code
                         Line[Settings.MaxFieldLen] = 0;
                         print(&Line[Pos]);
                         backup(strlen(&Line[Pos])-1);
                         updatelinesnow();
*/
/*                       New code as of 7/26/95 */
                         Line[Settings.MaxFieldLen] = 0;
                         stripright(&Line[Pos],' ');
                         print(&Line[Pos]);
                         cleareol();
                         backup(strlen(&Line[Pos])-1);
                         updatelinesnow();
                         padstr(Line,' ',Settings.MaxFieldLen);
                       }
                       Pos++;
                       break;  /* don't continue on to print the char */
                     } else {
                       Line[Pos] = (char) Key;
                       if (Pos > Settings.WrapPoint) {
                         if (Status.FullScreen) {
                           if ((X = wordwrap()) > LineNum) {
                             backupcleareol(Pos-strlen(Line));
                             redraw(LineNum+1,X);
                             Pos -= strlen(Line);
                             if (Status.TaggedText != NULL)
                               Pos += 3;
                           } else {
                             Line[Pos] = 0;
                             if (Key != ' ')
                               stuffbuffer(Key);
                             if (X != -1 && Insert) {
                               Key = CTRL_N;
                               goto process2;
                             }
                             Pos = 0;
                           }
                           return(NEXTLINE);
                         } else {
                           if (LineNum >= MaxMsgLines-1) {
                             Line[Pos-1] = 0;
                             Pos = 0;
                             return(NEXTLINE);
                           }
                           if ((p = strrchr(Line,' ')) != NULL && p != &Line[Pos]) {
                             *p = 0;
                             strcpy(OverFlow,p+1);
                             backupcleareol(Pos-((int) (p-Line+1)));
                           }
                           Pos = 0;
                           return(NEXTLINE);
                         }
                       }
                       Pos++;
                     }
                     Str[0] = (char) Key;
                     printnow(Str);
                     break;
     }
  }
}
#pragma warn +rvl


/********************************************************************
*
*  Function:  removetokens()
*
*  Desc    :  This function USED TO REMOVE pcboard macros (tokens), now it
*             just converts them to lowercase letters.  The effect is still
*             to disable their usage, but now, if someone writes a message with
*             the tokens in it, the original tokens can still be seen.
*
*             Affects all tokens except @X and @POS.
*
*  NOTES   :  The function findtoken() scans the string for a token and if
*             one is found it returns in the AX register the number of the
*             token.  The string passed to findtoken() becomes NULL-terminated
*             where the first @ was found.
*/

void LIBENTRY removetokens(char *Str) {
  int   Found;
  char *p;
  char *q;

  checkstack();
  while (Str[0] != 0 && (Found = findtoken(Str)) != 0) {
    p  = &Str[FindTokenStart];
    *p = '@';                      // restore the starting '@'
    q  = &Str[FindTokenEnd-1];
    if (Found != XCOLORS && Found != POS) {
      *q  = 0;                     // change the ending '@' to a 0
      strlwr(p+1);                 // convert the string to lowercase
      *q  = '@';                   // restore the ending '@'
    }
    Str = q + 1;                   // move the string pointer forward
  }
}


/********************************************************************
*
*  Function:  getlinenumber()
*
*  Desc    :  gets a line number either from a stacked request or by prompting
*             the user for the number.
*
*  Returns :  -1 if no number was requested or if it was invalid, otherwise it
*             returns line number requested (minus 1 since we start with 0)
*/

static int _NEAR_ LIBENTRY getlinenumber(int NumTokens, int TextNum) {
  int  Num;
  char Str[4];
  char *p;

  checkstack();
  if (NumTokens == 0) {
    Str[0] = 0;
    inputfield(Str,TextNum,3,NEWLINE|LFBEFORE,NOHELP,mask_numbers);
    if (Str[0] == 0)
      return(-1);
    p = Str;
  } else p = getnexttoken();

  Num = atoi(p);
  if (Num < 1 || Num > HighNum) {
    displaypcbtext(TXT_NOSUCHLINENUMBER,NEWLINE|LFBEFORE);
    return(-1);
  }
  return(Num-1);
}


static void _NEAR_ LIBENTRY printdivider(void) {
  char Divider[80];

  checkstack();
  memset(Divider,'-',79);
  if (Settings.ShowLineNums) {
    memcpy(Divider,"    (",5);
    Divider[77] = ')';
    Divider[78] = 0;
  } else Divider[79] = 0;
  println(Divider);
}


/********************************************************************
*
*  Function:  printtopline()
*
*  Desc    :  prints the TO: and SUBJ: fields followed by (---) underneath
*/

static void _NEAR_ LIBENTRY printtopline(msgheadertype *MsgHeader) {
  char        *p;
  char        Name[80];
  char        Subj[80];
  pcbtexttype ToBuf;
  pcbtexttype SubjBuf;
  pcbtexttype AllName;
  char        Str[256];

  checkstack();
  displaycmdfile("PREEDIT");

  printcolor(PCB_YELLOW);
  if (! Status.FullScreen) {
    newline();
    startdisplay(NOCHANGE);
  }

  getpcbtext(TXT_TO,&ToBuf);
  getpcbtext(TXT_SUBJ,&SubjBuf);
  stripleft(ToBuf.Str,' ');
  stripleft(SubjBuf.Str,' ');

  movestr(Name,MsgHeader->ToField,25);
  stripright(Name,' ');

  if (strcmp(Name,"ALL") == 0) {
    getpcbtext(TXT_ALLNAME,&AllName);
    strcpy(Name,AllName.Str);
  } else {
    if (Name[0] == 0 && (p = checkextendedheaders(EXTHDR_TO,NULL)) != NULL) {
      maxstrcpy(Name,p,sizeof(Name));
      Name[EXTDESCLEN] = 0;
      stripright(Name,' ');
    }
  }

  if ((p = checkextendedheaders(EXTHDR_SUBJECT,NULL)) != NULL) {
    maxstrcpy(Subj,p,sizeof(Subj));
    Subj[EXTDESCLEN] = 0;
  } else {
    memcpy(Subj,MsgHeader->SubjField,sizeof(MsgHeader->SubjField));
    Subj[sizeof(MsgHeader->SubjField)] = 0;
  }
  stripright(Subj,' ');

  sprintf(Str,"%*.*s%s%-25.25s %s%s",Settings.LeftMargin,Settings.LeftMargin,"",ToBuf.Str,Name,SubjBuf.Str,Subj);
  Str[79] = 0; /* limit the line to 79 characters */
  stripright(Str,' ');
  println(Str);
  printcolor(PCB_CYAN);
  printdivider();
}


/********************************************************************
*
*  Function:  listmsgbuffer()
*
*  Desc    :  lists the contents of the message buffer
*             optionally can begin at a specified line number if the number was
*             stacked on the request
*/

static void _NEAR_ LIBENTRY listmsgbuffer(int NumTokens) {
  bool RemoveTokens;
  int  Num;
  char Str[80];

  checkstack();
  RemoveTokens = (bool) (Status.CurSecLevel < PcbData.SysopSec[SEC_SUBS]);

  printtopline(Header);
  printdefcolor();
  Num = (NumTokens != 0 ? getlinenumber(NumTokens,0) : 0);
  if (Num == -1)
    Num = 0;

  HighNum = lastline();
  while (Num < HighNum && !Display.AbortPrintout) {
    if (Settings.ShowLineNums) {
      sprintf(Str,"%3d: ",Num+1);
      printcolor(PCB_CYAN);
      print(Str);
      printdefcolor();
    }
    if (RemoveTokens)
      removetokens(Msg[Num].Line);
    printnoesc(Msg[Num].Line);
    newline();
    Num++;
  }
  newline();
  checkdisplaystatus();
}


/********************************************************************
*
*  Function:  deletelinenum()
*
*  Desc    :  deletes a line of text from the message buffer
*             moves all text below the line up
*
*  Returns :  1 if successful, 0 if an error occurred
*/

static void _NEAR_ LIBENTRY deletelinenum(int NumTokens) {
  int  Num;
  char Str[128];

  checkstack();
  if ((Num = getlinenumber(NumTokens,TXT_DELETELINENUM)) == -1)
    return;

  printcolor(PCB_CYAN);
  printdivider();
  sprintf(Str,"%3d: ",Num+1);
  print(Str);
  printdefcolor();
  println(Msg[Num].Line);
  Str[0] = NoChar;
  Str[1] = 0;
  inputfield(Str,TXT_WANTTODELETELINE,1,YESNO|FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_yesno);
  if (Str[0] != YesChar)
    return;

  deleteline(Num);  /*lint !e534 */

  if (LineNum > HighNum)
    LineNum = HighNum;
}


/********************************************************************
*
*  Function:  editline()
*
*  Desc    :  allows the user to substitute a string of text with another or
*             to remove a string of text
*/

static void _NEAR_ LIBENTRY editline(int NumTokens) {
  int         Num;
  char        *p;
  char        *q;
  char        *Line;
  pcbtexttype Buf;
  char        Str[128];
  char        Old[128];
  char        New[256];

  checkstack();
  if ((Num = getlinenumber(NumTokens,TXT_EDITLINENUM)) < 0)
    return;

  Line = Msg[Num].Line;

  newline();
  while (1) {
    printcolor(PCB_CYAN);
    printdivider();
    sprintf(Str,"%3d: ",Num+1);
    print(Str);
    printdefcolor();
    printnoesc(Line);
    newline();
    displaypcbtext(TXT_OLDTEXTNEWTEXT,NEWLINE|LFBEFORE);
    Str[0] = 0;
    inputfieldstr(Str,"",PCB_YELLOW,sizeof(Str)-1,NEWLINE,NOHELP,mask_message);
    if (Str[0] == 0)
      break;

    strcpy(New,Line);
    if ((q = strchr(Str,';')) != NULL)
      *q = 0;
    if ((p = strstr(New,Str)) != NULL) {
      *p = 0;
      strcpy(Old,p+strlen(Str));
      if (q != NULL)
        strcat(p,q+1);
      strcat(p,Old);
      New[Settings.RightMargin] = 0;  // guarantee that it doesn't get too long!
      strcpy(Line,New);               //lint !e669  line above guarantees its okay

      if (Status.CurSecLevel < PcbData.SysopSec[SEC_SUBS])
        removetokens(Line);

    } else {
      newline();
      getpcbtextshowcolor(TXT_WASNOTFOUNDINLINE,&Buf);
      sprintf(Old,"(%s%s %d",Str,Buf.Str,Num+1);
      println(Old);
    }
  }
}


/********************************************************************
*
*  Function:  msgcleanup()
*
*  Desc    :  Used to free up any memory that has been allocated in this
*             module while not disturbing unallocated memory.
*/

static void _NEAR_ LIBENTRY msgcleanup(void) {
  checkstack();
  if (Header != NULL) {
    bfree(Header);
    Header = NULL;
  }
  if (Msg != NULL) {
    bfree(Msg);
    Msg = NULL;
  }
  if (Status.TaggedText != NULL) {
    bfree(Status.TaggedText);
    Status.TaggedText = NULL;
  }

  freehdrs();

  HideSaveMsg   = FALSE;
  EditMsg       = FALSE;
  EditHdr       = FALSE;
}


static void _NEAR_ LIBENTRY scanlistandsetflags(unsigned ConfNum) {
  int  X;
  long ToNameRecNum;
  char ToName[26];

  checkstack();
  showactivity(ACTBEGIN);
  ToName[25] = 0;
  for (X = 0; X < NumExtHdrs; X++) {
    if (memcmp(ExtHdr[X]->Function,ExtHdrFunctions[EXTHDR_LIST],EXTFUNCLEN) == 0) {
      memcpy(ToName,ExtHdr[X]->Descript,25);
      if ((ToNameRecNum = finduser(ToName)) != -1)
        putmessagewaiting(ConfNum,ToNameRecNum);
    }
    showactivity(ACTSHOW);
  }
  showactivity(ACTEND);
}


/********************************************************************
*
*  Function:  addtabs()
*
*  Desc    :  Converts spaces to tabs according to DOS standards
*/

static void _NEAR_ LIBENTRY addtabs(char *Str) {
  int  Num;

  checkstack();
  while (1) {
    if (strlen(Str) < 8)           /* must be 8 chars or longer */
      return;

    for (Num = 0, Str += 7; *Str == ' ';) { /* Skip forward 7 chars and */
      Num++;                     /* see if it's a space - if   */
      Str--;                     /* so then record it and move */
      if (Num == 8)              /* backward - exit if we find */
        break;                   /* 8 spaces in a row          */
    }

    Str++;                       /* fix our pointer            */
    if (Num < 2) {               /* if we found less than 2    */
      Str += Num;                /* then nothing can be saved  */
    } else {                     /* otherwise we'll conver the */
      *Str = 9;                  /* spaces to a single TAB and */
      strcpy(&Str[1],&Str[Num]); /* then remove the spaces     */
      Str++;                     /* get ready for next scan    */
    }
  }
}


/********************************************************************
*
*  Function:  packmessage()
*
*  Desc    :  1) converts spaces to tabs (if requested)
*             2) packs the message buffer in preparation for writing to disk
*             3) fills the rest of the Header structure
*
*  Returns : FALSE if the message is empty (to avoid saving an empty message)
*/

static bool _NEAR_ LIBENTRY packmessage(void) {
  bool RemoveTokens;
  int  Num;
  char *p;
  char Time[6];

  checkstack();

  // if HideSaveMsg is TRUE then this is an AUTOMATED message so we do not
  // want to remove tokens (macros).

  RemoveTokens = (bool) (Status.CurSecLevel < PcbData.SysopSec[SEC_SUBS] && ! HideSaveMsg);

  HighNum = lastline();
  if (AddTabs)
    for (Num = 0; Num < HighNum; Num++)
      addtabs(Msg[Num].Line);

  stripright(Msg[0].Line,' ');

  p = strchr(Msg[0].Line,0);      //lint !e613 there's always a 0 at the end
  *p = LineSeparator;             //lint !e613 there's always a 0 at the end
  p++;                            //lint !e613 there's always a 0 at the end

  // with the advent of GREATER-than-79 column messages, it is possible that
  // the first NULL encountered in the above strchr() search is already beyond
  // the first line of the message buffer.  So calculate how many buffers were
  // used up to the NULL, then add 1 to start picking up the next line.

  Num = (((unsigned) (p - Msg[0].Line)) / sizeof(msgbuffer)) + 1;

  for (; Num < HighNum; Num++) {
    stripright(Msg[Num].Line,' ');
    if (strlen(Msg[Num].Line) > 79) {
      // from here down the message is too wide to be processed this way,
      // the only way it could have gotten in here is through an upload, so
      // ASSUME the rest of the message is already formatted
      strcpy(p,Msg[Num].Line);
      p = strchr(Msg[Num].Line,0);
      break;
    }
    strcpy(p,Msg[Num].Line);
    p = strchr(p,0);
    *p = LineSeparator;
    p++;
  }

  *p = 0;  // terminate the string so that strlen() will work

  if (RemoveTokens)
    removetokens(Msg[0].Line);

  // if it's a RETURN RECEIPT acknowledgement then the body of the message
  // will be empty - don't bother aborting on an empty message

  if ((Num = strlen(Msg[0].Line)) == 1)
    if (! CreateReceipt)
      return(FALSE);

  // change ASCII 29's to ESC codes
  change(Msg[0].Line,29,27);

  // add size of all extended headers into the calculation
  Num += (sizeof(msgextendtype) * NumExtHdrs);

  // find out the number of 128 byte blocks
  Header->NumBlocks = (char) (Num / 128);

  // round up the number to cover any bytes that are beyond the last full
  // 128 byte block, but *only* if doing so results in a block count of
  // less than 255 or less.
  if ((Num & 127) != 0 && Header->NumBlocks < 254) {
    Header->NumBlocks++;
    // fill in the leftover bytes with spaces
    memset(p,' ',((int) Header->NumBlocks*128)-Num);
  }

  // now add one to the number of blocks to count the header block
  // but only if doing so won't cause the value to go to 0 (NumBlocks is stored
  // in a single byte, so the range is from 0 to 255)
  if (Header->NumBlocks < 255)
    Header->NumBlocks++;

  if (! EditMsg && ! CreateReceipt)
    moveback(Header->FromField,Status.DisplayName,sizeof(UsersRead.Name));

  Header->ActiveFlag = MSG_ACTIVE;
  datestr(Header->Date);
  memcpy(Header->Time,timestr2(Time),5);
  return(TRUE);
}




static void LIBENTRY pcbshowsaving(unsigned short ConfNum, long MsgNum, long MsgOffset) {
  bool         SessionClock;
  bool         KeyboardClock;
  int          Num;
  pcbtexttype  Buf;

  checkstack();
  getsystext(MsgLeftNum,&Buf);

  if (MsgLeftNum == TXT_MESSAGEMOVED || MsgLeftNum == TXT_MESSAGECOPIED) {
    lascii(&Buf.Str[strlen(Buf.Str)],MovedNum);
    strcat(Buf.Str," to ");
    ascii(&Buf.Str[strlen(Buf.Str)],ConfNum);
    strcat(Buf.Str," as ");
  } else {
    makeconfstrsys(&Buf.Str[strlen(Buf.Str)],ConfNum);
    strcat(Buf.Str,"# ");
  }

  Num = strlen(Buf.Str);
  lascii(&Buf.Str[Num],MsgNum);
  if (PostedBy == FROMPPL)
    strcat(Buf.Str," via PPL");

  Status.LastMsgOffset = MsgOffset;
  Status.LastMsgNum    = MsgNum;

  if (! HideSaveMsg) {
    #ifdef COMM
      bool IgnoreCD = Asy.IgnoreCDLoss;
      Asy.IgnoreCDLoss = TRUE;
    #endif

    SessionClock = Control.WatchSessionClock;
    KeyboardClock = Control.WatchKbdClock;
    Control.WatchSessionClock = FALSE;
    Control.WatchKbdClock = FALSE;
    displaypcbtext(MsgSaveNum,LFBEFORE);
    println(&Buf.Str[Num]);
    Control.WatchSessionClock = SessionClock;
    Control.WatchKbdClock = KeyboardClock;
    #ifdef COMM
      Asy.IgnoreCDLoss = IgnoreCD;
    #endif
  }

  writelog(Buf.Str,SPACERIGHT);
}


/********************************************************************
*
*  Function:  storemessage()
*
*  Desc    :  1) opens the message base and locks it for writing
*             2) writes the new message out, updating msg base and index files
*             3) cleans up the memory if needed
*             4) counts the message if it is NEW (not an edited message)
*
*  Returns :  -1 if an error occured or 0 if all went well
*/


static int _NEAR_ LIBENTRY storemessage(unsigned short ConfNum, cleanuptype Mem, bool CountMsg, postedtype Posted, char NetTag) {
  char         Answer[2];
  char        *p;
  msgbasetype  MsgBase;

  checkstack();
  if (openmessagebase(ConfNum,&Status.CurConf,&MsgBase,RDWRLOCK) == -1) {
    displaypcbtext(TXT_ERRORSAVINGMESSAGE,NEWLINE|LOGIT);
    return(-1);
  }

  if (OldIndex || Status.CurConf.OldIndex) {
    if (freemsgs(&MsgBase.Stats) < 1) {
      displaypcbtext(TXT_NOROOMFORTEXT,NEWLINE|LOGIT);
      closemessagebase(&MsgBase);
      return(-1);
    }
  }

  #ifdef PCB_DEMO
    /* limit the DEMO version to no more than 10 active messages */
    if (MsgBase.Stats.NumActiveMsgs > 10) {
      displaypcbtext(TXT_NOROOMFORTEXT,NEWLINE|LOGIT);
      closemessagebase(&MsgBase);
      return(-1);
    }
  #endif

  // in case OTHER software has posted a new message without updating
  // the index, go update it now before writing our new entry to disk
  updatemsgindex();

  switch (NetTag) {
    case   0:
    case ' ':
    case 255: Header->NetTag = 0;   break;
    default : Header->NetTag = '*'; break;
  }

  memset(Header->Reserved,0,sizeof(Header->Reserved));
  Header->ExtendedStatus = ExtendedStatus;

  if (EditMsg) {
    if (Header->NumBlocks != OldNumBlocks) {
      if (Header->NumBlocks < OldNumBlocks) {
        p = Msg[0].Line;
        memset(&p[(Header->NumBlocks-1)*128],' ',(OldNumBlocks-Header->NumBlocks)*128);
        Header->NumBlocks = OldNumBlocks;
      } else {
        EditMsg = FALSE;
        MsgGrew = TRUE;
        OldMsgNumber = 0;
        OldOffset    = 0;
      }
    }
  } else {
    OldMsgNumber = 0;
    OldOffset    = 0;
  }

  PostedBy = Posted;

  savetomsgbase(ConfNum,
                &Status.CurConf,
                &MsgBase,
                Header,
                Msg[0].Line,
                OldMsgNumber,
                OldOffset,
                pcbshowsaving);  /*lint !e534 */

  closemessagebase(&MsgBase);

  if (ExtendedStatus & HDR_CARB) {
    scanlistandsetflags(ConfNum);
  } else {
    if (ToRecNum > 0) {
      putmessagewaiting(ConfNum,ToRecNum);
      ToRecNum = 0;  /* reset it so it doesn't get used again accidently */
    } else if (Header->ToField[0] == '@') {
      Answer[0] = YesChar;
      Answer[1] = 0;
      inputfieldstr(Answer,"Set Mail Waiting Flags",PCB_RED,1,NEWLINE|FIELDLEN|YESNO,0,mask_yesno);
      if (Answer[0] == YesChar) {
        displaypcbtext(TXT_SCANNING,LFBEFORE);
        scanusersputmessagewaiting(ConfNum);
      }
    }
  }

  if (CountMsg) {
    UsersData.MsgsLeft++;
    #ifdef PCBSTATS
      updatestats(1,0,0,UPDATE);
    #endif

    #ifdef PCB152
      chargeformessage(Header->Status,(bool) (Header->EchoFlag == 'E'),Header->ToField);
    #endif
  }

  if (Mem == CLEANUP)
    msgcleanup();

  return(0);
}

/********************************************************************
*
*  Function:  gettoname()
*
*  Desc    :  prompts the user for the "TO" name for the message
*
*  Returns :  TRUE if the message is to "ALL", FALSE otherwise
*/

#pragma warn -rvl
bool LIBENTRY gettoname(int PcbTextNum, char *To) {
  bool        Validate;
  bool        List;
  bool        WasList;
  int         CountList;
  DISPLAYTYPE DispCtrl;
  char        Str[2];
  long        SearchNum;
  char        Found[26];
  pcbtexttype Buf;
  char        Temp[128];

  checkstack();
  Validate  = (bool) (PcbData.Validate && ! Status.CurConf.EchoMail);
  WasList   = (bool) (strcmp(To,"@LIST@") == 0);
  CountList = 0;

top:
  while (1) {
    ToRecNum = 0;

    if (Status.CurConf.LongToNames) {
      DispCtrl = LFBEFORE|HIGHASCII;
      if (To[0] != 0)
        DispCtrl |= FIELDLEN|GUIDE;
      inputfield(To,PcbTextNum,EXTDESCLEN*2,DispCtrl,HLP_E,mask_alphanum);
    } else
      inputfield(To,PcbTextNum,25,FIELDLEN|LFBEFORE|GUIDE|HIGHASCII,HLP_E,mask_alphanum);

    if (Status.KbdTimedOut) {
      To[0] = 0;
      return(FALSE);
    }

    stripleft(To,' ');
    if (PcbTextNum != TXT_CARBONLIST && strlen(To) > 25) {
      if (PcbTextNum != TXT_NEWINFO) {
        buildextheader(EXTHDR_TO,To,HDR_TO);
        if (strlen(To) > EXTDESCLEN)
          buildextheader(EXTHDR_TO2,&To[EXTDESCLEN],HDR_TO);
        To[0] = 0;
      }
      return(FALSE);
    }

    if (Status.CurConf.LongToNames && strchr(To,'@') != NULL) {
      // do NOT force to uppercase if it is an internet address
    } else
      strupr(To);

    strcpy(Temp,To);
    strupr(Temp);

    // check for @LIST@  ... but don't let them use the EDIT HEADER function to
    // enter an @LIST@ name

    if (strcmp(To,"@LIST@") == 0) {
      // if we were EDITING an existing header on a message, and it was @LIST@
      // and it is STILL set to @LIST@ then just get out now
      if (PcbTextNum == TXT_NEWINFO && WasList)
        return(FALSE);

      if (Status.CurSecLevel >= Status.CurConf.CarbonLevel && Status.CurConf.CarbonLimit > 1 && PcbTextNum != TXT_NEWINFO) {
        List = FALSE;
        newline();
        while (1) {
          To[0] = 0;
          gettoname(TXT_CARBONLIST,To);  /*lint !e534 */
          if (To[0] == 0)
            break;

          buildextheader(EXTHDR_LIST,To,HDR_CARB);
          List = TRUE;
          if (++CountList >= Status.CurConf.CarbonLimit) {
            displaypcbtext(TXT_CARBONLIMITREACHED,NEWLINE|LFBEFORE);
            break;
          }
        }

        newline();

        if (List) {
          strcpy(To,"@LIST@");
          return(FALSE);
        }
        strcpy(To,"ALL");
        return(TRUE);
      } else {
        strcpy(To,"ALL");
        return(TRUE);
      }
    }

    if (To[0] == 0) {
      if (PcbTextNum != TXT_CARBONCOPYTO && PcbTextNum != TXT_CARBONLIST) {
        strcpy(To,"ALL");
        return(TRUE);
      }
      return(FALSE);
    } else {
      if (strcmp(Temp,"ALL") == 0)
        return(TRUE);
      getpcbtext(TXT_ALLNAME,&Buf);
      if (strcmp(To,Buf.Str) == 0)
        return(TRUE);
      if (Status.CurSecLevel >= PcbData.SysopSec[SEC_GENERICMSGS]) {
        if (strchr(To,'@') != NULL)
          return(FALSE);
      } else {
        removetokens(To);
        if (To[0] == 0 || To[0] == '@') {
          strcpy(To,"ALL");
          return(TRUE);
        }
      }

      if (strcmp(Temp,"SYSOP") == 0) {
        ToRecNum = 1;
        return(FALSE);
      }

      if ((ToRecNum = finduser(Temp)) != -1) {
validatename:
        if (Validate) {
          if ((! Status.CurConf.AllowAliases && ! isrealname(ToRecNum,Temp)) ||
              ! userreginconf(Status.Conference,ToRecNum)) {
            newline();
            displaypcbtext(TXT_USERNOTREGINCONF,NEWLINE|LFBEFORE);
            goto askreenter;
          }
        }
        return(FALSE);
      } else if (! Validate)
        return(FALSE);
      else {
        newline();
        stripright(Temp,' ');
        strcpy(Status.DisplayText,Temp);
        displaypcbtext(TXT_COULDNTFINDINUSERS,NEWLINE|LFBEFORE);
        Str[0] = 'S';
        Str[1] = 0;
        inputfield(Str,TXT_DOSOUNDEXSEARCH,1,FIELDLEN|UPCASE|NEWLINE,HLP_E,mask_crs);
        if (Status.KbdTimedOut)
          return(FALSE);

        switch (Str[0]) {
          case 'R': break;
          case 'C': return(FALSE);
          case 'S':
          default :
            SearchNum = 0;
            while (1) {
              if ((ToRecNum = findusersoundex(Temp,Found,&SearchNum)) > 0) {

                // don't let him find users that are NOT registered in this
                // conference
                if (! userreginconf(Status.Conference,ToRecNum))
                  continue;

                // don't let him find non-alias names if this is a conference
                // where alias names are in use when we are trying to protect
                // alias names
                if (AliasSupport && Status.CurConf.AllowAliases && ProtectAlias && isrealname(ToRecNum,Found))
                  continue;

                maxstrcpy(Status.DisplayText,Found,sizeof(Status.DisplayText));
                displaypcbtext(TXT_FOUNDNAME,NEWLINE|LFBEFORE);
                Str[0] = 'U';
                inputfield(Str,TXT_USEFOUNDNAME,1,FIELDLEN|UPCASE|NEWLINE,HLP_E,mask_crsu);
                if (Status.KbdTimedOut)
                  return(FALSE);

                switch(Str[0]) {
                  case 'S': break;
                  case 'R': goto top;
                  case 'C': return(FALSE);
                  case 'U':
                  default : strcpy(To,Found);
                            if (Validate) {
                              strcpy(Temp,Found);
                              goto validatename;
                            }
                            return(FALSE);
                }
              } else {
                strcpy(Status.DisplayText,Temp);
                displaypcbtext(TXT_COULDNTFINDINUSERS,NEWLINE|LFBEFORE);
askreenter:
                Str[0] = 'R';
                Str[1] = 0;
                inputfield(Str,TXT_REENTERUSERSNAME,1,FIELDLEN|UPCASE|NEWLINE,HLP_E,mask_cr);
                if (Status.KbdTimedOut)
                  return(FALSE);

                switch (Str[0]) {
                  default :
                  case 'R': goto top;
                  case 'C': return(FALSE);
                }
              }
            }
        }
      }
    }
  }
}
#pragma warn +rvl


/********************************************************************
*
*  Function:  getsecurity()
*
*  Desc    :  prompts the user for the message security to be used
*/

static void _NEAR_ LIBENTRY getsecurity(bool ToAll) {
  unsigned short Date;
  char     Sec[2];
  char     Str[13];

  checkstack();
  if (memcmp(Header->ToField,"@LIST@",6) == 0) {
    Header->Status = MSG_RCVR;
    return;
  }

  Header->Status = 0;
  while (Header->Status == 0) {
    mask_gnrsd[0] = (char) (Status.CurSecLevel >= PcbData.SysopSec[SEC_KEEPMSG] ? 5 : 4);

    Sec[0] = 'N';
    Sec[1] = 0;
    inputfield(Sec,TXT_MSGSECURITY,sizeof(Sec)-1,FIELDLEN|UPCASE|NEWLINE,HLP_SEC,mask_gnrsd);
    switch (Sec[0]) {
      case  0 :
      case 'N': Header->Status = MSG_PBLC;
                break;
      case 'D': Header->Status = MSG_PBLC;
                Date = 0;
                while (1) {
                  Str[0] = 0;
                  inputfield(Str,TXT_ENTERPACKDATE,6,FIELDLEN|GUIDE|NEWLINE|LFBEFORE,NOHELP,mask_numbers);
                  if (Str[0] == 0)
                    break;
                  uncountrydate2(Str);
                  if ((Date = datetojulian(Str)) != 0) {
                    buildextheader(EXTHDR_PACKOUT,juliantodate(Date),0);
                    break;
                  }
                }
                break;
      case 'R': if (! ToAll) {
                  if (Status.CurConf.NoPrivateMsgs)
                    displaypcbtext(TXT_NOPRIVMSGS,NEWLINE|LFBEFORE|BELL);
                  else
                    Header->Status = MSG_RCVR;
                } else
                  displaypcbtext(TXT_CANTPROTECTMSGTOALL,NEWLINE);
                break;
      case 'S': Header->Status = MSG_SPWD;
                Str[0] = 0;
                inputfield(Str,TXT_SECURITYPASSWORD,12,FIELDLEN|UPCASE|NEWLINE|HIGHASCII,HLP_SEC,mask_alphanum);
                padstr(Str,' ',12);
                memcpy(Header->Password,Str,12);
                break;
      case 'G': Header->Status = (ToAll ? MSG_GPWD_ALL : MSG_GPWD);
                Str[0] = NoChar;
                Str[1] = 0;
                inputfield(Str,TXT_CALLERMUSTKNOWPWRD,1,YESNO|FIELDLEN|UPCASE|NEWLINE|LFBEFORE,HLP_SEC,mask_yesno);
                if (Str[0] != YesChar) {
                  Header->Status = 0;
                  break;
                }
                Str[0] = 0;
                inputfield(Str,TXT_SECURITYPASSWORD,12,FIELDLEN|UPCASE|NEWLINE|HIGHASCII,HLP_E,mask_alphanum);
                if (Str[0] == 0) {
                  Header->Status = 0;
                  break;
                }
                padstr(Str,' ',12);
                memcpy(Header->Password,Str,12);
                break;
    }
  }
}


/********************************************************************
*
*  Function:  getretreceipt()
*
*  Desc    :  On private messages, asks caller if RETURN RECEIPT is desired
*/

static void _NEAR_ LIBENTRY getretreceipt(void) {
  char Str[2];

  checkstack();
  if (Status.CurSecLevel >= Status.CurConf.RetReceiptLevel) {
    Str[0] = NoChar;
    Str[1] = 0;
    inputfield(Str,TXT_REQRETRECEIPT,sizeof(Str)-1,FIELDLEN|UPCASE|NEWLINE|YESNO,HLP_SEC,mask_yesno);
    if (Str[0] == YesChar)
      buildextheader(EXTHDR_REQRR,"Caller has requested a Return Receipt",HDR_RCPT);
  }
}


/********************************************************************
*
*  Function:  displayreferbody()
*
*  Desc    :  displays the body of a referred to message - used by the quoter
*/

static void _NEAR_ LIBENTRY displayreferbody(msgbasetype *MsgBase, int StartLine) {
  unsigned Bytes;
  int      Line;
  char *p;
  char *q;
  char Str[80];

  checkstack();
  Bytes = (MsgBase->Header.NumBlocks-1)*128; /* subtract 1 from NumBlocks to get Body length */
  p = skipextendedheader(MsgBase->Body);
  Bytes -= ((int) (p - MsgBase->Body));

  if (Status.FullScreen)
    printcls();

  startdisplay(FORCECOUNTLINES);
  printtopline(&MsgBase->Header);

  Line = 1;
  while ((q = strnchr(p,LineSeparator,Bytes)) != NULL && !Display.AbortPrintout) {
    if (*p != 1) {
      // only display and count lines that do NOT start with CTRL-A characters (used by FIDO)
      if (Line >= StartLine && *p != 1) {
        printcolor(PCB_RED);
        sprintf(Str,"%3d: ",Line);
        print(Str);
        printdefcolor();
        *q = 0;
        maxstrcpy(Str,p,75);  /* make sure it doesn't go off the screen edge */
        *q = LineSeparator;
        stripright(Str,' ');
        printxlated(Str);
        newline();
      }
      Line++;
    }
    if ((Bytes -= ((int) (q-p+1))) <= 0)
      break;
    p = q + 1;
  }
  checkdisplaystatus();
}


/********************************************************************
*
*  Function:  linequote()
*
*  Desc    :  brings in a group of lines into the line editor
*/

static int _NEAR_ LIBENTRY linequote(msgbasetype *MsgBase) {
  bool SaveFullScreen;
  int  StartLine;
  int  EndLine;
  int  Line;
  int  NumTokens;
  unsigned Size;
  unsigned Bytes;
  char *p;
  char *q;
  char Str[8];
  char Buffer[80];

  checkstack();
  displayreferbody(MsgBase,1);

  // this is going to be Line Oriented so change out of Full Screen mode
  // so that if showmessage() is called to display a warning about running
  // out of time it will not show up in the wrong location
  SaveFullScreen = Status.FullScreen;
  Status.FullScreen = FALSE;

  StartLine = 0;
  EndLine = 0;

  while (StartLine == 0) {
    newline();
    if (Status.FullScreen)
      cleareol();
    Str[0] = '1';
    Str[1] = 0;
    inputfield(Str,TXT_QUOTESTART,sizeof(Str)-1,NEWLINE|UPCASE|FIELDLEN|STACKED,NOHELP,mask_referlist);
    if (Status.KbdTimedOut || Str[0] == 'Q') {
      Status.FullScreen = SaveFullScreen;
      return(-1);
    }
    NumTokens = tokenizestr(Str);
    if (NumTokens) {
      StartLine = atoi(getnexttoken());
      NumTokens--;
    } else StartLine = 1;
  }

  if (NumTokens) {
    EndLine = atoi(getnexttoken());
    if (EndLine < StartLine)
      EndLine = 0;
  }

  while (EndLine == 0) {
    ascii(Str,StartLine+1);
    inputfield(Str,TXT_QUOTEEND,sizeof(Str)-1,NEWLINE|UPCASE|FIELDLEN,NOHELP,mask_referlist);
    if (Status.KbdTimedOut || Str[0] == 'Q') {
      Status.FullScreen = SaveFullScreen;
      return(-1);
    }
    EndLine = atoi(Str);
    if (EndLine < StartLine)
      EndLine = 0;
  }

  Status.FullScreen = SaveFullScreen;
  Size =(EndLine-StartLine+1) * (sizeof(msgbuffer) + 3); /* add 3 bytes for arrow */
  if ((Status.TaggedText = (char *) checkmalloc(Size,"LIST")) == NULL)
    return(-1);

  memset(Status.TaggedText,0,Size);
  Line = 1;
  Status.TaggedText[0] = 0;
  Status.TagPointer = Status.TaggedText;

  Bytes = (MsgBase->Header.NumBlocks-1)*128; /* subtract 1 from NumBlocks to get Body length */
  p = skipextendedheader(MsgBase->Body);
  Bytes -= ((int) (p - MsgBase->Body));

  while ((q = strnchr(p,LineSeparator,Bytes)) != NULL) {
    // only process and count lines that do NOT start with CTRL-A (used by FIDO)
    if (*p != 1) {
      if (Line >= StartLine) {
        if (Line > EndLine)
          break;
        *q = 0;
        maxstrcpy(Buffer,p,sizeof(Buffer));
        // check for a line full of characters with NO spaces going all the
        // way to the right margin (remember "-> " chars) and possibly beyond
        if (strlen(Buffer) >= Settings.RightMargin-3) {
          // if such a line is found, then truncate it
          if (strchr(Buffer,' ') == NULL)
            Buffer[Settings.RightMargin-3] = 0;
        }
        *q = LineSeparator;
        addtaggedtext(Buffer);
      }
      Line++;
    }
    if ((Bytes -= ((int) (q-p+1))) <= 0)
      break;
    p = q + 1;
  }

  if (*(Status.TagPointer-1) == 13)
    *(Status.TagPointer-1) = 0;
  stripright(Status.TaggedText,' ');
  Status.TagPointer = Status.TaggedText;
  return(0);
}


static void _NEAR_ LIBENTRY header(void) {
  checkstack();
  if (Status.FullScreen) {
    printcls();
    printtopline(Header);
  } else {
    displaycmdfile("PREEDIT");
    newline();
    if (Settings.MaxFieldLen != 79)
      print("    ");
    displaypcbtext(TXT_MSGENTERTEXT,DEFAULTS);
    ascii(Status.DisplayText,MaxMsgLines);
    displaypcbtext((Settings.MaxFieldLen == 79 ? TXT_79COLUMNS : TXT_72COLUMNS),NEWLINE);
    printcolor(PCB_CYAN);
    printdivider();
  }
}


static void _NEAR_ LIBENTRY showeditscreen(void) {
  int       X;
  msgbuffer *p;
  char      Str[10];

  checkstack();
  printdefcolor();
  wsmovecursor(1,3,ABSOLUTE);
  BottomLine = TopLine + PageLen;
  Display.Break = FALSE; /* turn off CTRL-X, CTRL-K checking while displaying */

  for (X = TopLine+1, p = &Msg[X-1]; X <= BottomLine; X++, p++) {
    if (X <= MaxMsgLines) {
      if (Settings.ShowLineNums) {
        printcolor(PCB_CYAN);
        sprintf(Str,"%3d: ",X);
        print(Str);
        printdefcolor();
      }
      stripright(p->Line,' ');
      if (p->Line[0] != 0)
        printnoesc(p->Line);
    }
    cleareol();
    newline();
  }

  wsmovecursor(8,PageLen+4,ABSOLUTE);
  displaypcbtext(TXT_ESCTOEXIT,DEFAULTS);
  showinsertmode();
  printdefcolor();

  if (BottomLine > MaxMsgLines)
    BottomLine = MaxMsgLines;

  Display.Break = TRUE;
}


static void _NEAR_ LIBENTRY removefileattachment(void) {
  int  NumTokens;
  char *p;
  char FileName[66];
  char Str[80];

  checkstack();
  if ((p = checkextendedheaders(EXTHDR_ATTACH,NULL)) != NULL) {
    strcpy(Str,p);
    NumTokens = tokenizestr(Str);
    if (NumTokens >= 3) {
      getnexttoken();  //lint !e534  skip over VIEW name
      getnexttoken();  //lint !e534  skip over file size
      buildstr(FileName,Status.CurConf.AttachLoc,getnexttoken(),NULL); // get STORED name
      unlink(FileName);
    }
  }
}


static int _NEAR_ LIBENTRY fullscreeneditor(msgbasetype *MsgBase) {
  rettype     Stat;
  msgbuffer   *p;
  char        Str[10];
  char        OverFlow[256];

  checkstack();
  memcpy(CharXlat,FullScreenXlat,sizeof(FullScreenXlat));
  if (PcbData.AllowEscCodes)
    CharXlat[29] = 29;

  Stat = NEXTLINE;
  OverFlow[0] = 0;
  PageLen     = (Display.PageLen == 0 || Display.PageLen >= 22 ? 22-2 : Display.PageLen-2);
  TopLine     = (LineNum >= PageLen ? LineNum-1 : 0);
  Display.Break = FALSE;
  showeditscreen();
  wsmovecursor(0,LineNum,RELATIVE);

  while (1) {
    if (! Settings.ShowLineNums) {
      sprintf(Str,"%4d",LineNum+1);
      wsmovecursor(80-4,1,ABSOLUTE);
      print(Str);
    }

    p = &Msg[LineNum];
    if (Stat == NEXTLINE && OverFlow[0] != 0) {
      wsmovecursor(0,LineNum,RELATIVE);
      if (Status.TaggedText != NULL) {
        strcpy(p->Line,"-> ");
        strcpy(&p->Line[3],OverFlow);
      } else strcpy(p->Line,OverFlow);
      Pos = strlen(p->Line);
      print(p->Line);
    } else {
      wsmovecursor(Pos,LineNum,RELATIVE);
    }

    Stat = inputline(OverFlow);
    HighNum = lastline();

    if (Control.WarnMinute != 0 && Stat != BREAKOUT)
      warntime();

    if (Stat == LOSTCD)
      return(-1);

    if (Stat == BREAKOUT)
      break;

    switch (Stat) {
      case NEXTLINE: if (LineNum < MaxMsgLines-1) {
                       LineNum++;
                       if (LineNum > BottomLine-1 && BottomLine == TopLine+PageLen) {
                         TopLine = LineNum-2;
                         showeditscreen();
                       }
                     }
                     break;
      case PREVLINE: if (LineNum > 0) {
                       LineNum--;
                       if (LineNum <= TopLine-1 && TopLine > 0) {
                         if (TopLine > PageLen)
                           TopLine = LineNum-PageLen+3;
                         else TopLine = 0;
                         showeditscreen();
                         wsmovecursor(Pos,LineNum,RELATIVE);
                       }
                     } else Stat = STAY;
                     break;
      case SHOWSCRN: showeditscreen();
                     break;
      case REPAINT : header();
                     showeditscreen();
                     break;
      case QUOTE   : if (MsgBase != NULL) {
                       wsmovecursor(1,PageLen+3,ABSOLUTE);
                       if (linequote(MsgBase) != -1) {
                         Pos = 0;
                         wsmovecursor(0,LineNum,RELATIVE);
                       }
                       header();
                       showeditscreen();
                     }
                     break;
    }
  }
  TopLine = 0;
  wsmovecursor(1,PageLen+3,ABSOLUTE);
  return(0);
}



static int _NEAR_ LIBENTRY lineeditor(void) {
  int       Num;
  rettype   Stat;
  msgbuffer *p;
  char      Str[10];
  char      OverFlow[128];

  checkstack();
  memcpy(CharXlat,LineEditorXlat,sizeof(LineEditorXlat));
  if (PcbData.AllowEscCodes) {
    CharXlat[27] = 27;
    CharXlat[29] = 29;
  }

  OverFlow[0] = 0;

  while (1) {
    if (HighNum == MaxMsgLines-2)
      displaypcbtext(TXT_TWOLINESLEFT,NEWLINE);
    else if (HighNum >= MaxMsgLines) {
      displaypcbtext((Insert ? TXT_CANNOTINSERT : TXT_TEXTENTRYFULL),NEWLINE);
      Insert = FALSE;
      break;
    }

    if (Insert)
      insertline(LineNum);  /*lint !e534 */

    if (Settings.ShowLineNums) {
      sprintf(Str,"%3d: ",LineNum+1);
      printcolor(PCB_CYAN);
      print(Str);
      printdefcolor();
    }

    p = &Msg[LineNum];
    if (OverFlow[0] != 0) {
      if (Status.TaggedText != NULL) {
        strcpy(p->Line,"-> ");
        strcpy(&p->Line[3],OverFlow);
      } else strcpy(p->Line,OverFlow);
    }
    printnoesc(p->Line);

    Pos = strlen(p->Line);
    Stat = inputline(OverFlow);
    HighNum = max(lastline(),LineNum+1);
    newline();

    if (Control.WarnMinute != 0)
      warntime();

    if (Stat == BREAKOUT || Stat == LOSTCD)
      break;

    LineNum++;
  }

  if (Insert) {
    for (Num = LineNum+1; Num < HighNum+1; Num++)
      strcpy(Msg[Num-1].Line,Msg[Num].Line);
    Insert = FALSE;
    LineNum = HighNum;
  }

  return(Stat == LOSTCD ? -1 : 0);
}


static bool _NEAR_ LIBENTRY receiveattachment(void) {
  bool     Success;
  bool     SaveCountLines;
  long     Size;
  char     NewPath[66];
  char     Desc[80];
  spectype File;

  checkstack();
  if (! proceedwithupload())
    return(FALSE);

  Success = FALSE;
  Status.FileAttach = TRUE;
  Status.DefaultProtocol = 'N';  // force caller to pick a protocol
  SaveCountLines = Display.CountLines;
  receive(0);
  Display.CountLines = SaveCountLines;
  Status.DefaultProtocol = 0;
  Status.FileAttach = FALSE;

  if (NumFiles && getfilespec(0,&File) != -1) {
    if (fileexist(File.FullPath) != 255 && File.Success == UPOKAY && ! File.Failed) {
      Size = DTA.ff_fsize;
      makefilenameunique(NewPath,File.FullPath);
      sprintf(Desc,"%s (%ld) %s",File.Name,Size,findstartofname(NewPath));
      buildextheader(EXTHDR_ATTACH,Desc,HDR_FILE);
      Success = TRUE;
      strcpy(Status.DisplayText,File.Name);
      logsystext(TXT_ATTACHMENT,SPACERIGHT);
    }
  }

  deallocatefilelist(THROWAWAY);
  deallocatefiledesc();
  return(Success);
}


static bool _NEAR_ LIBENTRY readfileintomsg(char *FileName, bool Live) {
  bool      TooLarge;
  unsigned  MaxLen;
  unsigned  Len;
  unsigned  BytesLeft;
  msgbuffer *p;
  char      *q;
  DOSFILE   File;
  char      Str[2048];

  checkstack();
  TooLarge = FALSE;
  if (fileexist(FileName) != 255 && dosfopen(FileName,OPEN_READ,&File) != -1) {
    MaxLen = 0;
    while (dosfgets(Str,sizeof(Str),&File) != -1) {
      Len = strlen(Str);
      if (Len > MaxLen)
        MaxLen = Len;
    }

    p = &Msg[LineNum];
    dosrewind(&File);

    if (MaxLen <= (unsigned) Wide.MaxFieldLen) {
      if (MaxLen > (unsigned) Settings.MaxFieldLen)
        Settings = Wide;
      while (dosfgets(Str,sizeof(Str),&File) != -1) {
        if (! PcbData.AllowEscCodes)
          stripall(Str,27);
        strcpy(p->Line,Str);
        p = &Msg[++LineNum];
        HighNum++;
        if (LineNum >= MaxMsgLines) {
          if (Live)
            displaypcbtext(TXT_TEXTENTRYFULL,NEWLINE|LFBEFORE);
          break;
        }
      }
    } else {
      TooLarge = TRUE;
      q = (char *) p;
      BytesLeft = (MaxMsgLines * sizeof(msgbuffer)) - ((int) (p - Msg));
      while (dosfgets(Str,sizeof(Str),&File) != -1) {
        if (! PcbData.AllowEscCodes)
          stripall(Str,27);
        Len = strlen(Str);
        if (Len > BytesLeft)  // e.g. 2K read in, but only 800 bytes left
          Len = BytesLeft-1;  // set Len to equal to all that's left
        maxstrcpy(q,Str,BytesLeft);
        q += Len;
        *q = LineSeparator;
        q++;
        BytesLeft -= (Len+1);
        if (BytesLeft <= sizeof(msgbuffer)) {
          if (Live)
            displaypcbtext(TXT_TEXTENTRYFULL,NEWLINE|LFBEFORE);
          break;
        }
      }
      if (q != (void *) p) {
        q--;
        *q = 0;  // put NULL terminator on the buffer
      }
    }

    dosfclose(&File);
  } else if (Live)
    newline();

  return(TooLarge);
}


static bool _NEAR_ LIBENTRY receivemsgupload(void) {
  bool TooLarge;
  char Name[20];

  checkstack();
  Status.MsgUpload = TRUE;
  displaypcbtext(TXT_UPLOADMODE,NEWLINE|LFBEFORE);
  Status.DefaultProtocol = 'N';  // force caller to pick a protocol
  receive(0);
  Status.DefaultProtocol = 0;
  Status.MsgUpload = FALSE;

  sprintf(Name,"MSG%d.$$$",PcbData.NodeNum);
  TooLarge = readfileintomsg(Name,TRUE);
  unlink(Name);

  #ifdef COMM
    if (Asy.Online == REMOTE) {
      Asy.IgnoreCDLoss = FALSE;
      if (cdstillup() == 0) {
        Asy.LostCarrier = TRUE;
        if (PcbData.Packet)
          turnoffdtr();
      }
    }
  #endif

  return(TooLarge);
}


/********************************************************************
*
*  Function:  msgeditor()
*
*  Desc    :  sets up for the lineeditor() and the fullscreeneditor()
*
*  Returns :  ABORTMSG, SAVEMSG, NEXTMSG or KILLMSG depending on the option
*             chosen by the user
*/

static savetype _NEAR_ LIBENTRY msgeditor(msgbasetype *MsgBase, int BeginLine, bool CountMsg) {
  char static mask_sa[] = {2, 'S', 'A'};
  bool OldUseAnsi;
  bool Logoff;
  bool Upload;
  bool TooLarge;
  bool SaveFullScreen;
  int  CarrierStatus;  /* -1 = carrier lost */
  int  Num;
  savetype RetVal;
  int  NumTokens;
  char *p;
  char Str[128];

  checkstack();
  if (Status.ScrollOn)
    scrolloff();

  sprintf(Str,"(%d) => %-25.25s",Status.Conference,Header->ToField);
  stripright(Str,' ');
  writeusernetstatus(ENTERMESSAGE,Str);

  if (UsersData.PackedFlags.DontAskFSE)
    Str[0] = (UsersData.PackedFlags.FSEDefault ? 'Y' : 'N');
  else {
    Str[0] = (UsersData.ExpertMode && UseAnsi ? YesChar : NoChar);
    Str[1] = 0;
    inputfield(Str,TXT_USEFULLSCREEN,1,YESNO|FIELDLEN|UPCASE|NEWLINE,NOHELP,mask_editor);
    if (Str[0] == YesChar && ! UseAnsi) {
      Str[0] = NoChar;
      displaypcbtext(TXT_REQUIRESANSI,NEWLINE|LFBEFORE|LFAFTER);
      inputfield(Str,TXT_USEFULLSCREEN,1,YESNO|FIELDLEN|UPCASE|NEWLINE,NOHELP,mask_editor);
    }
  }

  TooLarge = FALSE;
  Logoff = FALSE;
  HighNum = 0;
  Pos = 0;
  Status.EnteringMessage = TRUE;
  #ifdef COMM
    Asy.IgnoreCDLoss = TRUE;
  #endif
  Status.FullScreen = (bool) (Str[0] != NoChar && Str[0] != 'U');
  Insert  = Status.FullScreen;  /* default to ON for full screen editor */
  LineNum = (Status.FullScreen ? 0 : BeginLine);
  Upload  = (bool) (Str[0] == 'U');
  CarrierStatus = 0;

  OldUseAnsi = UseAnsi;
  if (Status.FullScreen)
    UseAnsi = TRUE;

  if (Upload) {
    TooLarge = receivemsgupload();
    goto checkcarrier;
  } else
    header();

top:
  startdisplay(FORCENONSTOP);  /* force non-stop mode during message entry */
  turnkbdtimeron();            /* but turn the clock back on!              */
  SaveFullScreen = Status.FullScreen;
  if (Status.FullScreen)
    CarrierStatus = fullscreeneditor(MsgBase);
  else
    CarrierStatus = lineeditor();

checkcarrier:
  #ifdef COMM
    if (Asy.LostCarrier) {
      CarrierStatus = -1;
      if (PcbData.Packet)
        turnoffdtr();
    }
  #endif

  if (CarrierStatus == -1) {
    RetVal = ABORTMSG;
    if (packmessage() && storemessage(Status.Conference,CLEANUP,CountMsg,FROMCALLER,0) != -1)
      RetVal = SAVEMSG;
    Logoff = TRUE;
    goto exit;
  }

  while (1) {
    if (Status.FullScreen) {
      Status.FullScreen = FALSE;
      newline();
      cleareol();
    }

    if (TooLarge) {
      displaypcbtext(TXT_MSGTOOWIDE,NEWLINE|LFBEFORE);
      Str[0] = 'S';
      Str[1] = 0;
      inputfield(Str,TXT_MSGTOOWIDESAVE,1,UPCASE|NEWLINE|LFBEFORE,HLP_E,mask_sa);
    } else {
      Str[0] = 0;
      if (UsersData.ExpertMode) {
        inputfield(Str,TXT_MSGCOMMANDEXPERT,sizeof(Str)-1,UPCASE|STACKED|NEWLINE,HLP_E,mask_editmsg);
      } else {
        displaypcbtext(TXT_MSGCOMMANDNOVICE1,NEWLINE|LFBEFORE);
        displaypcbtext(TXT_MSGCOMMANDNOVICE2,NEWLINE);
        inputfield(Str,TXT_TEXTENTRYCOMMAND,sizeof(Str)-1,UPCASE|STACKED|NEWLINE,HLP_E,mask_editmsg);
      }
    }

    #ifdef COMM
      if (Asy.LostCarrier)
        goto checkcarrier;
    #endif

    if (Status.KbdTimedOut) {
      CarrierStatus = -1;
      goto checkcarrier;
    }

    if ((NumTokens = tokenize(Str)) == 0)
      continue;

    p = getnexttoken();
    if (*(p+1) == 0) {
      switch (*p) {
        case 'A': Str[0] = NoChar;
                  Str[1] = 0;
                  inputfield(Str,TXT_MSGABORT,1,YESNO|FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_yesno);
                  if (Str[0] == YesChar) {
                    displaypcbtext(TXT_MSGABORTED,NEWLINE|LFBEFORE|LOGIT);
                    RetVal = ABORTMSG;
                    goto exit;
                  }
                  break;
        case 'U': TooLarge = receivemsgupload();
                  goto checkcarrier;
        case 'C': newline();
                  HighNum = lastline();
                  LineNum = HighNum;
                  if (LineNum > 0)
                    LineNum--;
                  printcolor(PCB_CYAN);
                  printdivider();
                  printdefcolor();
                  Insert = FALSE;
                  goto top;
        case 'D': deletelinenum(NumTokens-1);
                  break;
        case 'E': editline(NumTokens-1);
                  break;
        case 'V': /* for compatibility with ProDoor */
        case 'F': Status.FullScreen = TRUE;
                  Insert = TRUE;  /* default to ON in full screen mode */
                  header();
                  if (LineNum >= MaxMsgLines)
                    LineNum = MaxMsgLines-1;
                  goto top;
        case 'H': break;
        case 'I': if ((Num = getlinenumber(NumTokens-1,TXT_INSERTBEFORENUM)) != -1) {
                    Insert = TRUE;
                    LineNum = Num;
                    goto top;
                  }
                  break;
        case 'L': listmsgbuffer(NumTokens-1);
                  break;
        case 'Q': if (MsgBase != NULL) {
                    if (linequote(MsgBase) != -1) {
                      newline();
                      Msg[LineNum].Line[0] = 0; /* force it into column 1 */
                      Status.FullScreen = SaveFullScreen;  //lint !e644
                      if (Status.FullScreen) {
                        Insert = TRUE;  /* default to ON in full screen mode */
                        header();
                        if (LineNum >= MaxMsgLines)
                          LineNum = MaxMsgLines-1;
                      }
                      goto top;
                    }
                  }
                  break;
        case 'S': if (packmessage()) {
                    RetVal = (storemessage(Status.Conference,CLEANUP,CountMsg,FROMCALLER,0) == -1 ? ABORTMSG : SAVEMSG);
                    goto exit;
                  } else {
                    displaypcbtext(TXT_MSGABORTED,NEWLINE|LFBEFORE|LOGIT);
                    RetVal = ABORTMSG;
                    goto exit;
                  }
      }
      if (Status.KbdTimedOut) {
        CarrierStatus = -1;
        goto checkcarrier;
      }
    } else if (*p == 'S' && *(p+2) == 0) {
      p++;
      if (*p == 'A' || *p == 'C' || *p == 'K' || *p == 'N') {
        if (*p == 'A') {
          if (Status.CurSecLevel < Status.CurConf.AttachLevel || Status.CurConf.AttachLoc[0] == 0) {
            displaypcbtext(TXT_ATTACHNOTALLOWED,NEWLINE|LFBEFORE);
            goto checkcarrier;
          } else if ((ExtendedStatus & HDR_FILE) != 0) {
            displaypcbtext(TXT_ALREADYATTACHED,NEWLINE|LFBEFORE);
            goto checkcarrier;
          } else {
            if (receiveattachment()) {
              if (packmessage()) {
                if (storemessage(Status.Conference,CLEANUP,CountMsg,FROMCALLER,0) == -1) {
                  removefileattachment();
                  RetVal = ABORTMSG;
                } else RetVal = SAVEMSG;
                goto exit;
              }
            }
            goto checkcarrier;
          }
        }

        if (packmessage()) {
          switch (*p) {
            case 'C': while (1) {
                        if (storemessage(Status.Conference,NOCLEANUP,CountMsg,FROMCALLER,0) == -1) {
                          RetVal = ABORTMSG;
                          goto exit;
                        }
                        // don't let carbon copy work if editing an existing
                        // message or if the sysop doesn't allow carbon copies
                        // or if writing a comment to the sysop (especial password failure comments!)
                        // or if there is already an extended TO header
                        if (EditMsg ||
                            (! PcbData.AllowCCs) ||
                            (Header->Status == MSG_CMNT) ||
                            ((ExtendedStatus & HDR_TO) != 0)) {
                          RetVal = SAVEMSG;
                          goto exit;
                        }
                        #ifdef COMM
                          Asy.IgnoreCDLoss = FALSE;
                        #endif
                        Str[0] = 0;
                        gettoname(TXT_CARBONCOPYTO,Str);  //lint !e534
                        if (Status.KbdTimedOut) {
                          CarrierStatus = -1;
                          goto checkcarrier;
                        }
                        if (Str[0] == 0)
                          break;
                        padstr(Str,' ',25);
                        memcpy(Header->ToField,Str,25);
                      }
                      newline();
                      RetVal = SAVEMSG;
                      goto exit;
            case 'K': {
                        /* don't let the user KILL the message if he is editing an EXISTING message */
                        bool WasEditMsg = EditMsg;  // need to save EditMsg because cleanup(), called from storemessage(), changes it
                        RetVal = (storemessage(Status.Conference,CLEANUP,CountMsg,FROMCALLER,0) == -1 ? ABORTMSG : (WasEditMsg ? SAVEMSG : KILLMSG));
                      }
                      goto exit;
            case 'N': RetVal = (storemessage(Status.Conference,CLEANUP,CountMsg,FROMCALLER,0) == -1 ? ABORTMSG : NEXTMSG);
                      goto exit;
          }
        } else {
          displaypcbtext(TXT_MSGABORTED,NEWLINE|LFBEFORE|LOGIT);
          RetVal = ABORTMSG;
          goto exit;
        }
      }
    }
  }

exit:
  UseAnsi = OldUseAnsi;
  Status.EnteringMessage = FALSE;
  #ifdef COMM
    Asy.IgnoreCDLoss = FALSE;
  #endif

  if (Logoff)
    loguseroff(ALOGOFF);

  if (Status.ScrollOn)
    reenablescroll();
  return(RetVal);
}


/********************************************************************
*
*  Function:  initmessage()
*
*  Desc    :  sets up the message variables used by the routines below
*
*  Returns :  0 if everything is okay, -1 if an error occured
*/

static int _NEAR_ LIBENTRY initmessage(void) {
  int      X;
  unsigned Bytes;

  checkstack();
  if (OldIndex || Status.CurConf.OldIndex) {
    if (checkfreemsgs() < 10) {
      displaypcbtext(TXT_NOROOMFORTEXT,NEWLINE|LOGIT);
      return(-1);
    }
  }

  #ifndef __OS2__
    long Free = coreleft();
    if (Free < MINFREEMEM) {
      sprintf(Status.DisplayText,"MSG ENTRY (%u/%ld)",MINFREEMEM,Free);
      displaypcbtext(TXT_INSUFFICIENTMEMORY,NEWLINE|LOGIT);
      return(-1);
    }
  #endif

  MaxMsgLines = (EditHdr ? 400 : PcbData.MaxMsgLines);
  Bytes = MaxMsgLines * sizeof(msgbuffer);

  #ifndef __OS2__
    if (Bytes > Free-MINFREEMEM) {
      MaxMsgLines = (int) ((Free-MINFREEMEM) / sizeof(msgbuffer));
      Bytes = (MaxMsgLines * sizeof(msgbuffer));
    }
  #endif

  // Add 128 to the number of bytes requested.  This will make room for 1
  // more message block in case an extended header requires padding be
  // added to the end of the last message block
  if ((Msg = (msgbuffer *) checkmalloc(Bytes+128,"MSG ENTRY")) == NULL)
    return(-1);

  if ((Header = (msgheadertype *) checkmalloc(sizeof(msgheadertype),"MSG ENTRY")) == NULL)
    return(-1);

  for (X = 28; X <= 254; X++)  /* set up translate table to allow ALL chars */
    CharXlat[X] = (char) X;

  CharXlat[127] = CTRL_G;  /* change ascii 127 to a CTRL_G */

  if (! PcbData.DisableFilter) {
    memset(&CharXlat[128],0,168-128+1);
    memset(&CharXlat[224],0,246-224+1);
    memset(&CharXlat[251],0,253-251+1);
  }

  MsgGrew = FALSE;
  LineNum = 0;
  QuoteStage = 0;
  ExtendedStatus = 0;
  NumExtHdrs = 0;
  Settings = (UsersData.PackedFlags.WideEditor ? Wide : Narrow);
  memset(Msg,0,Bytes);
  memset(Header,' ',sizeof(msgheadertype));
  memset(Header->RefNumber,0,sizeof(Header->RefNumber));
  memset(Header->ReplyDate,0,sizeof(bassngl));
  return(0);
}


/********************************************************************
*
*  Function:  getechoflag()
*
*  Desc    :  if the conference is being echoed it asks the users if this
*             message is to be echoed.
*/

static void _NEAR_ LIBENTRY getechoflag(void) {
  char Str[2];

  checkstack();
  if (Status.CurConf.EchoMail) {
    if (Status.CurConf.ForceEcho)
      Header->EchoFlag = 'E';
    else {
      Status.DisplayText[0] = Str[0] = YesChar;
      Status.DisplayText[1] = Str[1] = 0;
      inputfield(Str,TXT_ECHOMESSAGE,1,YESNO|UPCASE|FIELDLEN|NEWLINE,HLP_E,mask_yesno);
      if (Str[0] != NoChar)
        Header->EchoFlag = 'E';
    }
    if (Header->EchoFlag == 'E' && PcbData.Origin[0] != 0 && Status.CurConf.RecordOrigin)
      buildextheader(EXTHDR_ORIGIN,PcbData.Origin,0);
  }
}


/********************************************************************
*
*  Function:  getroutinginfo()
*
*  Desc    :  If the conference is being echoed and the message is private,
*             this will get the routing information from the caller.  If the
*             message being replied to had an ORIGIN stamp on it then the
*             default will be the ORIGIN of the replied-to message.
*
*             If Old is NULL then it is a brand new message that is being
*             entered so don't bother looking for ORIGIN.
*
*             If Old is != NULL then it is a reply.  Old is a pointer to the
*             replied-to message base.  Search the replied-to message to see
*             if there is an ORIGIN header and if so, use that information as
*             the default for routing the message back.
*/

static void _NEAR_ LIBENTRY getroutinginfo(msgbasetype *Old) {
  bool  Internet;
  char *p;
  char  RouteTo[EXTDESCLEN+1];

  checkstack();
  if (Header->EchoFlag != 'E' || ! Status.CurConf.PromptForRouting)
    return;

  Internet = (bool) (Status.CurConf.ConfType == 3 || Status.CurConf.ConfType == 4);

  // get routing info ONLY if:
  //   a) it's a PUBLIC message in an internet conference, or
  //   b) it's a PRIVATE message, but NOT in an internet conference

  if (Internet) {
    if (Header->Status == MSG_RCVR)
      return;
  } else {
    if (Header->Status != MSG_RCVR)
      return;
  }


  if (Old != NULL && (p = getextendedheader(EXTHDR_ORIGIN,Old->Body)) != NULL)
    maxstrcpy(RouteTo,p,sizeof(RouteTo));
  else {
    if (Internet) {
      if (Old != NULL && (p = getextendedheader(EXTHDR_ROUTE,Old->Body)) != NULL)
        maxstrcpy(RouteTo,p,sizeof(RouteTo));
      else
        maxstrcpy(RouteTo,PcbData.uucpDefDist,sizeof(RouteTo));
    } else
      RouteTo[0] = 0;
  }

  inputfield(RouteTo,TXT_ROUTETO,sizeof(RouteTo)-1,FIELDLEN|NEWLINE,HLP_E,mask_alphanum);
  if (RouteTo[0] != 0)
    buildextheader(EXTHDR_ROUTE,RouteTo,0);
}


static void _NEAR_ LIBENTRY getnewsgroup(msgbasetype *Old) {
  bool  Internet;
  char *p;
  char  NewsGroup[EXTDESCLEN+1];

  checkstack();
  Internet = (bool) (Status.CurConf.ConfType == 3 || Status.CurConf.ConfType == 4);
  if (Header->EchoFlag != 'E' || ! Internet || Header->Status == MSG_RCVR)
    return;

  if (Old != NULL && (p = getextendedheader(EXTHDR_UFOLLOW,Old->Body)) != NULL)
    maxstrcpy(NewsGroup,p,sizeof(NewsGroup));
  else
    maxstrcpy(NewsGroup,Status.CurConf.Name,sizeof(NewsGroup));

  inputfield(NewsGroup,TXT_DESTNEWSGROUP,sizeof(NewsGroup)-1,FIELDLEN|GUIDE|NEWLINE,NOHELP,mask_alphanum);
  if (NewsGroup[0] != 0)
    buildextheader(EXTHDR_UNEWSGR,NewsGroup,0);
}


static void _NEAR_ LIBENTRY getfollowup(void) {
  bool  Internet;
  char  Followup[EXTDESCLEN+1];

  checkstack();
  Internet = (bool) (Status.CurConf.ConfType == 3 || Status.CurConf.ConfType == 4);
  if (Header->EchoFlag != 'E' || ! Internet || Header->Status == MSG_RCVR)
    return;

  Followup[0] = 0;
  inputfield(Followup,TXT_FOLLOWUPNEWSGROUP,sizeof(Followup)-1,FIELDLEN|NEWLINE,NOHELP,mask_alphanum);
  if (Followup[0] != 0)
    buildextheader(EXTHDR_UFOLLOW,Followup,0);
}


static bool _NEAR_ LIBENTRY insufcreditsformsg(char MsgStatus, char EchoFlag) {
  double Charge;

  checkstack();
  if (Status.ActStatus != ACT_ENFORCE)
    return(FALSE);

  Charge = AccountRates.ChargeForMsgWrite;
  if (MsgStatus == MSG_RCVR || MsgStatus == MSG_CMNT)
    Charge = AccountRates.ChargeForMsgWritePrivate;
  else if (EchoFlag == 'E')
    Charge = AccountRates.ChargeForMsgWriteEchoed;

  Charge += Status.CurConf.ChargeMsgWrite;
  return(insufficientcredits(Charge,0));
}


/********************************************************************
*
*  Function:  entermessage()
*
*  Desc    :  sets up the editor for creating a new message
*/

#pragma warn -par
void LIBENTRY entermessage(int NumTokens) {
  bool ToAll;
  int  Len;
  char Save[26];
  char Subj[EXTDESCLEN+1];
  char To[256];

  checkstack();
  if (Status.CurConf.ReadOnly) {
    displaypcbtext(TXT_CONFISREADONLY,NEWLINE|LFBEFORE|BELL);
    return;
  }

  if (! seclevelokay("E",Status.CurConf.ReqLevelToEnter))
    return;

  if (initmessage() == -1)
    goto exit;

  To[0] = 0;
  if (NumTokens) {
    while (NumTokens) {
      strcat(To,getnexttoken());
      addchar(To,' ');
      NumTokens--;
    }
    To[25] = 0;          /* truncate it to fit inside the field */
    stripright(To,' ');
  }

  ToAll = gettoname(TXT_MSGTO,To);

  Subj[0] = 0;
  inputfield(Subj,TXT_MSGSUBJ,sizeof(Subj)-7,FIELDLEN|NEWLINE|LFBEFORE|HIGHASCII,NOHELP,mask_message);
  if (Status.CurSecLevel < PcbData.SysopSec[SEC_SUBS])
    removetokens(Subj);
  if (Subj[0] == 0 && ! Status.CurConf.LongToNames)
    goto exit;

  // ConfType 1 is e-mail, do not allow public messages
  if (Status.CurConf.PrivMsgs || Status.CurConf.ConfType == 1)
    Header->Status = MSG_RCVR;
  else if (Status.CurConf.NoPrivateMsgs)
    Header->Status = MSG_PBLC;
  else
    getsecurity(ToAll);

  if ((Header->Status == MSG_RCVR || Header->Status == MSG_CMNT) && ! ToAll)
    getretreceipt();

  strcpy(Save,To);
  if (findtoken(Save) == 0) {
    getechoflag();
    getroutinginfo(NULL);
    getnewsgroup(NULL);
    getfollowup();
  }

  if (insufcreditsformsg(Header->Status,Header->EchoFlag))
    goto exit;

  memcpy(Header->ToField,To,strlen(To));

  Len = strlen(Subj);
  if (Len <= 25)
    memcpy(Header->SubjField,Subj,strlen(Subj));
  else {
    memcpy(Header->SubjField,Subj,25);  // truncate main subj to 25 characters
    buildextheader(EXTHDR_SUBJECT,Subj,HDR_SUBJ);
  }

  MsgLeftNum = TXT_MESSAGELEFT;
  MsgSaveNum = TXT_SAVINGMESSAGE;

  msgeditor(NULL,0,TRUE);    //lint !e534

//#ifdef PCB152
//  if (RetVal == SAVEMSG)
//    chargeformessage(Header->Status,Header->EchoFlag == 'E',To);
//#endif

  usernetavailable();

exit:
  msgcleanup();
}
#pragma warn +par


/********************************************************************
*
*  Function:  entermessagefromfile()
*
*  Desc    :  sets up the editor for creating a new message
*/

void LIBENTRY entermessagefromfile(unsigned short ConfNum, char *ToName, char *FromName, char *Subject, char Security, unsigned short PackDate, bool ReqReceipt, bool Echo, char *FileName) {
  int  Len;
  char Subj[EXTDESCLEN+1];
  char To[256];

  checkstack();
  if (fileexist(FileName) == 255)
    return;

  getconfrecord(ConfNum,&Status.CurConf);
  if (initmessage() == -1)
    goto exit;

  if (ToName == NULL)
    maxstrcpy(To,Status.DisplayName,sizeof(To));
  else {
    stripright(To,' ');
    maxstrcpy(To,ToName,sizeof(To));
    if (strlen(To) <= 25) {
      if (! Status.CurConf.LongToNames)
        strupr(To);
    } else {
      buildextheader(EXTHDR_TO,To,HDR_TO);
      if (strlen(To) > EXTDESCLEN)
        buildextheader(EXTHDR_TO2,&To[EXTDESCLEN],HDR_TO);
      To[0] = 0;
    }
  }

  memcpy(Header->ToField,To,strlen(To));

  if (strcmp(To,"SYSOP") == 0)
    ToRecNum = 1;
  else
    ToRecNum = finduser(To);

  if (FromName == NULL)
    CreateReceipt = FALSE;   // use caller's name
  else {
    CreateReceipt = TRUE;    // allows us to override the FROM name
    strupr(FromName);
    memcpy(Header->FromField,FromName,strlen(FromName));
  }

  if (PackDate != 0)
    buildextheader(EXTHDR_PACKOUT,juliantodate(PackDate),0);

  maxstrcpy(Subj,Subject,sizeof(Subj));
  switch (Security) {
    default :
    case 'N': Header->Status = MSG_PBLC;
              break;
    case 'R': Header->Status = MSG_RCVR;
              break;
  }

  if (ReqReceipt)
    buildextheader(EXTHDR_REQRR,"Caller has requested a Return Receipt",HDR_RCPT);

  if (Echo)
    Header->EchoFlag = 'E';

  Len = strlen(Subj);
  if (Len <= 25)
    memcpy(Header->SubjField,Subj,strlen(Subj));
  else {
    memcpy(Header->SubjField,Subj,25);  // truncate main subj to 25 characters
    buildextheader(EXTHDR_SUBJECT,Subj,HDR_SUBJ);
    if (Len > EXTDESCLEN)
      buildextheader(EXTHDR_SUBJ2,Subj+EXTDESCLEN,0);
  }

  HideSaveMsg = TRUE;                  // avoids telling caller about new msg
  MsgLeftNum  = TXT_MESSAGELEFT;
  MsgSaveNum  = TXT_SAVINGMESSAGE;

  readfileintomsg(FileName,FALSE);  //lint !e534

  // NOTE: If the message is posted from PPL, the CountMsg parameter of the
  // storemessage() function is set to FALSE because we only want to show
  // new messages that are posted by people, not by programs

  if (packmessage())
    storemessage(ConfNum,CLEANUP,FALSE,FROMPPL,0);  //lint !e534

  usernetavailable();

exit:
  getconfrecord(Status.Conference,&Status.CurConf);
  msgcleanup();
}


/********************************************************************
*
*  Function:  entercomment()
*
*  Desc    :  sets up the editor for creating a new comment to the sysop
*
*  Notes   :  if NumTokens is -1 then we'll assume that we should go ahead
*             without asking if the caller wants to enter a comment.  Otherwise
*             NumTokens = 0 means ask the caller if he wants to leave a
*             comment and NumTokens = -2 means we're logging the caller off
*             from a bad password and asking if he wants to leave a comment
*             to the sysop first.
*/

#define WRONGPASSWORD          -2
#define LEAVEACOMMENTINSTEAD   -1


#pragma warn -par
void LIBENTRY entercomment(int NumTokens) {
  unsigned short SaveArea;
  int            RetVal;
  int            Len;
  char           Time[8];
  char           Str[80];
  pcbtexttype    Buf;
  pcbconftype    Conf;

  checkstack();
  SaveArea = 0;

  if (NumTokens != LEAVEACOMMENTINSTEAD) {
    Str[0] = NoChar;
    Str[1] = 0;
    inputfield(Str,(NumTokens == WRONGPASSWORD ? TXT_WRONGPWRDCMNT : TXT_LEAVECOMMENT),1,YESNO|FIELDLEN|UPCASE|NEWLINE|LFBEFORE,NOHELP,mask_yesno);
    if (Str[0] != YesChar)
      goto exit;
  }

  if ((NumTokens == WRONGPASSWORD || PcbData.ForceMain) && Status.Conference != 0) { /* only on a bad password or when "forcing main" */
    SaveArea = Status.Conference;                                            /* force him into the Main Board for the comment */
    Status.Conference = 0;
    getconfrecord(Status.Conference,&Status.CurConf);
    getdisplaynames();  // in case we need to change alias name to real name
//  if (Status.CurConf.NoPrivateMsgs) {
//    displaypcbtext(TXT_NOPRIVMSGS,NEWLINE|LFBEFORE|BELL);
//    goto exit;
//  }
  }

  ToRecNum = 1;
  if (initmessage() == -1)
    goto exit;

  if (insufcreditsformsg(MSG_CMNT,0))
    goto exit;

  memcpy(Header->ToField,"SYSOP",5);
  if (NumTokens == WRONGPASSWORD) {
    getpcbtext(TXT_WRONGPWRDSUBJECT,&Buf);
    Buf.Str[25] = 0;
    padstr(Buf.Str,' ',25);
    memcpy(Header->SubjField,Buf.Str,25);
  } else {
    strcpy(Str,"COMMENT");
    if (PcbData.Network)
      sprintf(&Str[7]," (%d)",PcbData.NodeNum);
    if (SaveArea != 0) {
      getconfrecord(SaveArea,&Conf);
      sprintf(&Str[strlen(Str)]," (%s)",Conf.Name);
    }

#ifdef PCB152
    Time[0] = ' ';
    timestr2(&Time[1]);
    strcat(Str,Time);
#endif

    Len = strlen(Str);
    if (Len > 25) {
      memcpy(Header->SubjField,Str,25);
      buildextheader(EXTHDR_SUBJECT,Str,HDR_SUBJ);
      if (Len > EXTDESCLEN)
        buildextheader(EXTHDR_SUBJ2,Str+EXTDESCLEN,0);
    } else
      memcpy(Header->SubjField,Str,strlen(Str));
    getretreceipt();
  }

  Header->Status = MSG_CMNT;

  MsgLeftNum = TXT_COMMENTLEFT;
  MsgSaveNum = TXT_SAVINGCOMMENT;

  RetVal = msgeditor(NULL,0,TRUE);

  if (RetVal == SAVEMSG) {
    UsersData.Stats.NumComments++;

//  #ifdef PCB152
//  if (RetVal == SAVEMSG)
//    chargeformessage(MSG_CMNT,FALSE,"SYSOP");
//  #endif
  }



exit:
  if (SaveArea != 0) {                /* put him back where he was */
    Status.Conference = SaveArea;
    getconfrecord(Status.Conference,&Status.CurConf);
    getdisplaynames();  // in case we need to change real name back to alias
  }

  usernetavailable();
  msgcleanup();
}
#pragma warn +par


/********************************************************************
*
*  Function:  enterreply()
*
*  Desc    :  sets up the editor for creating a reply to a message
*
*  Returns :  ABORTMSG, SAVEMSG, NEXTMSG or KILLMSG depending on what the
*             user chose to do.
*/

savetype LIBENTRY enterreply(msgbasetype *Old, msgstattype MsgStatus, readaccesstype Access, bool AskOther) {
  char static Types[GPWD+1] = {PBLC,RCVR,CMNT,PBLC,GPWD};
  bool ToAll;
  savetype Save;
  int  Len;
  char *p;
  char Subj[(EXTDESCLEN*2)+1];
  char To[128];

  checkstack();
  if (Status.CurConf.ReadOnly) {
    displaypcbtext(TXT_CONFISREADONLY,NEWLINE|LFBEFORE|BELL);
    return(ABORTMSG);
  }

  if (! seclevelokay("REPLY",Status.CurConf.ReqLevelToEnter))
    return(ABORTMSG);

  if (initmessage() == -1) {
    msgcleanup();
    return(ABORTMSG);
  }

  // If the message has a long FROM, and the short FROM matches your name,
  // the long header may not.  Therefore, we do not want to address the reply
  // to the TO name.  Example:
  //
  //    TO:  DAVID TERRY
  //  FROM:  scott.carpenter@clarkdev.com, SYSOP
  //
  // If David Terry is the sysop of the system on which this message is being
  // read, and he REPLIES to the above, it should reply to scott.carpenter, not
  // to DAVID TERRY.

  if (Access.Msg.FromYou && (Old->Header.ExtendedStatus & HDR_FROM) && getextendedheader(EXTHDR_FROM,Old->Body) != NULL)
    Access.Msg.FromYou = FALSE;

  if (Access.Msg.FromYou || AskOther) {
    memcpy(Header->ToField,Old->Header.ToField,25);
    memcpy(To,Header->ToField,25);
    To[25] = 0;
    if (Old->Header.ExtendedStatus & HDR_TO && (p = getextendedheader(EXTHDR_TO,Old->Body)) != NULL) {
      if (AskOther) {
        maxstrcpy(To,p,sizeof(To));
        if (strlen(To) > 60)  // the maximum extended header length is 60 but it's not null terminated
          To[60] = 0;         // null terminate it now if necessary
      } else {
        buildextheader(EXTHDR_TO,p,HDR_TO);
        if ((p = getextendedheader(EXTHDR_TO2,Old->Body)) != NULL)
          buildextheader(EXTHDR_TO2,p,HDR_TO);
      }
    }
  } else {
    memcpy(Header->ToField,Old->Header.FromField,25);
    memcpy(To,Header->ToField,25);
    To[25] = 0;
    if (Old->Header.ExtendedStatus & HDR_FROM && (p = getextendedheader(EXTHDR_FROM,Old->Body)) != NULL) {
      if (memcmp(Old->Header.FromField,"RETURN RECEIPT           ",25) == 0)
        memcpy(Header->ToField,p,25);
      else {
        buildextheader(EXTHDR_TO,p,HDR_TO);
        if ((p = getextendedheader(EXTHDR_FROM2,Old->Body)) != NULL)
          buildextheader(EXTHDR_TO2,p,HDR_TO);
      }
    }
  }

  memcpy(Header->RefNumber,Old->Header.MsgNumber,sizeof(bassngl));

  if (Old->Header.ExtendedStatus & HDR_SUBJ && (p = getextendedheader(EXTHDR_SUBJECT,Old->Body)) != NULL) {
    maxstrcpy(Subj,p,sizeof(Subj));
    Subj[EXTDESCLEN] = 0;
    if ((p = getextendedheader(EXTHDR_SUBJ2,Old->Body)) != NULL) {
      memcpy(Subj+EXTDESCLEN,p,EXTDESCLEN);
      Subj[EXTDESCLEN*2] = 0;
    }
  } else {
    memcpy(Subj,Old->Header.SubjField,25);
    Subj[25] = 0;
  }
  stripright(Subj,' ');

/*
  Man, I don't think this belongs here....  PROVE ME WRONG!

  The reason I don't think it belongs is that if you enter a reply to a message
  that has a long TO name, you end up sending the message TO the same person
  as the TO name instead of to the FROM name.  WRONG!

  if (Old->Header.ExtendedStatus & HDR_TO && (p = getextendedheader(EXTHDR_TO,Old->Body)) != NULL) {
    buildextheader(EXTHDR_TO,p,HDR_TO);
    if ((p = getextendedheader(EXTHDR_TO2,Old->Body)) != NULL)
      buildextheader(EXTHDR_TO2,p,HDR_TO);
  }
*/

  stripright(To,' ');
  ToAll = (bool) (strcmp(To,"ALL") == 0);

  if (AskOther) {
    ToAll = gettoname(TXT_MSGTO,To);
    padstr(To,' ',25);
    memcpy(Header->ToField,To,25);
  } else {
    if (strcmp(To,"SYSOP") == 0)
      ToRecNum = 1;
    else
      ToRecNum = finduser(Header->ToField);
  }

  if ((! Status.CurConf.LongToNames) || AskOther) {
    Len = strlen(Subj);
    if (Len > EXTDESCLEN)
      Len = EXTDESCLEN*2;
    else
      Len = EXTDESCLEN;
    inputfield(Subj,TXT_NEWSUBJECT,Len,FIELDLEN|GUIDE|NEWLINE|LFBEFORE|HIGHASCII,NOHELP,mask_alphanum);
  }

  if (Status.CurSecLevel < PcbData.SysopSec[SEC_SUBS])
    removetokens(Subj);

  if (Subj[0] != 0) {
    Len = strlen(Subj);
    if (Len <= 25)
      memcpy(Header->SubjField,Subj,strlen(Subj));
    else {
      memcpy(Header->SubjField,Subj,25);  // truncate main subj to 25 characters
      buildextheader(EXTHDR_SUBJECT,Subj,HDR_SUBJ);
      if (Len > EXTDESCLEN)
        buildextheader(EXTHDR_SUBJ2,Subj+EXTDESCLEN,0);
    }
  } else {
    memcpy(Header->SubjField,Old->Header.SubjField,25);
    if (Old->Header.ExtendedStatus & HDR_SUBJ && (p = getextendedheader(EXTHDR_SUBJECT,Old->Body)) != NULL) {
      buildextheader(EXTHDR_SUBJECT,p,HDR_SUBJ);
      if ((p = getextendedheader(EXTHDR_SUBJ2,Old->Body)) != NULL)
        buildextheader(EXTHDR_SUBJ2,p,0);
    }
  }


  MsgStatus.Read = FALSE;

  if (AskOther && ! (Status.CurConf.PrivMsgs || Status.CurConf.NoPrivateMsgs)) {
    getsecurity(ToAll);
  } else {
    MsgStatus.Type = Types[MsgStatus.Type];
    if (MsgStatus.Type == GPWD) {
      memcpy(Header->Password,Old->Header.Password,sizeof(Old->Header.Password));
      #pragma warn -stv
      Header->Status = getnewmsgstatus(MsgStatus);
      #pragma warn +stv
    } else if (Status.CurConf.PrivMsgs || Status.CurConf.ConfType == 1)  {
      // ConfType 1 is e-mail, do not allow public messages
      MsgStatus.Type = RCVR;
      #pragma warn -stv
      Header->Status = getnewmsgstatus(MsgStatus);
      #pragma warn +stv
    } else if (MsgStatus.Type == PBLC && ! Status.CurConf.NoPrivateMsgs) {
      getsecurity(FALSE);
    } else {
      #pragma warn -stv
      Header->Status = getnewmsgstatus(MsgStatus);
      #pragma warn +stv
    }
  }

  getechoflag();
  getroutinginfo(Old);
  getnewsgroup(Old);
  getfollowup();

  Save = ABORTMSG;

  if (insufcreditsformsg(Header->Status,Header->EchoFlag))
    goto exit;

  if (Header->Status == MSG_RCVR || Header->Status == MSG_CMNT)
    getretreceipt();

  MsgLeftNum = TXT_MESSAGELEFT;
  MsgSaveNum = TXT_SAVINGMESSAGE;

  Save = msgeditor(Old,0,TRUE);

//#ifdef PCB152
//  if (Save == SAVEMSG)
//    chargeformessage(Header->Status,Header->EchoFlag == 'E',To);
//#endif

exit:
  usernetavailable();

  msgcleanup();
  return(Save);
}


/********************************************************************
*
*  Function:  createreceipt()
*
*  Desc    :  creates a return receipt message
*
*/

void LIBENTRY createreceipt(msgbasetype *Old) {
  msgstattype MsgStatus;
  char        *p;
  char        TimeStr[6];
  char        DateStr[9];
  char        Str[80];

  checkstack();
  if (initmessage() == -1) {
    msgcleanup();
    return;
  }

  CreateReceipt = TRUE;
  HideSaveMsg   = TRUE;
  memcpy(Header->ToField,Old->Header.FromField,25);
  memcpy(Header->FromField,"RETURN RECEIPT           ",25);
  memcpy(Header->RefNumber,Old->Header.MsgNumber,sizeof(bassngl));
  memcpy(Header->SubjField,Old->Header.SubjField,25);

  if (memcmp(Header->ToField,SysopName,25) == 0)
    ToRecNum = 1;
  else
    ToRecNum = finduser(Header->ToField);

  MsgStatus.Read = FALSE;
  MsgStatus.Type = RCVR;
  Header->Status = getnewmsgstatus(MsgStatus);
  MsgLeftNum     = TXT_RECEIPTLEFT;
  MsgSaveNum     = TXT_SAVINGMESSAGE;

  Header->EchoFlag = Old->Header.EchoFlag;

  datestr(DateStr);
  timestr2(TimeStr);

  sprintf(Str,"Acknowledge Receipt of msg %ld written on %s %s",
          bassngltolong(Old->Header.MsgNumber),
          DateStr,
          TimeStr);

  buildextheader(EXTHDR_ACKRR,Str,HDR_RCPT);
  strcpy(Str,Status.DisplayName);
  padstr(Str,' ',25);
  buildextheader(EXTHDR_FROM,Str,HDR_FROM);

  if ((p = getextendedheader(EXTHDR_ORIGIN,Old->Body)) != NULL)
    buildextheader(EXTHDR_ROUTE,p,0);

  if ((p = getextendedheader(EXTHDR_SUBJECT,Old->Body)) != NULL) {
    buildextheader(EXTHDR_SUBJECT,p,HDR_SUBJ);
    if ((p = getextendedheader(EXTHDR_SUBJ2,Old->Body)) != NULL)
      buildextheader(EXTHDR_SUBJ2,p,0);
  }

  if (packmessage())
    storemessage(Status.Conference,CLEANUP,FALSE,FROMCALLER,0);  //lint !e534

  usernetavailable();
  msgcleanup();
}


/********************************************************************
*
*  Function:  editmessage()
*
*  Desc    :  sets up the editor for editing an existing message
*/

savetype LIBENTRY editmessage(msgbasetype *Old, long MsgNum, long Offset) {
  unsigned Line;
  unsigned Bytes;
  unsigned Len;
  char     *p;
  char     *q;
  savetype Save;

  checkstack();
  if (initmessage() == -1) {
    msgcleanup();
    return(ABORTMSG);
  }

  EditMsg        = TRUE;
  OldNumBlocks   = Old->Header.NumBlocks;
  OldOffset      = Offset;
  OldMsgNumber   = MsgNum;
  MsgLeftNum     = TXT_MESSAGELEFT;
  MsgSaveNum     = TXT_SAVINGMESSAGE;
  ExtendedStatus = Old->Header.ExtendedStatus;

  memcpy(Header,&Old->Header,sizeof(msgheadertype));

  p = loadextendedheaders(Old->Body);
  Bytes = (Old->Header.NumBlocks-1)*128 - ((int) (p - Old->Body));

  Line = 0;
  while ((q = strnchr(p,LineSeparator,Bytes)) != NULL) {
    change(p,27,29);     /* change ESC codes to ASCII 29's */

    // only process and count lines that do NOT start with CTRL-A (used by FIDO)
    if (*p != 1) {
      if (Line < (unsigned) MaxMsgLines && *p != 1) {
        *q = 0;
        if ((Len = strlen(p)) > (unsigned) Settings.RightMargin) {
          if (Settings.ShowLineNums) {
            Settings = Wide;
            if (Len > (unsigned) Settings.RightMargin)
              Len = Settings.RightMargin;
          } else Len = Settings.RightMargin;
        }
        memcpy(Msg[Line].Line,p,Len);
        Msg[Line].Line[Len] = 0;
        *q = LineSeparator;
      }
      Line++;
    }
    if ((Bytes -= ((int) (q-p+1))) <= 0)
      break;
    p = q + 1;
  }
  change(p,27,29);     /* change ESC codes to ASCII 29's */

  Save = msgeditor(Old,Line-1,FALSE);
  if (Save != ABORTMSG && MsgGrew) {  /* if the user saved the message   */
    Save = KILLMSG;                   /* but it didn't fit in the old    */
  }                                   /* location, then kill the old one */


  usernetavailable();

  msgcleanup();
  return(Save);
}


/********************************************************************
*
*  Function:  modifyextheader()
*
*  Desc    :  Sets up the editor and then simply updates an existing extended
*             header or adds a new one and then saves the message back out.
*
*  Note    :  If an extended header is ADDED it may cause the message body to
*             grow too large to fit in the current block size.  If that occurs
*             then the message will be moved to the end of the message base.
*/

savetype LIBENTRY modifyextheader(msgbasetype *Old, long MsgNum, long Offset, exthdrtype Which, char *Desc, char Mask) {
  unsigned Bytes;
  unsigned MaxBytes;
  char     *p;
  savetype Save;

  checkstack();
  EditHdr = TRUE;

  if (initmessage() == -1) {
    msgcleanup();
    return(ABORTMSG);
  }

  EditMsg        = TRUE;
  HideSaveMsg    = TRUE;
  OldNumBlocks   = Old->Header.NumBlocks;
  OldOffset      = Offset;
  OldMsgNumber   = MsgNum;
  MsgLeftNum     = TXT_MESSAGELEFT;
  MsgSaveNum     = TXT_SAVINGMESSAGE;
  ExtendedStatus = Old->Header.ExtendedStatus;

  memcpy(Header,&Old->Header,sizeof(msgheadertype));

  p        = loadextendedheaders(Old->Body);
  Bytes    = (Old->Header.NumBlocks-1)*128 - ((int) (p - Old->Body));
  MaxBytes = (MaxMsgLines * sizeof(msgbuffer)) - 1;
  Bytes    = min(Bytes,MaxBytes);

  // copy the original message into the editor workspace
  // used to use Msg[0].Line, but that confuses memcheck which thinks that the
  // destination is only 82 bytes in size, so changed to (char*) Msg.
  memcpy((char *)Msg,p,Bytes);

  // get rid of any imbedded NULL terminators
  while ((p = strnchr(Msg[0].Line,0,Bytes)) != NULL)
    *p = ' ';

  // put a NULL terminator at the very end of the message
  p = Msg[0].Line;
  p[Bytes] = 0;

  // remove spaces (padding) from the end of the message
  stripright(p,' ');

  // FINALLY!  This line separator signals the end of the message.  If we
  // don't strip it out then ANOTHER ONE will be added on making the new
  // message one "display line" longer than the original.
  stripright(p,LineSeparator);

  // If it is a TO, FROM or SUBJECT header then it's a special case where
  // the main header can hold up to 25 characters.  So if the description is
  // 25 characters or less, we can REMOVE the extended description - if there
  // is one.  If the description is longer than 25 characters, or if it is not
  // a TO, FROM or SUBJECT header, then go ahead and modify the existing
  // header or add a new one if one doesn't already exist.

  if (Which == EXTHDR_SUBJECT || Which == EXTHDR_TO || Which == EXTHDR_FROM || Which == EXTHDR_ATTACH) {
    if (strlen(Desc) <= 25) {
      removeexthdr(Which,Mask);
      if (Which == EXTHDR_TO)
        removeexthdr(EXTHDR_TO2,Mask);
      else if (Which == EXTHDR_FROM)
        removeexthdr(EXTHDR_FROM2,Mask);
    } else {
      if ((p = checkextendedheaders(Which,NULL)) != NULL)
        memcpy(p,Desc,EXTDESCLEN);
      else
        buildextheader(Which,Desc,Mask);
      }
  } else {
    if (Which == EXTHDR_LIST) {
      if ((p = checkextendedheaders(Which,Status.DisplayName)) != NULL)
        memcpy(p,Desc,EXTDESCLEN);
    } else {
      if ((p = checkextendedheaders(Which,NULL)) != NULL)
        memcpy(p,Desc,EXTDESCLEN);
      else
        buildextheader(Which,Desc,Mask);
    }
  }

  Save = ABORTMSG;
  if (packmessage()) {
    Save = (storemessage(Status.Conference,CLEANUP,FALSE,FROMCALLER,Old->Header.NetTag) == -1 ? ABORTMSG : SAVEMSG);
    if (Save != ABORTMSG && MsgGrew) {  // if the message grew and no longer
      Save = KILLMSG;                   // fits in the old location then
    }                                   // then kill the old one
  }

  usernetavailable();

  msgcleanup();
  return(Save);
}


/********************************************************************
*
*  Function:  savemessage
*
*  Desc    :  Takes an existing message Header and Body and saves a new
*             message with the same header and body.
*
*  NOTES   :  If the ConfNum is different from the current conference number
*             then the Replies flag is reset as well as the Refer number.
*
*             Also, if the existing message is set to be echoed but the target
*             conference is not echoed then the echo flag is reset.
*
*             The storemessage() function is called which assumes that *Header
*             and *Msg have been initialized - in this case we will simply
*             point *Header to *MsgHeader and *Msg to *MsgBody to avoid having
*             to allocate and the deallocate memory for this function.
*/

int LIBENTRY savemessage(unsigned short ConfNum, msgheadertype *MsgHeader, char *MsgBody, bool Copying) {
  int           RetVal;
  int           NumTokens;
  char         *p;
  long          Size;
  msgstattype   Stat;
  char          To[26];
  char          OldPath[66];
  char          NewPath[66];
  char          ViewName[66];
  char          FileName[66];
  char          Temp[80];
  msgheadertype OldHeader;
  pcbconftype   Old;

  #ifdef DEBUG2
    #ifdef __BORLANDC__
      if (heapcheck() != _HEAPOK) {
        writedebugrecord("ERROR: msgenter() HEAP CORRUPT");
        return(-1);
      }
    #endif
  #endif

  checkstack();

  /*
  NOTE:  ConfNum *may* be pointing to a conference other than the current
  conference pointed to by Status.CurConf - e.g. when MOVING or COPYING a
  message to another conference.  For this reason - we will save the current
  conference information and get new information
  */
  Old = Status.CurConf;

  /* WARNING WARNING WARNING!!!!!!                                           */
  /* The next two lines below "borrow" the pointers for the Header and Body  */
  /* of already allocated buffers of memory.  The msgcleanup() function      */
  /* will FREE THESE BUFFERS if we don't put them back to NULL before        */
  /* exiting this function.  We must therefore be sure to set these pointers */
  /* to NULL at the bottom of this funcion!                                  */

  Header         = MsgHeader;
  Msg            = (msgbuffer *) MsgBody;


  MsgLeftNum     = (Copying ? TXT_MESSAGECOPIED : TXT_MESSAGEMOVED);
  MsgSaveNum     = TXT_SAVINGMESSAGE;
  MovedNum       = bassngltolong(Header->MsgNumber);
  OldHeader      = *MsgHeader;
  ExtendedStatus = MsgHeader->ExtendedStatus;
  p              = NULL;

  getconfrecord(ConfNum,&Status.CurConf);

  if ((OldIndex || Status.CurConf.OldIndex) && checkfreemsgs() < 10) {
    displaypcbtext(TXT_NOROOMFORTEXT,NEWLINE|LOGIT);
    RetVal = -1;
  } else {
    memset(Header->RefNumber,0,sizeof(bassngl));
  /*memset(Header->ReplyDate,0,sizeof(bassngl));*/
  /*memcpy(Header->ReplyTime,"00:00",5);*/

    Stat = getmsgstatus(Header->Status);
    Stat.Read = FALSE;
    if (Status.CurConf.PrivMsgs || Status.CurConf.ConfType == 1)
      Stat.Type = RCVR;
    else if (Status.CurConf.NoPrivateMsgs)
      Stat.Type = PBLC;

    #pragma warn -stv
    Header->Status = getnewmsgstatus(Stat);
    #pragma warn +stv
    Header->ReplyStatus = ' ';

    Header->EchoFlag = (Status.CurConf.EchoMail ? 'E' : ' ');

    memcpy(To,Header->ToField,25);
    To[25] = 0;
    stripright(To,' ');
    if (strcmp(To,"SYSOP") == 0)
      ToRecNum = 1;
    else
      ToRecNum = finduser(To);

    // check first for an embedded ATTACH header inside the message body as
    //    that is where it will be found if the message is being MOVED or
    //    COPIED to a new message
    // after that, check for a "loaded" ATTACH header in the ExtHdr array as
    //    that is where it will be found if the message is being FORWARDED

    if ((p = getextendedheader(EXTHDR_ATTACH,MsgBody)) != NULL ||
        (p = checkextendedheaders(EXTHDR_ATTACH,NULL)) != NULL) {
      maxstrcpy(Temp,p,sizeof(Temp));
      NumTokens = tokenizestr(Temp);

      if (NumTokens >= 3) {
        maxstrcpy(ViewName,getnexttoken(),sizeof(ViewName)); // get VIEW name
        Size = atol(getnexttoken()+1);                       // get file SIZE
        maxstrcpy(FileName,getnexttoken(),sizeof(FileName)); // get STORED name
        buildstr(OldPath,Old.AttachLoc,FileName,NULL);
        buildstr(NewPath,Status.CurConf.AttachLoc,ViewName,NULL);
        if (DebugLevel > 3) {
          writelog(OldPath,SPACERIGHT);
          writelog(NewPath,SPACERIGHT);
        }

        // copy STORED name to VIEW name
        if (pcbcopyfile(OldPath,NewPath,FALSE,FALSE,NULL,TRUE) == 0) {
          // play switcheroo with the variable names so the source looks good
          strcpy(OldPath,NewPath);

          // now we're renaming the VIEW file to a unique name, and that new
          // name will be found in NewPath
          makefilenameunique(NewPath,OldPath);

          // now replace the original header with a new one
          sprintf(Temp,"%s (%ld) %s",findstartofname(OldPath),Size,findstartofname(NewPath));
          memcpy(p,Temp,strlen(Temp));
        }
      }
    }

    /* we didn't allocate the memory being used so we must use the NOCLEANUP */
    /* parameter to avoid having the storemessage() function free up memory  */
    /* that belongs to the routine that call us                              */

    RetVal = storemessage(ConfNum,NOCLEANUP,FALSE,FROMCALLER,0);
  }

  *MsgHeader = OldHeader;
  Status.CurConf = Old;


  /* WARNING!  See comment above to understand why these two pointers */
  /* must be set back to NULL before returning from this function     */

  Header = NULL;
  Msg    = NULL;

  return(RetVal);
}



static void _NEAR_ LIBENTRY getqwkattached(void) {
  char *p;
  long  Size;
  char  Str[EXTDESCLEN+1];
  char  RootName[13];
  char  AttachedName[66];
  char  StoredName[66];
  char  UniqueName[66];

  checkstack();

  // check for an ATTACH extended header
  if ((p = checkextendedheaders(EXTHDR_ATTACH,NULL)) != NULL) {

    // copy the header into Str where we can tokenize it
    maxstrcpy(Str,p,EXTDESCLEN);

    // make sure there is at least the first token - the name of the file
    if (tokenizestr(Str) >= 1) {

      // get the attached name from the .REP packet and point to it in the WORK directory
      maxstrcpy(RootName,getnexttoken(),sizeof(RootName));
      buildstr(AttachedName,PcbData.TmpLoc,RootName,NULL);

      // get rid of the existing header, we'll either build a new one (if
      // the file really exists) or we'll throw it away (if it attached
      // file was not found inside of the REP packet).
      removeexthdr(EXTHDR_ATTACH,HDR_FILE);

      buildstr(StoredName,Status.CurConf.AttachLoc,RootName,NULL);

      // check to see if the file was really included in the .REP packet
      if (fileexist(AttachedName) == 255) {
        // the AttachedName (first parameter) was not found - did the author
        // screw up and insert the attachment by the 3rd parameter name
        // instead?

        getnexttoken();  // skip over size
        buildstr(AttachedName,PcbData.TmpLoc,getnexttoken(),NULL);
        if (fileexist(AttachedName) == 255) {
          writelog("ERROR: FILE ATTACHMENT NOT INCLUDED",SPACERIGHT);
          return;
        }
      }


      Size = DTA.ff_fsize;
      if (pcbcopyfile(AttachedName,StoredName,FALSE,FALSE,NULL,TRUE) == 0) {
        // now we're renaming the VIEW file to a unique name, and that new
        // name will be found in NewPath
        makefilenameunique(UniqueName,StoredName);
        // now replace the original header with a new one
        sprintf(Str,"%s (%ld) %s",RootName,Size,findstartofname(UniqueName));
        buildextheader(EXTHDR_ATTACH,Str,HDR_FILE);
        strcpy(Status.DisplayText,RootName);
        logsystext(TXT_ATTACHMENT,SPACERIGHT);
      }
      // get rid of the original out of the work directory
      unlink(AttachedName);
    }
  }
}


/********************************************************************
*
*  Function:  postqwkmessage()
*
*  Desc    :
*/


static int _NEAR_ LIBENTRY postqwkmessage(qwkmsgtype *QWKHeader, char *QWKBody, unsigned short Size, bool SetLMRs) {
  bool           NetStatus;
  unsigned short ConfNum;
  unsigned short Bytes;
  unsigned short MaxBytes;
  int            TxtNum;
  int            RetVal;
  char          *p;
  long           RefNum;
  char           Alias[26];
  char           Temp[80];
  char           To[80];
  pcbconftype    Old;

  checkstack();
  if (UsersData.Alias[0] == 0)
    Alias[0] = 0;
  else
    moveback(Alias,UsersData.Alias,sizeof(UsersRead.Name));

  NetStatus = FALSE;

  memcpy(Temp,QWKHeader->Number,7);
  Temp[7] = 0;
  ConfNum = (unsigned short) atoi(Temp);

  // if the QWK/Net PSA is installed, and if the caller has Net Status in
  // this conference, then allow all messages to be uploaded.
  if (QwkSupport && isset(&ConfReg[NET],ConfNum)) {
    NetStatus = TRUE;
  } else {
    // otherwise, if this message isn't a personal message from the
    // user, and it's not from a user named "NEW USER", then reject it.
    if (! (memicmp(QWKHeader->FromName,UsersRead.Name,25) == 0 ||
           memicmp(QWKHeader->FromName,"NEW USER                 ",25) == 0 ||
           (Status.User == SYSOP && memicmp(QWKHeader->FromName,SysopName,25) == 0) ||
           (Alias[0] != 0 && memicmp(QWKHeader->FromName,Alias,25) == 0))) {
      maxstrcpy(Status.DisplayText,QWKHeader->FromName,26);
      displaypcbtext(TXT_ERRORINFROMNAME,NEWLINE|LOGIT|BELL);
      return(0);
    }
  }

  if (ConfNum != Status.Conference && ! isregistered(ConfReg,ConfNum))
    return(0);

  Old = Status.CurConf;
  getconfrecord(ConfNum,&Status.CurConf);
  RetVal = 0;

  getdisplaynames();

  if (initmessage() == -1)
    goto exit;

  memcpy(To,QWKHeader->ToName,25);
  To[25] = 0;
  strupr(To);

  if (memicmp(To,SysopName,25) == 0)
    ToRecNum = 1;
  else
    ToRecNum = finduser(To);

  if (Status.CurSecLevel < PcbData.SysopSec[SEC_GENERICMSGS]) {
    removetokens(To);
    if (To[0] == 0 || To[0] == '@') {
      strcpy(To,"ALL");
      moveback(QWKHeader->ToName,To,25);
    }
  }

  memcpy(Header->ToField,QWKHeader->ToName,25);
  memcpy(Header->SubjField,QWKHeader->Subject,25);
  memcpy(Header->Password,QWKHeader->Password,12);

  printdefcolor();
  sprintf(Temp,"(%u) %-25.25s %-25.25s  ",ConfNum,QWKHeader->ToName,QWKHeader->Subject);
  print(Temp);

  if (Status.CurConf.PrivMsgs || Status.CurConf.ConfType == 1)
    Header->Status = MSG_RCVR;
  else if (Status.CurConf.NoPrivateMsgs)
    Header->Status = MSG_PBLC;
  else
    Header->Status = QWKHeader->Status;

  switch (Header->Status) {
    case MSG_CMNT     :
    case MSG_CMNT_READ:
    case MSG_RCVR     :
    case MSG_RCVR_READ:  TxtNum = TXT_MSGRCVRONLY;
                         break;
    case MSG_PBLC     :
    case MSG_PBLC_READ:  TxtNum = TXT_MSGPUBLIC;
                         break;
    case MSG_SPWD     :
    case MSG_SPWD_READ:  TxtNum = TXT_MSGSENDERPWRD;
                         break;
    case MSG_GPWD     :
    case MSG_GPWD_ALL :
    case MSG_GPWD_READ:  TxtNum = TXT_MSGGROUPPWRD;
                         break;
    // if a valid status byte was not found, then default it to receiver only
    default           :  Header->Status = MSG_RCVR;
                         TxtNum = TXT_MSGRCVRONLY;
                         break;
  }
  displaypcbtext(TxtNum,LFAFTER);

  if (Status.CurConf.ReadOnly) {
    displaypcbtext(TXT_CONFISREADONLY,NEWLINE|LFBEFORE|BELL);
    goto exit;
  }

  if (! (seclevelokay("E",Status.CurConf.ReqLevelToEnter) && seclevelokay("E",PcbData.UserLevels[SEC_E])))
    goto exit;

  if (insufcreditsformsg(Header->Status,Header->EchoFlag))
    goto exit;

  RefNum = atol(QWKHeader->ReferenceNum);
  longtobassngl(Header->RefNumber,RefNum);

  Header->NumBlocks = (char) (Size / 128);

  memcpy(Header->Date,QWKHeader->Date,8);
  memcpy(Header->Time,QWKHeader->Time,5);

  longtobassngl(Header->ReplyDate,0L);
  memset(Header->ReplyTime,0,5);
  Header->ReplyStatus=' ';

  // if it's an echo mail conference, and if the subject of the message
  // does not start out with "NE:", then set the ECHO FLAG on.
  if (Status.CurConf.EchoMail && memcmp(Header->SubjField,"NE:",3) != 0)
    Header->EchoFlag = 'E';

  MsgLeftNum = TXT_MESSAGELEFT;
  MsgSaveNum = TXT_SAVINGMESSAGE;

  p = loadextendedheaders(QWKBody);
  Bytes = (unsigned short) (Size - ((unsigned short) (p - QWKBody)));
  MaxBytes = (unsigned short) (MaxMsgLines * (sizeof(msgbuffer)-1));  // the -1 is to guarantee some "fluff" in the workspace
  if (Bytes > MaxBytes)
    Bytes = MaxBytes;

  // copy the original message into the editor workspace
  // used to use Msg[0].Line, but that confuses memcheck which thinks that the
  // destination is only 82 bytes in size, so changed to (char*) Msg.
  memcpy((char *)Msg,p,Bytes);

  // put a NULL terminator at the very end of the message
  p = Msg[0].Line;
  p[Bytes] = 0;

  // remove spaces (padding) from the end of the message
  stripright(p,' ');

  // FINALLY!  This line separator signals the end of the message.  If we
  // don't strip it out then ANOTHER ONE will be added on making the new
  // message one "display line" longer than the original.
  stripright(p,LineSeparator);

  getqwkattached();

  if (packmessage())
    if (NetStatus) {
      // override the FROM name - packmessage() put the current caller's name
      // in the header, but since this caller has Net Status, the message
      // might not be from him, so copy in the *real* FROM name
      memcpy(Header->FromField,QWKHeader->FromName,25);
    }
    if (storemessage(ConfNum,CLEANUP,TRUE,FROMCALLER,Header->NetTag) == 0) {
      RetVal = 1;
      if (RefNum != 0)
        setreplystatus(RefNum);

      // if SetLMRs is set, then update the LMR pointer with msg # just posted
      if (SetLMRs)
        MsgReadPtr[ConfNum] = Status.LastMsgNum;

//    #ifdef PCB152
//      chargeformessage(Header->Status,Header->EchoFlag == 'E',To);
//    #endif
    }

exit:
  Status.CurConf = Old;
  getdisplaynames();
  msgcleanup();
  return(RetVal);
}


/********************************************************************
*
*  Function:  readqwkreplies()
*
*  Desc    :
*/

void LIBENTRY readqwkreplies(char *Msgs, bool SetLMRs) {
  unsigned short Size;
  unsigned       NumStored;
  char          *Body;
  char           Temp[7];
  DOSFILE        File;
  qwkmsgtype     QWKHeader;
  pcbconftype    Old;

  checkstack();
  NumStored = 0;
  if (dosfopen(Msgs,OPEN_READ|OPEN_DENYRDWR,&File) == -1)
    goto end;

  Old = Status.CurConf;

  dosfseek(&File,QWK_SIZE,SEEK_SET);   // skip over main header
  Body = NULL;
  Temp[6] = 0;

  startdisplay(FORCENONSTOP);

  while (dosfread(&QWKHeader,QWK_SIZE,&File) == QWK_SIZE) {
    memcpy(Temp,QWKHeader.NumBlocks,6);
    Size = (unsigned short) ((atoi(Temp)-1)*128);
    if ((Body = (char *) bmalloc(Size)) != NULL) {
      if (dosfread(Body,Size,&File) == Size)
        NumStored += postqwkmessage(&QWKHeader,Body,Size,SetLMRs);
      bfree(Body);
    }
  }

  dosfclose(&File);
  unlink(Msgs);
  Status.CurConf = Old;

end:
  displaypcbtext(NumStored != 0 ? TXT_REPSUCCESSFUL : TXT_REPFAILED,NEWLINE|LFBEFORE|LOGIT);
  usernetavailable();
}


#ifdef PCB152
void LIBENTRY forwardmessage(unsigned short MoveConfNum, msgbasetype *MsgBase) {
  char            SaveStatus;
  unsigned short  BodyBytes;
  unsigned short  OrgBytes;
  unsigned short  NewBytes;
  unsigned short  AllocSize;
  unsigned        X;
  unsigned short  SaveConfNum;
  int             OffsetIntoBody;
  char           *p;
  char           *Temp;
  char            Buf[80];
  char            To[(EXTDESCLEN*2)+1];
  char            Str[(EXTDESCLEN*2)+1];
  pcbconftype     SaveConf;

  #ifdef DEBUG2
    #ifdef __BORLANDC__
      if (heapcheck() != _HEAPOK) {
        writedebugrecord("ERROR: forwardmessage() HEAP CORRUPT 1");
        return;
      }
    #endif
  #endif

  checkstack();
  SaveConf    = Status.CurConf;
  SaveConfNum = Status.Conference;
  if (MoveConfNum != Status.Conference) {
    Status.Conference = MoveConfNum;
    getconfrecord(Status.Conference,&Status.CurConf);
  }

  if ((p = getextendedheader(EXTHDR_TO,MsgBase->Body)) != NULL) {
    memcpy(To,p,EXTDESCLEN);
    To[EXTDESCLEN] = 0;
    if ((p = getextendedheader(EXTHDR_TO2,MsgBase->Body)) != NULL) {
      memcpy(&To[EXTDESCLEN],p,EXTDESCLEN);
      To[EXTDESCLEN*2] = 0;
    }
  } else {
    movestr(To,MsgBase->Header.ToField,sizeof(UsersRead.Name));
  }
  stripright(To,' ');

  BodyBytes = (unsigned short) ((MsgBase->Header.NumBlocks-1) * 128);

  // convert any embedded NULL bytes into spaces
  for (X = 0, p = MsgBase->Body; X < BodyBytes; X++, p++)
    if (*p == 0)
      *p = ' ';

  MsgBase->Body[BodyBytes-1] = 0;  // put a NULL terminator at the very end
  stripright(MsgBase->Body,' ');   // strip off trailing spaces (padding)

  p = loadextendedheaders(MsgBase->Body);

  // find out how far into the body of the message the actual text begins
  // (jumping over any extended headers)
  OffsetIntoBody = (int) (p - MsgBase->Body);

  // find out the original number of bytes in the message body, ignoring the
  // extended headers because we are about to remove some of them
  OrgBytes = BodyBytes - (NumExtHdrs * sizeof(msgextendtype));

  removeexthdr(EXTHDR_TO,0);       // if it already exists, get rid of it
  removeexthdr(EXTHDR_TO2,0);      // if it already exists, get rid of it
  removeexthdr(EXTHDR_FORWARD,0);  // if it already exists, get rid of it

  maxstrcpy(Str,To,sizeof(Str));
  gettoname(TXT_MSGTO,Str);        //lint !e534

  if (! Status.CurConf.LongToNames)
    strupr(Str);

  padstr(Str,' ',25);
  memcpy(MsgBase->Header.ToField,Str,25);

  sprintf(Buf,"%u %s",SaveConfNum,Status.DisplayName);
  buildextheader(EXTHDR_FORWARD,Buf,0);

  // add in the bytes from the extended headers
  NewBytes = (unsigned short) (OrgBytes + sizeof(msgextendtype) * NumExtHdrs);

  // check to see if we are shrinking the size ... if we are, then increase it
  // back up so that the math down below (where we fill it with spaces) does
  // not accidentally go negative and try to set a negative number of bytes.
  if (NewBytes < BodyBytes)
    NewBytes = BodyBytes;

  MsgBase->Header.NumBlocks = (char) (NewBytes / 128);
  if ((NewBytes & 127) != 0)
    MsgBase->Header.NumBlocks++;

  // find out the number of bytes currently allocated to the message body by
  // the msgread.c module
  AllocSize = MsgBase->BodySize * 128;

  NewBytes  = MsgBase->Header.NumBlocks * 128;

  // if the new size is larger than the size currently allocated then we need
  // to allocate more memory now
  if (NewBytes > AllocSize) {
    if ((Temp = (char *) brealloc(MsgBase->Body,NewBytes)) == NULL)
      return;

    MsgBase->Body = Temp;
    p = Temp + OffsetIntoBody;  // reload p in case MsgBase->Body "moved" in memory due to the realloc()
  }

  // Create a pointer that points to the end of the message body by
  // getting the start of the message body first, then add the number of
  // bytes of the original body (including extended headers)
  char *pEndOfBody = MsgBase->Body + BodyBytes;

  // calculate the new total number of bytes in the body of the message
  // including any new extended headers
  int NewBodyBytes = MsgBase->Header.NumBlocks * 128;

  // calculate the number of bytes from the current pointer (the end of the
  // message body) to the very end of the message body
  int BytesToEnd = NewBodyBytes - BodyBytes;

  // now fill the remaining bytes, from the end of the current text, to
  // the very end of the message body, with spaces
  memset(pEndOfBody,' ',BytesToEnd);

  #ifdef DEBUG2
    #ifdef __BORLANDC__
      if (heapcheck() != _HEAPOK) {
        writedebugrecord("ERROR: forwardmessage() HEAP CORRUPT 2");
        return;
      }
    #endif
  #endif

  // add one more for the header block
  MsgBase->Header.NumBlocks++;

  // restore the conference information now because savemessage() needs to
  // be able to detect both the source and destination conferences in case
  // a file attachment is being moved
  Status.CurConf    = SaveConf;
  Status.Conference = SaveConfNum;

  // save the extended status value so that it can be restored below
  SaveStatus = MsgBase->Header.ExtendedStatus;
  // then set the value to reflect *this* message which may be different
  // if, for example, a long TO name has been added
  MsgBase->Header.ExtendedStatus = ExtendedStatus;

  // save the message
  savemessage(MoveConfNum,&MsgBase->Header,p,TRUE);  //lint !e534

  // restore the ExtendedStatus value in case it is used later
  MsgBase->Header.ExtendedStatus = SaveStatus;
  msgcleanup();

  #ifdef DEBUG2
    #ifdef __BORLANDC__
      if (heapcheck() != _HEAPOK) {
        writedebugrecord("ERROR: forwardmessage() HEAP CORRUPT 3");
        return;
      }
    #endif
  #endif
}
#endif
