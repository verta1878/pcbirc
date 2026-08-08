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


#include <string.h>
#include <screen.h>
#include "ansi.h"

#ifdef DEBUG
#include <memcheck.h>
#endif

//#if defined(__BORLANDC__) && defined(__OS2__)
//  #include <inline.h>
//#endif

static void LIBENTRY copyline(void);

const int MAXCOL = 79;    // maximum X position

static char   Color;         // current color
static char   Xpos;          // current X coordinate
static char   Ypos;          // current Y coordinate
static char   SaveX;         // saved X coordinate (used by SAVPOS command)
static char   SaveY;         // saved X coordinate (used by SAVPOS command)
static char   MaxRow;        // bottom row of screen memory we'll be accessing
static bool   Hidden;        // true if screen writing is hidden
static bool   DoEscCodes;    // true if ESC codes are to be processed
static bool   CopyToBuf;     // true if copying to the scrollback buffer is enabled
static bool   Reposition;    // true if need to reposition screen pointer (because the cursor moved)
static char **ScrlOfsArray;  // address of index-into scrollback buffer
static int    ScrlNumLines;  // number of lines in scrollback buffer

// static char  *ScrlBuf;       // address of scrollback buffer
// static short  ScrnBytes;     // number of bytes in screen memory we'll be accessing

static char AscColors[] = { '0','4','2','6','1','5','3','7' };

static short ColorAndOrMask[48] = {
  (short) 0x0007,   //0
  (short) 0xFF08,   //1
  (short) 0xFFFF,   //2    (BAD CODE)
  (short) 0xFFFF,   //3    (BAD CODE)
  (short) 0xF801,   //4
  (short) 0xFF80,   //5
  (short) 0xFFFF,   //6    (BAD CODE)
  (short) 0xF870,   //7
  (short) 0x8800,   //8
  (short) 0xFFFF,   //9    (BAD CODE)
  (short) 0xFFFF,   //10   (BAD CODE)
  (short) 0xFFFF,   //11   (BAD CODE)
  (short) 0xFFFF,   //12   (BAD CODE)
  (short) 0xFFFF,   //13   (BAD CODE)
  (short) 0xFFFF,   //14   (BAD CODE)
  (short) 0xFFFF,   //15   (BAD CODE)
  (short) 0xFFFF,   //16   (BAD CODE)
  (short) 0xFFFF,   //17   (BAD CODE)
  (short) 0xFFFF,   //18   (BAD CODE)
  (short) 0xFFFF,   //19   (BAD CODE)
  (short) 0xFFFF,   //20   (BAD CODE)
  (short) 0xFFFF,   //21   (BAD CODE)
  (short) 0xFFFF,   //22   (BAD CODE)
  (short) 0xFFFF,   //23   (BAD CODE)
  (short) 0xFFFF,   //24   (BAD CODE)
  (short) 0xFFFF,   //25   (BAD CODE)
  (short) 0xFFFF,   //26   (BAD CODE)
  (short) 0xFFFF,   //27   (BAD CODE)
  (short) 0xFFFF,   //28   (BAD CODE)
  (short) 0xFFFF,   //29   (BAD CODE)
  (short) 0xF800,   //30
  (short) 0xF804,   //31
  (short) 0xF802,   //32
  (short) 0xF806,   //33
  (short) 0xF801,   //34
  (short) 0xF805,   //35
  (short) 0xF803,   //36
  (short) 0xF807,   //37
  (short) 0xFFFF,   //38   (BAD CODE)
  (short) 0xFFFF,   //39   (BAD CODE)
  (short) 0x8F00,   //40
  (short) 0x8F40,   //41
  (short) 0x8F20,   //42
  (short) 0x8F60,   //43
  (short) 0x8F10,   //44
  (short) 0x8F50,   //45
  (short) 0x8F30,   //46
  (short) 0x8F70,   //47
};

static short Yoffsets[] = {
   0*160, 1*160, 2*160, 3*160, 4*160, 5*160, 6*160, 7*160, 8*160, 9*160,
  10*160,11*160,12*160,13*160,14*160,15*160,16*160,17*160,18*160,19*160,
  20*160,21*160,22*160,23*160,24*160,25*160,26*160,27*160,28*160,29*160,
  30*160,31*160,32*160,33*160,34*160,35*160,36*160,37*160,38*160,39*160,
  40*160,41*160,42*160,43*160,44*160,45*160,46*160,47*160,48*160,49*160,
  50*160,51*160,52*160,53*160,54*160,55*160,56*160,57*160,58*160,59*160
};


