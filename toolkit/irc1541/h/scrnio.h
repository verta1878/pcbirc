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


#ifndef H_SCRNIO
#define H_SCRNIO

#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

#define HELPLINES           11     /* used for the help display */
#define HELPLINEWIDTH       65     /* used for the help display */
#define HELPTITLEWIDTH      40     /* used for the help display */
#define HELPSUBTITLEWIDTH   65     /* used for the help display */

typedef enum { NOCAPS, CAPS } capstype;
typedef enum { NOCLEARFLD, CLEAR } cleartype;
typedef enum { NOCLOCK, CLOCK } clocktype;
typedef enum { SMALL, BIG } menusizetype;
typedef enum { vBOOL,vCHAR,vSTR,vUPSTR,vBYTE,vINT,vLONG,vFLOAT,vFLOATD,vDATE,vUNSIGNED,vHEXINT,vUNLONG,vHEXBYTE } vartype;
typedef enum { INPUT_DEFAULT=0, INPUT_CLEAR=1, INPUT_CAPS=2, INPUT_WRAP=4 } inputattrtype;

#if defined(__WATCOMC__) && !defined(__cplusplus)
  #undef inputattrtype
  #define inputattrtype int
#endif

/*
#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif
*/

void LIBENTRY  addmenu(int MenuNum,char *MenuItem,void (LIBENTRY *f)(void));
bool LIBENTRY  charinmask(char Ch, char *Mask);
void LIBENTRY  errorexit(char *A, char *B, char *File, int Line);
void LIBENTRY  formatdate(char *Str);
void LIBENTRY  generalscreen(char *Heading1, char *Heading2);
void LIBENTRY  getkeyflags(int HelpNum);
void LIBENTRY  initmenu(int MenuNum,bool ForwBack,char *Text,char *L,char *R);
void LIBENTRY  initscrnio(void);
char LIBENTRY  inkey(char *Ext, clocktype Clock);
void LIBENTRY  inputall(void);
void LIBENTRY  inputextend(int X,int Y,int ScrnLimit,int AnsLimit,char *Buf,char *Mask,int Attribute,int HelpNum);
int  LIBENTRY  inputline(int X,int Y,int Limit,int Pos,char *Buf,char *Mask,int Attribute,int HelpNum);
void LIBENTRY  inputnum(int X,int Y,int Limit,char *Q,void *Old, vartype VarType,int HelpNum);
void LIBENTRY  inputstr(int X,int Y,int Limit,char *Q,char *Old,char *Ans,char *Mask,int Attribute,int HelpNum);
bool LIBENTRY  keypressed(clocktype Clock);
char LIBENTRY  keyschanged(void);
void *LIBENTRY mallochk(unsigned size);
void LIBENTRY  menusel(int MenuNum, int HelpNum, menusizetype BigSize, int StartNum, bool AllowJump);
void LIBENTRY  bigmenusel(int MenuNum, int HelpNum, int StartNum, void (LIBENTRY *func)(void), int (LIBENTRY *bounds)(unsigned Num, unsigned *Offset), bool AllowJump);
void LIBENTRY  showmessage(void);
bool LIBENTRY  processcursorkeys(int *Index, long *TopLine, int *Column, long MaxLine, int MaxColumn, int LinesOnScreen);
void LIBENTRY  readcolors(void);
void LIBENTRY  removespaces(char *Str, int StrLen);
void LIBENTRY  savecolors(char *C, char *B);
void LIBENTRY  scale(long Num, long Total);
void LIBENTRY  setupscale(int Len, char Ch, int Y);
void LIBENTRY  showhelp(int HelpNum);
void LIBENTRY  showtime(void);
void LIBENTRY  showkeystatus(void);
char LIBENTRY  timedinkey(char *Ext, int Seconds);

/* private to scrnio.lib -- used by initscrn.c & colors.c */
void LIBENTRY setdefcolor1(void);
void LIBENTRY setdefcolor2(void);
void LIBENTRY setdefbandw(void);
void LIBENTRY interact(void);

#ifdef __OS2__
void LIBENTRY initshowstatus(void);
void LIBENTRY disableshowstatus(void);
void LIBENTRY enableshowstatus(void);
#endif

/*
#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif
*/

/**********************************************************/

#pragma pack(4)
struct MenuType {                   /* used to hold an array of menus */
  char First,Last;
  bool ForwBack;
  char Head[61];
  char LeftStr[21];
  char RightStr[21];
};
#pragma pack()

#pragma pack(1)
typedef struct {
  int  OffsetToTitle;
  long OffsetToHeader;
} HelpTableType;
#pragma pack()

#pragma pack(1)
typedef struct {
  char Title[HELPTITLEWIDTH+1];
} HelpTitleType;
#pragma pack()

#pragma pack(1)
typedef struct {
  char ForwBack;
  char PageNum;
  char NumPages;
  char NumLines;
} HelpHeaderType;
#pragma pack()

