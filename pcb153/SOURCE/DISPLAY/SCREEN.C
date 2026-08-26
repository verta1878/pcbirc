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

#if defined(_MSC_VER) || defined(__WATCOMC__)
  #include <malloc.h>
  #ifndef __WATCOMC__
    #define farmalloc _fmalloc
    #define farfree   _ffree
  #endif
#else
  #ifndef __OS2__
    #include <alloc.h>
  #endif
#endif

#define MAX_SAVE 5

#if defined(__LARGE__) || defined(__COMPACT__)
  #define farmalloc(x) malloc(x)
  #define farfree(x)   free(x)
#endif

/*
typedef struct {
  char Char;
  char Color;
} celltype;
*/

typedef struct {
  celltype Mem[50][80];
} scrntype;

typedef struct {
  int      ScreenNum;
  char     SaveX;
  char     SaveY;
  char     SaveColor;
} displaysessiontype;

static int  NumSaved;
static displaysessiontype Session[MAX_SAVE];

static char _FAR_ *ScrnBuff = NULL;
static int  CursorType;


/********************************************************************
*
*  Function:  savescrnbuffer()
*
*  Desc    :  Saves the current screen buffer
*
*  Returns :  0 = if all went well with saving the buffer
*             0 = screen couldn't be saved (no big loss)
*            -1 = if there ALREADY EXISTS a saved buffer - we don't want
*                 to lose the saved buffer so force it to WAIT until the
*                 existing buffer is fully restored and freed.
*/

int LIBENTRY savescrnbuffer(void) {
  if (NumSaved >= MAX_SAVE)
    return(-1);

  Session[NumSaved].SaveX     = awherex();
  Session[NumSaved].SaveY     = awherey();
  Session[NumSaved].SaveColor = curcolor();
  Session[NumSaved].ScreenNum = memsavescreen();
  NumSaved++;
  return(0);
}


void LIBENTRY restscrnbuffer(void) {
  char          Temp;
  char          Color;
  bool          SaveCountLines;
  int           X;
  int           Y;
  int           Start;
  int           End;
  int           Len;
  unsigned char SaveNumLinesPrinted;
  char         *p;
  char          Str[81];
  scrntype     *ScrnBuf;

  if (NumSaved <= 0 || Session[NumSaved-1].ScreenNum < 0)
    return;

  NumSaved--;
  ScrnBuf = (scrntype *) memscreenptr(Session[NumSaved].ScreenNum);
  SaveCountLines = Display.CountLines;
  SaveNumLinesPrinted = Display.NumLinesPrinted;
  Display.CountLines = FALSE;

  End = Session[NumSaved].SaveY;
  if (Display.PageLen < Session[NumSaved].SaveY)
    Start = Session[NumSaved].SaveY - Display.PageLen - 1;
  else
    Start = 0;

  if (UseAnsi)
    printcls();

  Str[80] = 0;
  for (Y = Start; Y <= End; Y++) {
    Color = ScrnBuf->Mem[Y][0].Byte.Attrib;
    printcolor(Color);

    for (X = 0; X < 80; X++)
      Str[X] = ScrnBuf->Mem[Y][X].Byte.Value;

    if (Y == End && ! UseAnsi) {
      Str[Session[NumSaved].SaveX] = 0;
    } else {
      for (X = 79; X >= 0; X--) {
        if (Str[X] == ' ' && ScrnBuf->Mem[Y][X].Byte.Attrib <= 0x0F)
          Str[X] = 0;
        else
          break;
      }
    }

    for (X = 0, p = Str, Len = strlen(Str); X < Len; X++) {
      if (ScrnBuf->Mem[Y][X].Byte.Attrib != Color) {
        Color = ScrnBuf->Mem[Y][X].Byte.Attrib;
        Temp = Str[X];
        Str[X] = 0;
        print(p);
        p = &Str[X];
        *p = Temp;
        printcolor(Color);
      }
    }
    print(p);
    if (Y != End && Len != 80)
      newline();
  }

  if (UseAnsi) {
    sprintf(Str,"[%d;%dH",Session[NumSaved].SaveY+1,Session[NumSaved].SaveX+1);
    print(Str);
  }

  printcolor(Session[NumSaved].SaveColor);
  Display.CountLines = SaveCountLines;
  Display.NumLinesPrinted = SaveNumLinesPrinted;
  memfreescreen(Session[NumSaved].ScreenNum);
}


void LIBENTRY initdisplay(void) {
  if (ScrnBuff != NULL)
    turndisplayon(FALSE);

  ScrnBuff = NULL;
  CursorType = CUR_NORMAL;
  Control.Screen = TRUE;
  Status.ForceScreenOff = FALSE;

  memset(Session,0,sizeof(Session));
  NumSaved = 0;
}


/********************************************************************
*
*  Function:  turndisplayon()
*
*  Desc    :  Restores the video screen address & enables screen output
*/