void LIBENTRY setlimits(char NumLines) {
//ScrnBytes  = Yoffsets[NumLines];
  MaxRow     = (char) (NumLines - 1);
  Hidden     = FALSE;
  DoEscCodes = TRUE;
  CopyToBuf  = FALSE;
  setscreenupdateinterval(UPDT_DFLTINTERVAL);
}


#pragma argsused
void LIBENTRY toggleoff(char _FAR_ *ScrnBuf) {
//savescreen((savescrntype *) ScrnBuf);
//cls();
//setscreenupdateinterval(-1);  // set screen to update only on deman
  hidescreen();
  Hidden = TRUE;
  mydelay(25);
}


#pragma argsused
void LIBENTRY toggleon(char _FAR_ *ScrnBuf) {
//restorescreen((savescrntype *) ScrnBuf);
  gotoxy(Xpos,Ypos);
  unhidescreen();
  Hidden = FALSE;
  setscreenupdateinterval(UPDT_DFLTINTERVAL);
}


#undef changeupdateinterval
void LIBENTRY changeupdateinterval(int Interval) {
  if (Hidden)
    return;

  setscreenupdateinterval(Interval);
}


char LIBENTRY curcolor(void) {
  return(Color);
}


char LIBENTRY awherex(void) {
  return(Xpos);
}


char LIBENTRY awherey(void) {
  return(Ypos);
}


void LIBENTRY agotoxy(char X, char Y) {
  Xpos = X;

  if (Y > MaxRow)
    Ypos = MaxRow;
  else
    Ypos = Y;

  if (! Hidden)
    gotoxy(Xpos,Ypos);
}


void LIBENTRY asetcolor(char NewColor) {
  Color = NewColor;
}


int LIBENTRY colorstr(char *Str, char NewColor) {
  // if no new color is required, then return 0 to indicate no string generated
  if (Color == NewColor)
    return(FALSE);

  *Str++ = '';   // put the start of the ESC code into Str
  *Str++ = '[';

  if (NewColor == 0x70) {   // reverse video?
    *Str++ = '0';
    *Str++ = ';';
    *Str++ = '7';
    *Str++ = 'm';
    *Str   = 0;
    Color = NewColor;
    return(TRUE);
  }

  // split up the foreground and background colors

  char OldFore = (char) (Color & 0x0F);
  char OldBack = (char) ((Color & 0xF0) >> 4);
  char NewFore = (char) (NewColor & 0x0F);
  char NewBack = (char) ((NewColor & 0xF0) >> 4);

  // if the backgrounds have changed, or if the old foreground was high
  // intensity but the new one is not, then reset everything
  if (OldBack != NewBack || (((OldFore & 0x08) == 0x08) && NewFore < 0x08))
    Color = 0;

  // if the old color was 0 that is a signal to do a full reset
  if (Color == 0) {
    *Str++ = '0';
    *Str++ = ';';
    OldFore = 7;

    // set the background if necessary
    if (NewBack != 0) {
      // if color is greater than 7 then need to set the blinking mode
      if (NewBack > 7) {
        *Str++ = '5';
        *Str++ = ';';
        NewBack -= (char) 8;
      }
      *Str++ = '4';
      *Str++ = AscColors[NewBack];
      *Str++ = ';';
    }
  }

  // set the foreground if necessary
  if (NewFore != OldFore) {
    // if color is greater than 7 then need to set high intensity mode
    if (NewFore > 7) {
      *Str++ = '1';
      *Str++ = ';';
      NewFore -= (char) 8;
    }
    *Str++ = '3';
    *Str++ = AscColors[NewFore];
    *Str++ = 'm';
    *Str   = 0;
    Color = NewColor;
    return(TRUE);
  }

  // we added a trailing ';' which needs to be removed
  Str--;
  // now terminate the color string and return
  *Str++ = 'm';
  *Str   = 0;
  Color = NewColor;
  return(TRUE);
}