#pragma pack(1)
typedef struct {
  char Text[HELPLINES*(HELPLINEWIDTH+1)+(HELPSUBTITLEWIDTH+1)];
} HelpBodyType;
#pragma pack()

#pragma pack(4)
struct ScrnType {                   /* used to hold an array of input scrns */
  char Head[2][61];
  int  First,Last,MoveFactor;
  bool ClearScreen;
};
#pragma pack()

#pragma pack(4)
typedef struct {                    /* used to hold an array of input specs */
  vartype   InputType;
  char      *Mask;
  int       HelpNum;
  char      Xloc,Yloc,Flen;
  cleartype AutoClear;
  char      *Question; /* memory allocated by addquest */
  char      *Answer;   /* memory allocated by addquest */
  void      *Fptr;
  int       (*SubFunc)(int);
} FldType;
#pragma pack()

#pragma pack(4)
struct InpType {  /*  Used to pass variables between functions for Input */
  char Ch;
  char Ext;
  bool Keyed;
  bool AllCaps;
  bool AllowWrap;
  char OrgX;
  char OrgY;
  char CurX;
  int  CurXY;
  char Len;
  char ScrLimit;
  int  AnsLimit;
  int  HelpNum;
  char *Old;
  char *Ans;
  char *Mask;
};
#pragma pack()

#pragma pack(4)
typedef struct {
  int  X1;
  int  X2;
  int  Y1;
  int  Y2;
  int  Line1;
  char *Msg1;
  int  Color1;
  int  Line2;
  char *Msg2;
  int  Color2;
  int  Line3;
  char *Msg3;
  int  Color3;
  int  Line4;
  char *Msg4;
  int  Color4;
  int  QuestLine;
  char *Quest;
  char Answer[2];
  char *Mask;
  bool Save;
  bool AutoBox;
} MsgType;
#pragma pack()


/*
#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif
*/

void LIBENTRY addquest(FldType *Fields,int FieldNum,vartype VarType,int HelpN,char *InpMask,int Xl,int Yl,int Fl,char *Q,void *Fp,cleartype AutoClear,int (*Sub)(int));
void LIBENTRY initquest(FldType *Fields, int NumFields);
int  LIBENTRY readscrn(FldType *Ptr,int LastField,int StartField,char *H1,char *H2,int MoveFactor,cleartype Clr);
void LIBENTRY freescrn(FldType *Fields,int LastField);
void LIBENTRY freeanswers(FldType *Fields, int LastField);

/*
#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif
*/

/* Used to get the status of the keyboard */
#define RIGHT   0x01
#define LEFT    0x02
#define CTRL    0x04
#define ALT     0x08
#define SCROLL  0x10
#define NUM     0x20
#define CAPLCK  0x40
#define INSERT  0x80

/* Used to get the return status of the input function */
#define NOTHING     0     /* used for special cases */
#define UP          1
#define DN          2
#define RET         3
#define TAB         4
#define SHIFTTAB    5
#define ESC         6
#define PGUP        7
#define PGDN        8
#define CTRLPGUP    9
#define CTRLPGDN   10
#ifndef _DISPLAYTYPE_DEFINED
#define WORDWRAP   11
#endif
#define FLAG1      12
#define FLAG2      13
#define FLAG3      14
#define FLAG4      15
#define FLAG5      16
#define FLAG6      17
#define FLAG7      18
#define FLAG8      19
#define FLAG9      20
#define FLAG10     21
#define FLAG11     22
#define FLAG12     23


/*****  the following are simply array indexes into the variable Colors[] ****/

#define NUMCOLORS 23
enum { OUTBOX,         /*  color of outer box                        */
       STATUS,         /*  color of status info (time/date/keyboard) */
       HEADING,        /*  color of heading                          */
       MENUBOX,        /*  color of box around the menu              */
       MENUTITLE,      /*  color of menu title                       */
       MENUSELECT,     /*  color of menu selections                  */
       MENUBAR,        /*  color of menu highlight bar               */
       MENUDESC,       /*  color of menu descriptions at bottom      */
       MENUUNAVAIL,    /*  color of unavailable selection            */
       MENUUNBAR,      /*  color of highlighted unavailable selection*/
       QUESTION,       /*  color of questions                        */
       ANSWER,         /*  color of answers                          */
       EDIT,           /*  color of answers when being edited        */
       DISPLAY,        /*  color of display only fields              */
       DESC,           /*  color of descriptions at bottom of screen */
       HELPBOX,        /*  color of box around help screen           */
       HELPTITLE,      /*  color of title within help screen         */
       HELPSUB,        /*  color of sub title                        */
       HELPTEXT,       /*  color of help text                        */
       HELPDESC,       /*  color of description at bottom of help    */
       HELPKEY,        /*  color of F1 help sign at bottom of screen */
       SCALECOLOR1,    /*  color of scale on side of screen          */
       SCALECOLOR2};   /*  color of position indicator on scale      */
#endif  /* #ifndef H_SCRNIO */
