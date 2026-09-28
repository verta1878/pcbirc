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


#ifndef H_SCREEN
#define H_SCREEN

#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

typedef enum {VID_NONE,VID_MDA,VID_CGA,VID_EGA,VID_VGA,VID_MCGA} vidcardtype;
typedef enum {VID_MONO,VID_COLOR} vidmodetype;
typedef enum {NMT= 0, DDOS=2, TV=4, MOS=6, WIN=8, OS2=10, WIN4=12} taskertype;
typedef enum {SMALLFONT,BIGFONT} fonttype;
typedef enum {KEEPSCREEN,FREESCREEN} savefreetype;

#ifdef SECURE
  #define int21()  asm int 3h
#else
  #define int21()  asm int 21h
#endif

/*  These defines are used with  box()  to select the LineType  */

typedef enum {BLANK,SINGLE,DOUBLE,HDOUBLE,VDOUBLE} boxlinetype;

/*  These defines are used with  setcursor()  to select the CursorType */

#define CUR_BLANK   0
#define CUR_NORMAL  1
#define CUR_BLOCK   2
#define CUR_INSERT  3

/*  Global variables, initially set in getmode()  */

#if defined(__OS2__) || defined(__WATCOMC__)
extern long Scrn_Addr;
// extern unsigned long __far16 *Scrn_Addr;
extern void *Scrn_Buf;
#else
extern long Scrn_Addr;
#endif

extern char Scrn_Rtrc;
extern char Scrn_Mode;
extern char Scrn_ColorCard;
extern char Scrn_EGA;
extern char Scrn_Box;
extern char Scrn_24Hour;
extern char Scrn_DateSeparator;
extern char Scrn_BottomRow;
extern bool Novell;
extern bool DisableGiveup;
extern vidcardtype Scrn_Adapter;
extern taskertype Tasker;

extern int   Scrn_SizeBytes;
extern short Scrn_Size16;
extern int   Scrn_Size32;

#if defined(__OS2__) || defined(__WATCOMC__)
extern long Scrn_Addr;
extern char Scrn_X;
extern char Scrn_Y;
#endif

/*  Structures, variables and functions necessary for windows  */

#pragma pack(4)
typedef struct {
  char X;
  char Y;
  char *Str;
  char Color;
} ScrType;
#pragma pack()


#ifdef OLDWINDOWFUNCTION
struct WindType {
  int  _FAR_ *P;                       /* pointer to save screen buffer */
  char WindX1,WindY1,WindX2,WindY2;  /* window coordinates            */
  char CursorX,CursorY;              /* cursor location               */
  int  CursorStatus;                 /* cursor status (scan lines)    */
  char BorderType;                   /* border used by box()          */
  char BorderColor;                  /* color of border               */
};
#endif  /* OLDWINDOWFUNCTIONS */


#pragma pack(4)
typedef struct {
  char Screen[8000];    /* Maximum of 8000 bytes for video memory */
} savescrntype;
#pragma pack()


#pragma pack(1)
typedef union {
  short Word;
  struct {
    char Value;
    char Attrib;
  } Byte;
} celltype;
#pragma pack()



/*
#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif
*/

/*  NOTE: you MUST define  WindType Windows[xxx]  in the main program */

#ifdef OLDWINDOWFUNCTIONS
void LIBENTRY defwindow(int Num,char X1,char Y1,char X2,char Y2,char Type,char Color,int _FAR_ *Sp);
void LIBENTRY openwindow(int Num);
void LIBENTRY closewindow(int Num);
#endif  /* OLDWINDOWFUNCTIONS */


/*  routines found in SCREEN.LIB */