/******  I DON'T THINK THESE FUNCTIONS WILL BE NEEDED  *********************

// This function pulls characters out of the string being processed and
// moves the string pointer forward for every valid numeric it pulls out.
//
// It returns a value indicating how far to move forward, so a return value
// of 0 indicates that no valid numerics were found.  The numeric value
// to be returned is passed via a pointer

static int _NEAR_ LIBENTRY getnum(char *Str, short *Num) {
  char *p = Str;

  if (*p < '0' || *p > '9') {
    *Num = 1;
    return(0);
  }

  *Num = (*p - '0');

  for (p++; *p >= 0 && *p <= '9'; p++) {
    *Num *= 10;
    *Num += (*p - '0');
  }

  return((int) (p - Str));
}


// This function reads bytes out of a string and converts them into a color
// attribute and returns a pointer to where the string should be when done,
// based on how many bytes it read out of the string.

static char * _NEAR_ LIBENTRY docolor(char *Str) {
  short Num;
  int   Valid;

  if ((Valid = getnum(Str,&Num)) != 0) {
    Str += Valid;

    if (Num > 47)
      return(Str);

    Num = ColorAndOrMask[Num];
    if (Num == (short) 0xFFFF)
      return(Str);

    Color &= ((Num & 0xFF00) >> 8);
    Color |= (Num & 0x00FF);
    return(Str);
  }

  Color = 0x07;
  return(Str);
}

****************************************************************************/


static void _NEAR_ LIBENTRY scroll(void) {
  scrollup(0,0,MAXCOL,MaxRow,Color);
}


static void _NEAR_ LIBENTRY ansiclear(void) {
  clsbox(0,0,MAXCOL,MaxRow,Color);
  agotoxy(0,0);
  Reposition = TRUE;
}


static void _NEAR_ LIBENTRY ansicleareol(void) {
  int    X;
  short  Cell = (short) (((short) Color << 8) + ' ');
  short *Scrn;

  Scrn = (short *) ((char *) Scrn_Buf + Yoffsets[Ypos] + (Xpos << 1));
  for (X = Xpos; X <= MAXCOL; X++, Scrn++)
    *Scrn = Cell;

  if (! Hidden)
    updatelines(UPDATE_MIXED,Ypos,Ypos);
}


void LIBENTRY doesccodes(void) {
  DoEscCodes = TRUE;
}


void LIBENTRY noesccodes(void) {
  DoEscCodes = FALSE;
}


static void _NEAR_ LIBENTRY cursorup(char Param) {
  if (Ypos < Param)
    Ypos = 0;
  else
    Ypos -= Param;
}


static void _NEAR_ LIBENTRY cursordown(char Param) {
  if (Ypos + Param > MaxRow)
    Ypos = MaxRow;
  else
    Ypos += Param;
}


static void _NEAR_ LIBENTRY cursorforward(char Param) {
  if (Xpos + Param > MAXCOL)
    Xpos = MAXCOL;
  else
    Xpos += Param;
}


static void _NEAR_ LIBENTRY cursorbackward(char Param) {
  if (Xpos < Param)
    Xpos = 0;
  else
    Xpos -= Param;
}


static void _NEAR_ LIBENTRY cursorposition(char Param1, char Param2) {
  Param1--;
  if (Param1 > MaxRow)
    Ypos = MaxRow;
  else
    Ypos = Param1;
  Param2--;
  if (Param2 > MAXCOL)
    Xpos = MAXCOL;
  else
    Xpos = Param2;
}


static void _NEAR_ LIBENTRY color(int State, char P1, char P2, char P3, char P4, char P5) {
  char  P;
  char  X;
  short Num;

  // X will range from 0 to State, where State has a maximum of 4
  // therefore State is used to pick up each of the 5 potential parameters
  // P will never be uninitialized because it will go through the 0-4 switch

  for (X = 0; X <= State; X++) {
    switch (X) {
      case 0: P = P1; break;
      case 1: P = P2; break;
      case 2: P = P3; break;
      case 3: P = P4; break;
      case 4: P = P5; break;
    }

    if (P <= 47) {         //lint !e644 initialization okay, see comment above
      Num = ColorAndOrMask[P];
      if (Num != (short) 0xFFFF) {
        Color &= (char) ((Num & 0xFF00) >> 8);
        Color |= (char) (Num & 0x00FF);
      }
    }
  }
}