void LIBENTRY turndisplayon(bool WriteToDisk) {
  if (ScrnBuff != NULL) {
    toggleon(ScrnBuff);
    #ifndef __OS2__
      farfree(ScrnBuff);
    #endif
    ScrnBuff = NULL;
  }

  if (CursorType == CUR_BLANK && Asy.Online != OFFLINE)
    CursorType = CUR_NORMAL;

  setcursor(CursorType);
  Control.Screen = TRUE;
  Status.ForceScreenOff = FALSE;
  if (WriteToDisk)
    makepcboardsys();
}


/********************************************************************
*
*  Function:  turndisplayoff()
*
*  Desc    :  Changes to a virtual screen and clears the screen
*
*  Notes   :  If ScrnBuff is already allocated there is something wrong so get
*             out without doing anything.  Otherwise allocate 4000 bytes to
*             hold the screen display.
*
*             If WriteToDisk - then the sysop has TOGGLED the screen off so
*             we should set Status.ForceScreenOff to TRUE and write it to disk.
*/

void LIBENTRY turndisplayoff(bool WriteToDisk) {
  if (ScrnBuff != NULL)
    return;

  #ifdef __OS2__
    ScrnBuff = (char *) 1;  /* use non-zero value to indicate screen is off */
    toggleoff(NULL);
  #else
    if ((ScrnBuff = (char _FAR_ *) farmalloc(4000)) == NULL)
      return;
    toggleoff(ScrnBuff);
  #endif

  CursorType = getcursor();
  setcursor(CUR_BLANK);
  Control.Screen = FALSE;
  if (WriteToDisk) {
    Status.ForceScreenOff = TRUE;
    makepcboardsys();
  }
}


#ifndef LIB
static void _NEAR_ LIBENTRY removeattributes(char *Dest, char *Srce) {
  for (; *Srce != 0; Dest++, Srce+=2)
    *Dest = *Srce;
  *Dest = 0;
}

/*
#define MAX_COLS            80

static void _NEAR_ LIBENTRY scottsuglylookingsourcecode(int X, int Y, char *Buf, int Len) {
  memcpy(Buf, (void _FAR_ *) (Scrn_Addr + ((Y)*MAX_COLS*2) + ((X)*2)), Len*2);
  Buf[Len*2] = 0;
}
*/


static void _NEAR_ LIBENTRY readscreenremoveattributes(int X, int Y, char *Buf, int Len) {
  char Line[162];

  readscreen2(X,Y,Line,Len);
/*scottsuglylookingsourcecode(X,Y,Line,Len);*/
  removeattributes(Buf,Line);
}


char * LIBENTRY meganum(int Num) {
  char static MegaNums[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  char static Buf[3];
  int  Tens;

  if (Num >= 36) {
    Tens = Num / 36;
    Num -= (Tens * 36);
    Buf[0] = MegaNums[Tens];
  } else
    Buf[0] = '0';

  Buf[1] = MegaNums[Num];
  Buf[2] = 0;
  return(Buf);
}

#pragma warn -par
void LIBENTRY setripmouseregion(int Num, int X1, int Y1, int X2, int Y2, int FontX, int FontY, bool Invert, bool Clear, char *Text) {
#ifdef COMM
  printcom("|1M");
  printcom(meganum(Num));
  printcom(meganum((X1-1) * FontX));
  printcom(meganum((Y1-1) * FontY));
  printcom(meganum((X2-1) * FontX));
  printcom(meganum((Y2-1) * FontY));
  printcom(Invert ? "1" : "0");
  printcom(Clear  ? "1" : "0");
  printcom("00000");  /* reserved */
  printcom(Text);
#endif
}
#pragma warn +par


int LIBENTRY checkscreenforfilenames(int StartLine, char *FileName) {
  int      Y;
  int      Len;
  long     Size;
  char     *p;
  char     Buf[14];

  for (Y = StartLine-1; Y < Status.StatusLine1; Y++) {
    readscreenremoveattributes(12,Y,Buf,11);
    Size = atol(Buf);
    if (Size > 0) {
      readscreenremoveattributes(23,Y,Buf,8);
      if (datetojulian(Buf) > 0) {
        readscreenremoveattributes(0,Y,Buf,13);
        stripright(Buf,' ');
        Len = strlen(Buf);
        if (Len <= 12) {
          if (strpbrk(Buf," ,:*<>\\") == NULL) {
            p = strchr(Buf,'.');
            if (p == NULL) {
              if (Len > 8)
                continue;
            } else {
              if (p > &Buf[8] || ((Len - (int) (p-Buf)) > 4))
                continue;
            }
            if (FileName != NULL)
              strcpy(FileName,Buf);
            return(Y+1);
          }
        }
      }
    }
  }

  if (FileName != NULL)
    FileName[0] = 0;

  return(0);
}
#endif