void   LIBENTRY box(int X1,int Y1,int X2,int Y2,char Color,boxlinetype LineType);
void   LIBENTRY boxcls(char X1,char Y1,char X2,char Y2,char Color,boxlinetype LineType);
void   LIBENTRY checkmultitaskers(void);
void   LIBENTRY cls(void);
void   LIBENTRY clsbox(char X1, char Y1, char X2, char Y2, char Color);
void   LIBENTRY clscolor(char Color);
char   LIBENTRY cursecond(void);
void   LIBENTRY deletechar(int X,int Y,char C,int Len);
void   LIBENTRY fastcenter(int Y, char Str[80], char Attr);
void   LIBENTRY fastprint(int X,int Y,char Str[80],char Attr);
void   LIBENTRY fastprintmove(int X, int Y, char Str[80], char Attr);
void   LIBENTRY fastprintv(int X,int Y,char Str[80],char Attr);
void   LIBENTRY fastputc(int Loc,char C);
void   LIBENTRY fastputch(int Loc, char C, char Color);
int    LIBENTRY getcursor(void);
void   LIBENTRY getmode(void);
char   LIBENTRY getrows(void);
long   LIBENTRY getseconds(void);
void   LIBENTRY growbox(char X1,char Y1,char X2,char Y2,char Color,boxlinetype LineType,int DelayTime);
int    LIBENTRY insertchar(int X, int Y, char C, int Len);
void   LIBENTRY memfreescreen(int ScreenNum);
int    LIBENTRY memsavescreen(void);
void * LIBENTRY memscreenptr(int ScreenNum);
void   LIBENTRY memrestorescreen(int ScreenNum, savefreetype Status);
void   LIBENTRY memfreeallscreens(void);
void   LIBENTRY scrollup(int X1, int Y1, int X2, int Y2, char Color);
void   LIBENTRY scrolldn(char X1, char Y1, char X2, char Y2, char Color);
void   LIBENTRY setatt(int X1,int Y1,int X2,int Y2,char Color);
void   LIBENTRY setcursor(int CursorType);
void   LIBENTRY setdelay(void);
void   LIBENTRY setrows(int NumRows);
void   LIBENTRY showscreen(ScrType *p, int NumLines);
void   LIBENTRY readscreen(int X,int Y,char Str[80],char Attr,int Len);
void   LIBENTRY readscreen2(int X, int Y, char *Str, int Len);
void   LIBENTRY updatevirtualscreen(void);
int    LIBENTRY xyvalue(int X, int Y);

#ifndef NOCURSOR
// these functions were actually written clear back in the days of Turbo C 1.0
// when TC did not yet come with functions by these names (but TP did).  If
// you use the CONIO.H header you'll get conflicts with these functions.  To
// avoid the problem, define #NOCURSOR and recompile.
void   LIBENTRY gotoxy(int x,int y);
char   LIBENTRY wherex(void);
char   LIBENTRY wherey(void);
#endif

fonttype LIBENTRY getfont(void);
void LIBENTRY setfont(fonttype Font);

void LIBENTRY mysound(int Freq, int Duration);
void LIBENTRY beep(void);

void LIBENTRY twodigit(int *Num);
void LIBENTRY twodigit0(int *Num);
char * LIBENTRY datestr(char * DStr);
char * LIBENTRY timestr1(char TStr[9]);
char * LIBENTRY timestr2(char TStr[6]);
char LIBENTRY timechanged(void);

/*
#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif
*/

#if defined(__OS2__) || defined(__WATCOMC__)
extern long Scrn_Addr;
  const int SCRN_25LINES = 25;     // all 25 lines
  const int SCRN_50LINES = 50;
  typedef enum {UPDATE_NONE= -1, UPDATE_MIXED=0, UPDATE_25LINES=25, UPDATE_50LINES=50} scrnupdttype;
  void LIBENTRY updatelines(scrnupdttype ScrnUpdate, int StartLine, int EndLine);
  void LIBENTRY updatelinesnow(void);
  int  LIBENTRY getviolines(void);
  void LIBENTRY setviolines(int NumLines);
  void LIBENTRY setscreenupdateinterval(int Interval);
  void LIBENTRY hidescreen(void);
  void LIBENTRY unhidescreen(void);
#else
  #define updatelinesnow()
  #define setscreenupdateinterval(i)
  #define hidescreen()
  #define unhidescreen()
#endif

#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif
void LIBENTRY giveup(void);
void LIBENTRY savescreen(savescrntype _FAR_ *TempScrnBuff);
void LIBENTRY restorescreen(savescrntype _FAR_ *TempScrnBuff);
void LIBENTRY mydelay(int Hundredths);
#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif


#endif   /* ifndef H_SCREEN */