static char * _NEAR_ LIBENTRY escapecodes(char *Str) {
  char  Param1,Param2,Param3,Param4,Param5;
  char  C;
  int   State;
  char *p;

  Param1 = Param2 = Param3 = Param4 = Param5 = 0;
  State = 0;
  p = Str;

  while (1) {
    C = *p++;
    switch (C) {
      case  0  : return(Str);
      case '0' :
      case '1' :
      case '2' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '7' :
      case '8' :
      case '9' : switch (State) {
                   case 0: Param1 *= (char) 10;
                           Param1 += (char) (C - '0');
                           break;
                   case 1: Param2 *= (char) 10;
                           Param2 += (char) (C - '0');
                           break;
                   case 2: Param3 *= (char) 10;
                           Param3 += (char) (C - '0');
                           break;
                   case 3: Param4 *= (char) 10;
                           Param4 += (char) (C - '0');
                           break;
                   case 4: Param5 *= (char) 10;
                           Param5 += (char) (C - '0');
                           break;
                 }
                 break;
      case ';' : State++;
                 break;
      case 'A' : if (Param1 == 0) Param1 = 1;
                 cursorup(Param1);
                 Reposition = TRUE;
                 return(p);
      case 'B' : if (Param1 == 0) Param1 = 1;
                 cursordown(Param1);
                 Reposition = TRUE;
                 return(p);
      case 'C' : if (Param1 == 0) Param1 = 1;
                 cursorforward(Param1);
                 Reposition = TRUE;
                 return(p);
      case 'D' : if (Param1 == 0) Param1 = 1;
                 cursorbackward(Param1);
                 Reposition = TRUE;
                 return(p);
      case 'E' : return(Str);
      case 'F' : return(Str);
      case 'G' : return(Str);
      case 'H' : if (Param1 == 0) Param1 = 1;
                 if (Param2 == 0) Param2 = 1;
                 cursorposition(Param1,Param2);
                 Reposition = TRUE;
                 return(p);
      case 'I' : return(Str);
      case 'J' : if (Param1 != 2) return(Str);
                 ansiclear();
                 return(p);
      case 'K' : ansicleareol();
                 return(p);
      case 'L' : return(Str);
      case 'M' : return(Str);
      case 'N' : return(Str);
      case 'O' : return(Str);
      case 'P' : return(Str);
      case 'Q' : return(Str);
      case 'R' : return(Str);
      case 'S' : return(Str);
      case 'T' : return(Str);
      case 'U' : return(Str);
      case 'V' : return(Str);
      case 'W' : return(Str);
      case 'X' : return(Str);
      case 'Y' : return(Str);
      case 'Z' : return(Str);
      case '[' : return(Str);
      case '\\': return(Str);
      case ']' : return(Str);
      case '^' : return(Str);
      case '_' : return(Str);
      case '\'': return(Str);
      case 'a' : return(Str);
      case 'b' : return(Str);
      case 'c' : return(Str);
      case 'd' : return(Str);
      case 'e' : return(Str);
      case 'f' : if (Param1 == 0) Param1 = 1;
                 if (Param2 == 0) Param2 = 1;
                 cursorposition(Param1,Param2);
                 Reposition = TRUE;
                 return(p);
      case 'g' : return(Str);
      case 'h' : return(p);    // Set Mode - not implemented
      case 'i' : return(Str);
      case 'j' : return(Str);
      case 'k' : return(Str);
      case 'l' : return(p);    // Reset Mode - not implemented
      case 'm' : color(State,Param1,Param2,Param3,Param4,Param5);
                 return(p);
      case 'n' : return(Str);
      case 'o' : return(Str);
      case 'p' : return(p);    // Redefine Keyboard - not implemented
      case 'q' : return(Str);
      case 'r' : return(Str);
      case 's' : SaveX = Xpos;
                 SaveY = Ypos;
                 return(p);
      case 't' : return(Str);
      case 'u' : agotoxy(SaveX,SaveY);
                 Reposition = TRUE;
                 return(p);
      default  : return(Str);
    }
  }
}


static void _NEAR_ LIBENTRY callupdate(char *UpdatedLines) {
  int FirstY = 255;
  int LastY  = 0;
  int Y;

  for (Y = 0; Y <= MaxRow; Y++) {
    if (UpdatedLines[Y]) {
      if (Y < FirstY)
        FirstY = Y;
      if (Y > LastY)
        LastY = Y;
      UpdatedLines[Y] = 0;
    }
  }

  updatelines(UPDATE_MIXED,FirstY,LastY);
}


