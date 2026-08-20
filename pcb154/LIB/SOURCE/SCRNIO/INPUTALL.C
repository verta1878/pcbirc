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


#include <mem.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <screen.h>
#include <system.h>
#include <misc.h>
#include "scrnio.h"
#include "scrnio.ext"

#ifdef DEBUG
#include <memcheck.h>
#endif

/*  Format for MASK definitions is:
 *    MASK[0] = length of MASK
 *    1) any number of characters  (like YESNO below)
 *    2) a 0 followed by 2 characters indicating a range of chars LOW and HIGH
 *       (like ALLCHAR below)
 *    3) a mix of any number of 1 and 2 (like ALLNUM below)
 */

char ALLCHAR[] = { 3, 0,' ',255 };
char ALLNUM[]  = { 7, '+','-','.',',', 0,'0','9' };
char ALLHEX[]  = { 9, 0,'0','9', 0,'A','F', 0,'a','f' };
char YESNO[]   = { 2, 'Y','N' };

static bool Insert;

struct InpType Input; /* Used to pass variables between functions for Input */


void (LIBENTRY *inputcallback)(char *Str) = NULL;

static void _NEAR_ LIBENTRY blankfield(int Offset);
static void _NEAR_ LIBENTRY delchar(int *Offset);

int KeyFlags;


/* This function is different from stripright in that it changes all        */
/* trailing spaces to NULL characters instead of just moving the terminator */

void LIBENTRY removespaces(char *Str, int StrLen) {
  for (--StrLen, Str += StrLen; StrLen >= 0; StrLen--, Str--) {
    if (*Str == ' ')
      *Str = 0;
    else if (*Str != 0)
      break;
  }
}


static void _NEAR_ LIBENTRY readscrnintowindow(int Offset, char Color) {
  int  AnsLen;
  int  ScrLen;
  char Str[80];

  /* read what's on the screen into a temporary buffer called Str */
  readscreen(Input.OrgX,Input.OrgY,Str,Color,Input.ScrLimit);

  /* if the screen and answer limits are the same, then just copy it in */
  if (Input.ScrLimit == Input.AnsLimit)
    strcpy(Input.Ans,Str);
  else {
    /* otherwise, we have a scrollable window */
    AnsLen = strlen(Input.Ans);
    ScrLen = strlen(Str);

    /* if the current answer is shorter than what was on the screen, then */
    /* just copy the answer off the screen into the answer field beginning */
    /* at the current window offset */

    if (AnsLen < Offset + ScrLen)
      strcpy(Input.Ans + Offset,Str);
    else {
      /* otherwise, we'll need to "fit" the answer that is on the screen */
      /* into the current answer without disturbing what is off to the left */
      /* or right of the window shown on screen */
      memcpy(Input.Ans + Offset,Str,ScrLen);
    }
  }
}


static void _NEAR_ LIBENTRY showwindow(char *Str, int Offset) {
  char New[80];

  sprintf(New,"%-*.*s",Input.ScrLimit,Input.ScrLimit,Str + Offset);
  fastprint(Input.OrgX,Input.OrgY,New,Colors[EDIT]);
}


/********************************************************************
*
*  Function: movecursor()
*
*  Utilizes the global variable  Input  and access the following fields:
*  --  Len, Limit, CurXY, CurX, OrgY  --
*
*  It adjusts the appropriate fields in accordance with the Move parameter
*  passed to it by the calling function
*/

static void _NEAR_ LIBENTRY movecursor(int Move, int *Offset) {
  if (Move == 0)
    return;

  if (Move > 0) {
    if (Input.Len < Input.ScrLimit) {
      Input.CurXY += Move << 1;
      Input.CurX  += (char) Move;
      Input.Len   += (char) Move;
      gotoxy(Input.CurX,Input.OrgY);
    } else {
      if (Input.Len + *Offset < Input.AnsLimit) {
        readscrnintowindow(*Offset,Colors[EDIT]);
        (*Offset)++;
        showwindow(Input.Ans,*Offset);
      }
    }
  } else {
    if (Input.Len > 0) {
      Input.CurXY += Move << 1;
      Input.CurX  += (char) Move;
      Input.Len   += (char) Move;
      gotoxy(Input.CurX,Input.OrgY);
    } else {
      if (Input.Len + *Offset < Input.AnsLimit) {
        readscrnintowindow(*Offset,Colors[EDIT]);
        (*Offset)--;
        showwindow(Input.Ans,*Offset);
      }
    }
  }
}


/********************************************************************
*
*  Function: blankfield()
*
*  Utilizes the following fields from Input:  Limit, Len, CurX, OrgY
*  It then simply blanks the field on the screen to the right of the cursor
*/

