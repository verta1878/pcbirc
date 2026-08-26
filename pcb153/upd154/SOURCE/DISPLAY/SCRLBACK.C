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

#include <system.h>

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <malloc.h>
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#define BYTESPERLINE     83        /* one full line plus quoting and NULL */
#define SINGLELINEFUDGE   5        /* quoting and NULL overhead */

#if defined(__LARGE__) || defined(__COMPACT__) || defined(__OS2__)
  #define farmalloc(x) malloc(x)
  #define farfree(x)   free(x)
#endif

enum {NOTAG, START, END};

typedef struct {
  int X;
  int Y;
} postype;


#ifdef __OS2__
  static char **Offset = NULL;
#else
  static int  _FAR_ *Offset = NULL;
#endif

static char    _FAR_ *Buffer   = NULL;
static postype Start;
static postype End;
static bool    FirstLineInScrollBack = FALSE;

extern uint far *KbdStatus;

/********************************************************************
*
*  Function:  turnoffscrollback()
*
*  Desc    :
*/

void LIBENTRY turnoffscrollback(void) {
  if (Status.ScrollOn) {
    farfree(Offset);
    farfree(Buffer);
    Offset = NULL;
    Buffer = NULL;
    scrolloff();
    Status.ScrollOn = FALSE;
  }
}


/********************************************************************
*
*  Function:  turnonscrollback()
*
*  Desc    :
*/