void LIBENTRY ansi(char *Str) {
  char   C;
  char   OldX, OldY;
  char  *Scrn;
  bool   Updated;
  char   UpdatedLines[50];

  memset(UpdatedLines,0,sizeof(UpdatedLines));

  Reposition = FALSE;
  Updated    = FALSE;
  OldX       = Xpos;
  OldY       = Ypos;

repositionpointer:
  Scrn = ((char *) Scrn_Buf) + Yoffsets[Ypos] + (Xpos << 1);

  while (1) {
    C = *Str++;
    switch (C) {
      case  0: goto done;

      case  7: bell();
               break;

      // backspace
      case  8: if (Xpos > 0) {
                 Xpos--;
                 Scrn -= 2;
               }
               break;

      // tab
      case  9: Xpos += (char) (8 - (Xpos & 7));
               if (Xpos <= MAXCOL)
                 goto repositionpointer;
               Xpos = 0;
               // fall through to line feed

      // line feed
      linefeed:
      case 10: if (CopyToBuf)                    //lint !e616
                 copyline();
               if (Updated && ! Hidden) {
                 callupdate(UpdatedLines);
                 Updated = FALSE;
               }
               Ypos++;
               if (Ypos > MaxRow) {
                 scroll();
                 Ypos = MaxRow;
               }
               goto repositionpointer;

      // carriage return
      case 13: Scrn -= (Xpos << 1);
               Xpos = 0;
               break;

      // EOF character - just strip it out
      case 26: break;

      // escape character
      case 27: if (DoEscCodes) {
                 // check first for the [ that goes with the "ESC[" pair
                 if (*Str != '[') {
                   // the [ was not found, so loop back up and print it
                   break;
                 }
                 Str = escapecodes(Str+1);
                 if (Reposition) {
                   Reposition = FALSE;
                   goto repositionpointer;
                 }
                 break;
               }
               C = '';
               // fall through to display the '' character.

      // all other characters are displayed on the screen
      default: *Scrn++ = C;
               *Scrn++ = Color;
               Updated = TRUE;
               UpdatedLines[Ypos] = TRUE;
               Xpos++;
               if (Xpos > MAXCOL) {
                 Xpos = 0;
                 goto linefeed;
               }
               break;
    }
  }

done:
  if (! Hidden) {
    if (Updated)
      callupdate(UpdatedLines);
    if (OldX != Xpos || OldY != Ypos)
      gotoxy(Xpos,Ypos);
  }
}


void LIBENTRY scrolloff(void) {
  CopyToBuf = FALSE;
}


void LIBENTRY reenablescroll(void) {
  CopyToBuf = TRUE;
}


void LIBENTRY scrollon(char *Buffer, char **Offset, int NumLines) {
  int    X;
  char  *p;
  short *q;

//ScrlBuf      = Buffer;
  ScrlOfsArray = Offset;
  ScrlNumLines = NumLines;

  // fill the Offset array with offsets (pointers) into the Buffer where each
  // offset is the start of another line of 160 bytes (80 chars+attributes)
  for (X = 0, p = Buffer; X < NumLines; X++, Offset++) {
    *Offset = p;
    p += 160;
  }

  // initialize the scrollback buffer to all spaces
  for (X = 0, q = (short _FAR_ *) Buffer; X < NumLines*80; X++, q++)
    *q = 0x0720;

  CopyToBuf = TRUE;
}


#ifndef TEST
static void LIBENTRY copyline(void) {
  char *Save;

  // rotate the Offset array by saving a copy of offset 0
  // then moving offset 1 to 0, 2 to 1, 3 to 2, etc
  // then put the saved copy of offset 0 into the last slot
  Save = ScrlOfsArray[0];
  memmove(&ScrlOfsArray[0],&ScrlOfsArray[1],sizeof(int)*ScrlNumLines);
  ScrlOfsArray[ScrlNumLines-1] = Save;

  // now copy a complete 160-byte line from the video memory into
  // the scrollback buffer at the address in the scrollback buffer which
  // is pointed to by the bottom index pointer
  memcpy(Save,(char *) Scrn_Buf + Yoffsets[Ypos],160);
}