static void _NEAR_ LIBENTRY blankfield(int Offset) {
  int  AnsLen;
  int  ScrLen;
  int  X;
  char Temp[80];

  ScrLen = Input.ScrLimit - Input.Len;
  memset(Temp,' ',ScrLen);
  Temp[ScrLen] = 0;
  fastprint(Input.CurX,Input.OrgY,Temp,Colors[EDIT]);

  AnsLen = strlen(Input.Ans);
  if (Input.ScrLimit != Input.AnsLimit && AnsLen > Input.ScrLimit) {
    for (X = Input.Len + Offset; X < Input.AnsLimit; X++)
      Input.Ans[X] = ' ';
    Input.Ans[X] = 0;
  }
}


/********************************************************************
*
*  Function: showkey()
*
*  Checks to see if in INSERT mode.  If not and this is the first key pressed
*  then it blanks the field on the screen.  It then displays the character.
*  If in INSERT mode it shifts the characters to the right of it 1 space.
*/

static int _NEAR_ LIBENTRY showkey(int *Offset) {
  int LastChar;
  int Pos;

  if (! Insert) {
    if (! Input.Keyed)
      blankfield(*Offset);
    if (Input.ScrLimit != Input.AnsLimit && Input.Len+1 > Input.ScrLimit) {
      movecursor(1,Offset);
      movecursor(-1,Offset);
    }
    fastputc(Input.CurXY,Input.Ch);
    LastChar = ' ';
  } else {
    if (Input.ScrLimit != Input.AnsLimit) {
      Pos = Input.Len + *Offset;
      memmove(&Input.Ans[Pos + 1],&Input.Ans[Pos],Input.AnsLimit - Pos);
      Input.Ans[Input.AnsLimit] = 0;
      Input.Ans[Pos] = ' ';
      if (Input.Len >= Input.ScrLimit-2) {
        movecursor(1,Offset);
        movecursor(-1,Offset);
      }
    }

    LastChar = insertchar(Input.CurX,Input.OrgY,Input.Ch,(Input.ScrLimit+Input.OrgX-Input.CurX)-1);
  }

  movecursor(1,Offset);
  Input.Keyed = TRUE;
  return(LastChar);
}


/********************************************************************
*
*  Function: delchar()
*
*  Deletes the cursor to the left if BACKSPACE was hit.  Otherwise it deletes
*  the character where the cursor is locate.  In either case it shifts all
*  characters to the right of it left one space.
*/

static void _NEAR_ LIBENTRY delchar(int *Offset) {
  int Pos;

  if ((! Input.Ext) && (Input.Len == 0) && (*Offset == 0)) {
    beep();
  } else {
    if (! Input.Ext) {
      if (Input.Len > 0)
        movecursor(-1,Offset);
      else
        (*Offset)--;
    }


    if (Input.ScrLimit != Input.AnsLimit) {
      readscrnintowindow(*Offset,Colors[EDIT]);
      Pos = Input.Len + *Offset;
      memmove(&Input.Ans[Pos],&Input.Ans[Pos+1],Input.AnsLimit - Pos);
      Input.Ans[Input.AnsLimit] = 0;
      showwindow(Input.Ans,*Offset);
    } else
      deletechar(Input.CurX,Input.OrgY,' ',(int)Input.ScrLimit+Input.OrgX-Input.CurX);
  }
}


/********************************************************************
*
*  Function: charinmask
*
*  This function checks to see if Ch is in the Mask[] according to the
*  following rules:
*
*  1) if Input.AllCaps then convert Ch to uppercase
*  2) if Mask[x] != 0 then check to see if Ch == Mask[x]
*  3) if Mask[x] == 0 then check to see if Ch >= Mask[x+1] && Ch <= Mask[x+2]
*
*  if neither condition (2 or 3) is met then BEEP is called and FALSE is
*  returned.  Otherwise, TRUE is returned to the calling function.
*/

bool LIBENTRY charinmask(char Ch, char Mask[]) {
  int  X;

  for (X=0; X<=Mask[0]; X++) {
    if (Mask[X] == 0) {
      if (Ch >= Mask[X+1] && Ch <= Mask[X+2])
        return(TRUE);
      X += 2;
    } else {
      if (Ch == Mask[X])
        return(TRUE);
    }
  }
  return(FALSE);
}



/********************************************************************
*
*  Function: inputall
*
*  - Prints the default answer (question should already be on the screen)
*  - Initializes all of the fields in Input
*  - Continues to accept keystrokes until KeyFlags != 0
*  - Accepts user defined extended keyboard keystrokes and acts upon them
*  - Exits after a user defined keystroke if required
*  - Checks to see if keystroke is a built in editing command
*  - Checks to see if keystroke is in Mask[] making it a valid character
*  - Shows all input on screen
*  - Reads final input data from screen and stores it in Ans[]
*/