void LIBENTRY turnonscrollback(void) {
  if (PcbData.MaxScrollBack == 0) {
    Status.ScrollOn = FALSE;
    return;
  }

  #ifndef __OS2__
    if (PcbData.MaxScrollBack > (int) (64000U / 162))
      PcbData.MaxScrollBack = (int) (64000U / 162);
    else
  #endif

  if (PcbData.MaxScrollBack < 30)
    PcbData.MaxScrollBack = 30;

  if ((Buffer = (char _FAR_ *) farmalloc(PcbData.MaxScrollBack * 160U)) == NULL) {
    Status.ScrollOn = FALSE;
    return;
  }

  #ifdef __OS2__
  if ((Offset = (char **) farmalloc((unsigned)PcbData.MaxScrollBack*sizeof(int))) == NULL) {
  #else
  if ((Offset = (int _FAR_ *) farmalloc((unsigned)PcbData.MaxScrollBack*sizeof(int))) == NULL) {
  #endif
    farfree(Buffer);
    Status.ScrollOn = FALSE;
    return;
  }

  scrollon(Buffer,Offset,PcbData.MaxScrollBack);
  Status.ScrollOn = TRUE;

  #ifdef USE_ATEXIT   // no longer used!
    atexit(turnoffscrollback); //lint !e534
  #endif
}


/********************************************************************
*
*  Function:  addtaggedtext()
*
*  Desc    :  takes text that is flagged either by the SCROLLBACK BUFFER or
*             by the message editor quote function and adds it to the the
*             Status.TaggedText via the Status.TagPointer variable.
*
*             Note: both the scrollback and msg editor routines are in
*             overlays so leave this routine here to share between the two.
*/

void LIBENTRY addtaggedtext(char *Str) {
  bool Formatted;
  int  X;

  Formatted = FALSE;
  stripright(Str,' ');

  for (X = 1; X <= 31; X++)       /* Remove any CONTROL CODES from the text */
    stripall(Str,(char) X);       /* so the editor does not act upon them   */

  if (Str[0] == 0) {
    if (Status.TagPointer > Status.TaggedText+3 && strcmp(Status.TagPointer-3,"\r \r") != 0) {
/*    *(Status.TagPointer++) = 13; */
      *(Status.TagPointer++) = ' ';
      *(Status.TagPointer++) = 13;
    }
  } else {
    if (Str[0] == ' ') {
      Str++;
      if (Str[0] == ' ')  /* check the 2nd column */
        Str++;
      if (Str[0] == ' ') {  /* check the 3rd column */
        Str++;
        Formatted = TRUE;  /* 3rd is blank too must be formatted text */
      }
    } else if (Str[0] == '-' && Str[1] == '>') {
      switch (Str[2]) {
        case ' ':  Str += 3; Formatted = TRUE; break;
        case  0 :  Str += 2; Formatted = TRUE; break;
      }
    } else if (strlen(Str) < 63 && ! FirstLineInScrollBack) {
      Formatted = TRUE;
    }
/*
    if (! Formatted)
*/
      *(Status.TagPointer++) = ' ';
    strcpy(Status.TagPointer,Str);
    Status.TagPointer += strlen(Str);
    if (Formatted)
      *(Status.TagPointer++) = 13;
  }
  *Status.TagPointer = 0;  /* always NULL terminate it */

/*
  if (Str[0] == '-' && Str[1] == '>' && (Str[2] == ' ' || Str[2] == 0)) {
  } else {
    *(Status.TagPointer++) = '-';
    *(Status.TagPointer++) = '>';
    *(Status.TagPointer++) = ' ';
  }
  strcpy(Status.TagPointer,Str);
  Status.TagPointer += strlen(Str);
  *(Status.TagPointer++) = 13;
  *Status.TagPointer     = 0;
*/
}


/********************************************************************
*
*  Function:  viewscrollback()
*
*  Desc    :
*/

/* found a bug in BC++ 3.1 .. when using -3 and -Op the Start and End  */
/* structures are not properly managed.  Using -O-p solves this until  */
/* Borland can get a real fix into the compiler.                       */
#ifndef __OS2__
#pragma option -O-p
#endif

void LIBENTRY viewscrollback(void) {
  bool    Tag;
  bool    ScrollLock;
  char    RestoreColor;
  short   Line;
  short   OldLine;
  int     OldX;
  int     OldY;
  int     SaveX;
  int     SaveY;
  union {
    short W;
    struct {
      char LoB,HiB;
    } B;
  } Key;
  postype Cursor;
  savescrntype *Screen;
  char          Str[81];
  pcbtexttype   Keys;
  pcbtexttype   Above;
  pcbtexttype   Below;

  if (Status.InScrollBack)
    return;

  if (! Status.ScrollOn)
    goto exit;

  if ((Screen = (savescrntype *) farmalloc(sizeof(savescrntype))) == NULL)
    goto exit;

  Status.InScrollBack = TRUE;
  RestoreColor = (char) (0x07 | (Display.DefaultColor & 0xF0));

  getpcbtext(TXT_SCROLLKEYS ,&Keys);
  getpcbtext(TXT_SCROLLABOVE,&Above);
  getpcbtext(TXT_SCROLLBELOW,&Below);

  ScrollLock = getkbdstatus() & SCROLL;

  OldX = awherex();
  OldY = awherey();
  savescreen(Screen);
  clsbox(0,23,79,24,7<<4);
  fastprint(2,23,Keys.Str,7<<4);
  fastprint(6,24,Above.Str,7<<4);
  fastprint(43,24,Below.Str,7<<4);

  setcursor(CUR_BLOCK);
  agotoxy(39,22);
  Cursor.X = 39;
  Cursor.Y = 22;
  Line = (short) (PcbData.MaxScrollBack-23);
  OldLine = -1;
  SaveX = SaveY = 0;
  Tag = FALSE;

  while (1) {
    if (Cursor.X != SaveX || Cursor.Y != SaveY) {
      agotoxy((char) Cursor.X,(char) Cursor.Y);
      sprintf(Str,"%-3d",Cursor.Y+Line);
      fastprint(33,24,Str,7<<4);
      sprintf(Str,"%-3d",PcbData.MaxScrollBack-Cursor.Y-Line-1);
      fastprint(70,24,Str,7<<4);
      SaveX = Cursor.X;
      SaveY = Cursor.Y;
    }

    if (Line != OldLine) {
      viewbuff(Line);
      OldLine = Line;
    }

    #ifdef __OS2__
      updatelinesnow();
      if ((Key.W = (short) waitforkey()) == 0) {
        if (ScrollLock && (getkbdstatus() & SCROLL) == 0)
          Key.W = 27;
      }
    #else
      if ((Key.W = kbdhit(NOBUFFER)) == 0) {
        if (ScrollLock && (*KbdStatus & SCROLL) == 0)
          Key.W = 27;
        else {
          giveup();
          continue;
        }
      }
    #endif

    switch (Key.W) {
      case   27: restorescreen(Screen);
                 farfree(Screen);
                 setcursor(CUR_NORMAL);
                 agotoxy((char) OldX,(char) OldY);
                 if (Tag != NOTAG) {
                   if (Status.EnteringMessage) {
                     if ((Status.TaggedText = (char *) checkmalloc((End.Y-Start.Y) * BYTESPERLINE + (End.X-Start.X+SINGLELINEFUDGE),"TAG")) != NULL) {
                       Status.TaggedText[0] = 0;
                       Status.TagPointer = Status.TaggedText;
                       for (OldY = Start.Y; OldY <= End.Y; OldY++) {
                         gettagged(Str,(OldY==Start.Y ? Start.X : 0),(OldY==End.Y ? End.X : 79),OldY,RestoreColor);
                         FirstLineInScrollBack = (bool) (OldY == Start.Y && Start.X > 0);
                         addtaggedtext(Str);
                         FirstLineInScrollBack = FALSE;
                       }
                       if (*(Status.TagPointer-1) == 13)
                         *(Status.TagPointer-1) = 0;
                       stripright(Status.TaggedText,' ');
                       Status.TagPointer = Status.TaggedText;
                     }
                   } else tagem(Start.X,Start.Y,End.X,End.Y,RestoreColor);
                 }
                 renewkbdtimer();
                 Status.InScrollBack = FALSE;
                 goto exit;
      case   32: switch (Tag) {
                   case NOTAG: Tag = START;
                               Start = Cursor;
                               Start.Y += Line;
                               End = Start;
                               tagem(Start.X,Start.Y,End.X,End.Y,0x70);
                               break;
                   case START: Tag = END;
                               if (Cursor.Y+Line > Start.Y || (Cursor.Y+Line == Start.Y && Cursor.X >= Start.X)) {
                                 End = Cursor;
                                 End.Y += Line;
                                 tagem(Start.X+1,Start.Y,End.X,End.Y,0x70);
                               } else {
                                 End = Start;
                                 Start = Cursor;
                                 Start.Y += Line;
                                 tagem(Start.X,Start.Y,End.X-1,End.Y,0x70);
                               }
                               break;
                   case END:   Tag = NOTAG;
                               tagem(Start.X,Start.Y,End.X,End.Y,RestoreColor);
                               break;
                 }
                 OldLine = -1;
                 break;
      case 1071: Cursor.X = 0;                           break;
      case 1072: Cursor.Y--;                             break;
      case 1073: Line-= (short) 23; SaveY = -1;          break;
      case 1075: Cursor.X--;                             break;
      case 1077: Cursor.X++;                             break;
      case 1079: Cursor.X = 79;                          break;
      case 1080: Cursor.Y++;                             break;
      case 1081: Line+= (short) 23; SaveY = 01;          break;
      case 1117: Cursor.Y = 23; Line = (short) (PcbData.MaxScrollBack-23); break;
      case 1119: Cursor.Y = 0; Line = 0;                 break;
    }

    if (Cursor.X == -1)
      Cursor.X = 0;
    else if (Cursor.X > 79)
      Cursor.X = 79;

    if (Cursor.Y == -1) {
      Cursor.Y = 0;
      Line--;
      SaveY = 01;
    } else if (Cursor.Y > 22) {
      Cursor.Y = 22;
      Line++;
      SaveY = -1;
    }

    if (Line < 0)
      Line = 0;
    else if (Line > PcbData.MaxScrollBack-23)
      Line = (short) (PcbData.MaxScrollBack-23);
  }

exit:
//if (getkbdstatus() & SCROLL)
    setkbdstatus(getkbdstatus() & ~SCROLL);

  redisplaystatusline();
  #ifdef __OS2__
    updatelinesnow();
  #endif
}

#ifndef __OS2__
#pragma -Op
#endif