void LIBENTRY viewbuff(int TopLine) {
  int   X;
  char *p = (char *) Scrn_Buf;

//for (X = 0; X <= MaxRow; X++, TopLine++, p += 160)
  for (X = 0; X < 23; X++, TopLine++, p += 160)
    memcpy(p,ScrlOfsArray[TopLine],160);

  updatelines(UPDATE_MIXED,0,MaxRow);
}


void LIBENTRY tagem(int X1, int Y1, int X2, int Y2, char Attr) {
  int   X;
  int   Y;
  int   EndX;
  char *p;

  //  Tags an area in the Scrollback Buffer by reversing the video attributes
  //  (taken from the following C code:)

  X1   = (X1 << 1) + 1;
  X2   = (X2 << 1) + 1;
  EndX = 159;

  for (Y = Y1; Y <= Y2; Y++) {
    if (Y == Y2)
      EndX = X2;
//  Ofs = Yoffsets[Y];
    p = ScrlOfsArray[Y];
    for (X = X1; X <= EndX; X+=2)
//    ((char *) Scrn_Buf)[Ofs + X] = Attr;
      p[X] = Attr;
    X1 = 1;
  }

  updatelines(UPDATE_MIXED,Y1,Y2);
}


void LIBENTRY gettagged(char _FAR_ *Str,int X1, int X2, int Y, char Attr) {
  int   X;
  char *p;

  //  Tags an area in the Scrollback Buffer by reversing the video attributes
  //  (taken from the following C code:)

  X1 = (X1 << 1);
  X2 = (X2 << 1);

//Ofs = Yoffsets[Y];
  p = ScrlOfsArray[Y];

  for (X = X1; X <= X2; ) {
//  *Str++ = ((char *) Scrn_Buf)[Ofs + X++];
//  ((char *) Scrn_Buf)[Ofs + X++] = Attr;
    *Str++ = p[X++];
    p[X++] = Attr;
  }
  *Str = 0;

  updatelines(UPDATE_MIXED,Y,Y);
}
#endif

#if defined(PREANSI) || defined(BYTEANSI)
  static char SaveBuf[128];  // a buffer to hold incomplete ESC codes
  static int  SaveLen;       // a record of the size of the incomplete code
#endif


#ifdef PREANSI
void LIBENTRY preansi(char *Str, int Len) {

  char *pEsc;  // pointer to the ESC character that was found
  char *p;
  int   X;
  int   EscCodeLen;

  // nothing to print, get out now
  if (Len == 0)
    return;

  // if SaveLen != 0 then we were previously interrupted so now we need to
  // resume by scanning for the end of the current ESC code while copying
  // the bytes we are scanning into SaveBuf
  if (SaveLen != 0) {
    for (p = SaveBuf+SaveLen; Len > 0; p++, Str++, Len--) {
      // copy the character out of Str and into SaveBuf
      *p = *Str;

      // record that we've added a character to SaveBuf
      SaveLen++;

      // this is part of the ESC code so just loop back around for more
      if (*Str == '[')
        continue;

      // check for invalid end characters...
      // anything from ascii 1 to 47 is invalid, anything from ascii
      // 60 to 255 is either valid (which means we're done), or invalid (which
      // means it's not an ESC code) so print it and then go back to
      // normal processing down below
      if (*Str < '0' || *Str > ';') {
        // first we need to terminate the string, then print it
        *(p+1) = 0;
        ansi(SaveBuf);
        // reset SaveLen back to 0 since we're done processing the saved bytes
        SaveLen = 0;
        // advance Str by the 1 byte we copied before we "break" out
        Str++;
        // decrement Len by the 1 byte we copied before we "break" out
        Len--;

        #ifdef TEST
        memset(SaveBuf,0xff,sizeof(SaveBuf));
        #endif

        break;
      }
    }

    // if Len != 0 then there are still more characters in the string that
    // need to be processed, which we can do by dropping down into the code
    // below, otherwise we're done and can exit now
    if (Len == 0)
      return;
  }

  // search backwards from the end of Str looking for an ESC character
  for (X = Len, p = Str+Len-1; X > 0; X--, p--) {
    if (*p == 27) {
      pEsc = p;
      goto found;
    }
  }

  // no ESC character was found, so just print the string and get out now

  // WARNING!!!!!!  This line is going to write over the last byte in
  //                Str, so make sure that the Str buffer is ONE BYTE LARGER
  //                than the "Len" value that is passed in!

  Str[Len] = 0;     // Make sure the string is NULL-terminated
  ansi(Str);
  return;


  found:
  // Since we were searching backwards, the fact that we are here now means
  // that we found the last possible ESC code in the string.  Now we need to
  // scan forward and find out if this is a complete "ESC code" or if the
  // code has been interrupted.

  // skip over the ESC character by moving X and p forward and set the
  // EscCodeLen to 1 to account for the ESC character
  for (X++, p++, EscCodeLen = 1; X <= Len; X++, p++, EscCodeLen++) {
    // this is part of the ESC code so just loop back around for more
    if (*p == '[')
      continue;

    // check for invalid end characters...
    // anything from ascii 1 to 47 is invalid, anything from ascii
    // 60 to 255 is either valid (which means we're done), or invalid (which
    // means it's not an ESC code) so just go print the string and get out
    if (*p < '0' || *p > ';')
      goto print;
  }

  // if we got this far then we've reached the end of the string without
  // having encountered the end of the ESC code
  if (pEsc != Str) {
    // The ESC character was not at the beginning of the string, so let's
    // print what we have so far in order to reduce the size of what needs
    // to be saved.  First, NULL-terminate the string, then print it,
    // then put the ESC character back
    *pEsc = 0;
    ansi(Str);
    *pEsc = 27;
  }
  // save the escape code and store the length of the code for later
  // reference, then exit and we'll resume this process on the next call
  memcpy(SaveBuf+SaveLen,pEsc,EscCodeLen);
  SaveLen = EscCodeLen;
  return;

  print:
  // WARNING!!!!!!  This line is going to write over the last byte in
  //                Str, so make sure that the Str buffer is ONE BYTE LARGER
  //                than the "Len" value that is passed in!

  Str[Len] = 0;     // Make sure the string is NULL-terminated
  // it's either a complete ESC code, or an invalid one, so print it now
  ansi(Str);
  return;
}
#endif