void LIBENTRY inputall(void) {
  int  X;
  int  Offset;
  char Temp[128];
  char Save[1024];

  strcpy(Save,Input.Ans);

  Insert = (bool) ((getkbdstatus() & INSERT) != 0);

  showwindow(Input.Old,0);
  gotoxy(Input.CurX,Input.OrgY);

  Input.CurXY = xyvalue(Input.CurX,Input.OrgY);
  Input.Len   = (char) (Input.CurX - Input.OrgX);
  Offset      = 0;

  KeyFlags = 0;
  while (KeyFlags == 0) {
    Input.Ch = inkey(&Input.Ext,ShowClock);

    if (Input.Ext) {
      for (X = 0; X < NumExitKeys; X++) {
        if (Input.Ch == ExitKeyNum[X]) {
          KeyFlags = ExitKeyFlag[X];
          break;
        }
      }

      if (KeyFlags == 0) {
        switch (Input.Ch) {
          case /* S-TAB  */  15: KeyFlags = SHIFTTAB; break;
          case /* F1     */  59: if (HelpFile != 0) showhelp(Input.HelpNum);
                                 break;
          case /* F10    */  68: strcpy(Input.Ans,Save);
                                 Offset = 0;
                                 showwindow(Input.Ans,Offset);
                                 break;
          case /* home   */  71: movecursor(-Input.Len,&Offset);
                                 if (Offset != 0) {
                                   readscrnintowindow(Offset,Colors[EDIT]);
                                   Offset = 0;
                                   showwindow(Input.Ans,Offset);
                                 }
                                 break;
          case /* up     */  72: KeyFlags = UP; break;
          case /* pgup   */  73: KeyFlags = PGUP; break;
          case /* left   */  75: if (Input.Len + Offset > 0) {
                                   movecursor(-1,&Offset); Input.Keyed=TRUE;
                                 } else {
                                   beep();
                                   continue;
                                 }
                                 break;
          case /* right  */  77: if (Input.Len + Offset < Input.AnsLimit) {
                                   movecursor(1,&Offset); Input.Keyed=TRUE;
                                 } else {
                                   beep();
                                   continue;
                                 }
                                 break;
          case /* end    */  79: readscrnintowindow(Offset,Colors[EDIT]);
                                 X = strlen(Input.Ans);
                                 if (X > 0) {
                                   for (X--; X >= 0 && Input.Ans[X] == ' '; X--);
                                   X++;
                                   if (X >= Input.ScrLimit && Input.ScrLimit != Input.AnsLimit) {
                                     Offset = X - (Input.ScrLimit-1);
                                     X = Input.ScrLimit-1;
                                     showwindow(Input.Ans,Offset);
                                   }
                                 }
                                 Input.CurX  = (char) (Input.OrgX + X);
                                 Input.CurXY = xyvalue(Input.CurX,Input.OrgY);
                                 Input.Len   = (char) X;
                                 gotoxy(Input.CurX,Input.OrgY);
                                 Input.Keyed=TRUE;
                                 break;
          case /* down   */  80: KeyFlags = DN; break;
          case /* pgdn   */  81: KeyFlags = PGDN; break;
          case /* insert */  82: if (Insert) {
                                   Insert = FALSE;
                                   setkbdstatus((uint) (getkbdstatus() & ~INSERT));
                                 } else {
                                   Insert = TRUE;
                                   setkbdstatus((uint) (getkbdstatus() | INSERT));
                                 }
                                 break;
          case /* delete */  83: delchar(&Offset); Input.Keyed=TRUE; break;
          case /* c-end  */ 117: blankfield(Offset); Input.Keyed=TRUE; break;
          case /* c-pgdn */ 118: KeyFlags = CTRLPGDN; break;
          case /* c-pgup */ 132: KeyFlags = CTRLPGUP; break;
        }
      }
    } else {
      switch (Input.Ch) {
        case  8: delchar(&Offset); Input.Keyed=TRUE; break;
        case  9: KeyFlags = TAB; break;
        case 13: KeyFlags = RET; break;
        case 27: KeyFlags = ESC; break;
        default: if (Input.AllCaps)
                   Input.Ch = (char) toupper(Input.Ch);
                 if (charinmask(Input.Ch,Input.Mask)) {
                   if (Input.Len + Offset < Input.AnsLimit) {
                     Input.Ch = (char) showkey(&Offset);
                     if (Input.Ch != ' ' && Input.AllowWrap)
                       KeyFlags = WORDWRAP;
                   } else {
                     if (Input.AllowWrap)
                       KeyFlags = WORDWRAP;
                     else {
                       beep();
                       continue;
                     }
                   }
                 } else {
                   beep();
                   continue;
                 }
      }
    }

    #ifdef __OS2__
      updatelinesnow();
    #endif

    if (inputcallback != NULL) {
      readscrnintowindow(Offset,Colors[EDIT]);
      maxstrcpy(Temp,Input.Ans,sizeof(Temp));
      removespaces(Temp,Input.AnsLimit);
      inputcallback(Temp);
    }
  }

  readscrnintowindow(Offset,Colors[ANSWER]);

  /* then replace all spaces on the end of the answer with NULL terminators */
  removespaces(Input.Ans,Input.AnsLimit);
}