#ifdef BYTEANSI
void LIBENTRY byteansi(char Byte) {
  // check to see if we already have an ESC code "in progress"
  if (SaveLen) {
    // we do, so save this new byte into the buffer
    SaveBuf[SaveLen++] = Byte;
    // if the character is a valid character for ESC codes, then just return
    if (Byte == '[' || (Byte >= '0' && Byte <= ';'))
      return;
    // otherwise, we either finished the code, or we encountered an invalid
    // character. in either case, we need to display the buffer
    goto display;
  }

  // check for the ESC character
  if (Byte == 27) {
    // this is the start of an ESC code so save the first byte
    SaveBuf[0] = 27;
    SaveLen = 1;
    return;
  } else {
    // otherwise just print the byte
    SaveBuf[0] = Byte;
    SaveBuf[1] = 0;
    // this is inefficient (printing just one byte at a time), but it works...
    ansi(SaveBuf);
    return;
  }

  display:
  // Add the NULL-terminator, then display the contents of SaveBuf
  SaveBuf[SaveLen] = 0;
  ansi(SaveBuf);
  // Reset back to the beginning of the buffer
  SaveLen = 0;
}
#endif


#ifdef TEST
#include <dosfunc.h>
void LIBENTRY bell(void) {
}

void LIBENTRY copyline(void) {
}

void main(int argc, char **argv) {
  DOSFILE File;
  #ifdef BYTEANSI
  char    Byte;
  #endif
  #ifdef PREANSI
  int     Status;
  char    Str[6];
  #else
  char    Str[4096];
  #endif

  if (argc != 2)
    return;

  if (dosfopen(argv[1],OPEN_READ|OPEN_DENYNONE,&File) == -1)
    return;

  getmode();
  setlimits(24);
  cls();
  asetcolor(0x07);
  agotoxy(0,0);

  #ifdef TESTBYTEANSI
    while (dosfread(&Byte,1,&File) == 1)
      byteansi(Byte);
  #else
    while (1) {
      Status = dosfgets(Str,sizeof(Str),&File);
      if (Status == -1)
        break;

      #ifdef TESTPREANSI
        preansi(Str,strlen(Str));
      #else
        ansi(Str);
      #endif

      if (Status == 0)
        ansi("\r\n");
    }
  #endif

  updatelinesnow();
  mydelay(50);
}
#endif
