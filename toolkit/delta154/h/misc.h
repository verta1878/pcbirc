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


#ifndef H_MISC
#define H_MISC

#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

#ifdef SECURE
  #define int21()  asm int 3h
#else
  #define int21()  asm int 21h
#endif

#define ERRNOTDIR       1
#define ERRNOTFILE      2
#define ERRNOTFOUND     3
#define ERRDIRNOTFOUND  4
#define ERRDISKINVALID  5
#define ERRPATHINVALID  6

#define COPYFILE_ERROPENSRCE  1
#define COPYFILE_ERROPENDEST  2
#define COPYFILE_ERRMEMALLOC  3
#define COPYFILE_ERRREADSRCE  4
#define COPYFILE_ERRWRITDEST  5
#define COPYFILE_DRIVEINUSE   6

#define PRNREADY     0x80
#define PRNPAPEROUT  0x20
#define PRNSELECTED  0x10

#pragma pack(1)
typedef struct {
  char          Attrib;
  unsigned int  Time;
  unsigned int  Date;
  unsigned long Size;
  char          Name[13];
} dtalisttype;
#pragma pack()

#pragma pack(4)
typedef struct {
  unsigned int  w0;
  unsigned int  w1;
  unsigned int  w2;
  unsigned int  w3;
} dlong;
#pragma pack()


/*
#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif
*/

#ifndef BASREAL
#define BASREAL 1
typedef unsigned char pasreal[6];
typedef unsigned char bassngl[4];
typedef unsigned char basdble[8];
typedef union {
          double        value;
          unsigned char byte[8];
        } IEEEdouble;
double LIBENTRY bassngltodouble(bassngl OldNum);
long   LIBENTRY bassngltolong(bassngl Num);
double LIBENTRY basdbletodouble(basdble OldNum);
long   LIBENTRY basdbletolong(basdble Num);
ulong  LIBENTRY basdbletoulong(basdble Num);
void   LIBENTRY doubletobasdble(basdble New, double Old);
void   LIBENTRY doubletobassngl(bassngl New, double Old);
long   LIBENTRY doubletolong(void *Num);
void   LIBENTRY doubletopasreal(pasreal *New, double Old);
void   LIBENTRY longtobasdble(basdble New, long Num);
void   LIBENTRY ulongtobasdble(basdble New, long Num);
void   LIBENTRY longtobassngl(bassngl New, long Num);
void   LIBENTRY longtodouble(char *F, long Num);
void   LIBENTRY longtopasreal(char *F, long Num);
double LIBENTRY pasrealtodouble(pasreal OldNum);
long   LIBENTRY pasrealtolong(void *Num);
#endif

void LIBENTRY maketable(char *Table, char *Srch, char SrchLen);
int  LIBENTRY bmsearch(char *Buf, int BufLen, char *Table, char *Srch, char SrchLen);
int  LIBENTRY bmisearch(char *Buf, int BufLen, char *Table, char *Srch, char SrchLen);


#pragma pack(4)
typedef struct {
  char Key[80];
  char Table[256];
  char KeyLen;
} psearchtype;
#pragma pack()

psearchtype * LIBENTRY getparsersearcharray(void);
int  LIBENTRY getparsersearchcount(void);
void LIBENTRY setparsersearch(int Num, psearchtype *Array);
void LIBENTRY resetparsersearch(void);
int  LIBENTRY tokenscan(char *Input, char *NewInput, bool CaseSensitive);
bool LIBENTRY parsersearch(char *Text, int TextLen, char *Input, bool RetVal, int LastToken);
int  LIBENTRY searchfirst(char *Str, int *FoundLen);
void LIBENTRY stopsearch(void);
bool LIBENTRY insearch(void);

void buildstr(char *Dest, ...);



void     LIBENTRY abortpgm(char *FileName, int LineNum, char *Str);
void     LIBENTRY addchar(char Str[], char Ch);
int      LIBENTRY appendfile(char *Srce, char *Dest, bool CheckForEOF);
bool     LIBENTRY alldigits(char *Buffer);
void     LIBENTRY ascii(char *Str, int Num);
void     LIBENTRY asciispace(char *Str, int Num);
void     LIBENTRY lascii(char *Str, long Num);
void     LIBENTRY breakdate(unsigned short *Numbers, char *DateStr);
char *   LIBENTRY commastr(char *Buffer, bool Negative);/* do not use this one */
char *   LIBENTRY comma(char *Buffer, long Num);
char *   LIBENTRY ucomma(char *Buffer, unsigned long Num);
char *   LIBENTRY dcomma(char *Buffer, double Num);
int      LIBENTRY copyfile(char *Srce, char *Dest, bool CheckForEOF);
int      LIBENTRY copyfilesandpaths(char *SrcePath, char *DestPath, void (*display)(char *FileName));
int      LIBENTRY cputype(void);
unsigned int LIBENTRY ctod(char *Date);
unsigned LIBENTRY currentminute(void);
char     LIBENTRY currentsecond(void);
unsigned LIBENTRY dostimetofiletime(void);
void     LIBENTRY dadd(dlong *Arg1, dlong *Arg2, dlong *Sum);
unsigned short LIBENTRY datetojulian(char DateStr[]);
int      LIBENTRY dayofweek(int Mon, int Day, int Year);
int      LIBENTRY dayofweektoday(void);
int      LIBENTRY dcmp(dlong *Arg1, dlong *Arg2);
long     LIBENTRY ddiv(dlong *Dividend, long Divisor);
char *   LIBENTRY decimal(char *Str, unsigned long Value);
void     LIBENTRY decrypt(char *Str, int Len);
void     LIBENTRY decrypt2(char *Str, int Len);
#define decrypt3(Str,Len) encrypt3(Str,Len)
void     LIBENTRY deletefiles(char *FileSpec);
int      LIBENTRY directory(dtalisttype *List, char Mask[], int SearchAttribute, int MaxEntries);
unsigned long LIBENTRY diskfreespace(char *Path);
long     LIBENTRY dlongtolong(dlong *Arg1);
void     LIBENTRY dmul(long Arg1, long Arg2, dlong *Product);
bool     LIBENTRY driveok(char Drive);
void     LIBENTRY dsub(dlong *Arg1, dlong *Arg2, dlong *Sum);
char *   LIBENTRY dtoc(int Date, char *Temp);
void     LIBENTRY encrypt(char *Str, int Len);
void     LIBENTRY encrypt2(char *Str, int Len);
void     LIBENTRY encrypt3(char *Str, int Len);
char *   LIBENTRY endofstring(char *Str, int MaxLen);
int      LIBENTRY equalwilds(char S1[], char S2[]);
long     LIBENTRY evaluate(void _FAR_ *p, unsigned Size);
long     LIBENTRY exacttime(void);
long     LIBENTRY getfiledatetime(char *FileName);
unsigned short LIBENTRY getjuliandate(void);
unsigned char LIBENTRY fileexist(char *FileName);
int      LIBENTRY findfour(char *Base, char *Srch);
char *   LIBENTRY findstartofname(char *Path);
void     LIBENTRY formatwild(char Name[]);
void     LIBENTRY fullyqualifiedname(char *QualifiedName, char *FileName, int QLen);
int      LIBENTRY hextoint(char *Str);
int      LIBENTRY index(char *s,char c);
int      LIBENTRY isset(void _FAR_ *BitStream, int BitNum);
char *   LIBENTRY juliantodate(unsigned short JD);
int      LIBENTRY lastcharinstr(char *Line, char NotChar);
void     LIBENTRY leftstr(char *Dest, char *Srce, int Len);
void     LIBENTRY ltodlong(long Arg1, dlong *Arg2);
void     LIBENTRY makefilenameunique(char *NewName, char *Original);
void     LIBENTRY maxstrcpy(char *Dest, char *Srce, int Max);
void     LIBENTRY midstr(char *Dest, char *Srce, int Start, int Len);
int      LIBENTRY movefile(char *Srce, char *Dest);
int      LIBENTRY checkmovefile(char *Srce, char *Dest);
void     LIBENTRY padstr(char *Str, char C, int MaxLen);

bool     LIBENTRY printerready(int LptNum);
void     LIBENTRY proper(char *Str);
void     LIBENTRY rightstr(char *Dest, char *Srce, int Len);
void     LIBENTRY setbit(void _FAR_ *BitStream, int BitNum);
bool     LIBENTRY shareloaded(void);

/* WARNING:  soundex() expects to see all uppercase letters */
void     LIBENTRY soundex(char *Dest, char *Srce);

unsigned LIBENTRY strtominutes(char *Str);
char *   LIBENTRY stripall(char *Str, char Ch);
char *   LIBENTRY stripboth(char *Str, char Ch);
char *   LIBENTRY stripleft(char *Str, char Ch);
char *   LIBENTRY stripright(char *Str, char Ch);
bool     LIBENTRY substitute(char *Str, char *Search, char *Replace, int MaxLen);
bool     LIBENTRY testforshareloaded(char *FileName);
bool     LIBENTRY timeinrange(char *Start, char *Stop);
long     LIBENTRY timesten(char *Str);
char *   LIBENTRY ttoc(int Time, char *Temp);
void     LIBENTRY unsetbit(void _FAR_ *BitStream, int BitNum);
int      LIBENTRY validatesemantics(char *Path);
void     LIBENTRY wildcard(char _FAR_ *Field, int Len);

#ifndef __OS2__
/* these functions have not been ported to OS/2 */
void LIBENTRY initexitfunctions(int NumFunc);
void LIBENTRY doexitfunctions(void);
void LIBENTRY pushexitfunction(void (*function)(void));
void LIBENTRY popexitfunction(void);
#endif

unsigned LIBENTRY doRLE(char *Dest, char *Srce, unsigned Len);
unsigned LIBENTRY unRLE(char *Dest, char *Srce, unsigned Len);

void LIBENTRY change(char *Str, char Old, char New);

#ifdef __OS2__
  #define farmemcpy(d,s,l) memcpy(d,s,l)
  #define fmemset(d,c,l)   memset(d,c,l)
#else
  void LIBENTRY farmemcpy(void _FAR_ *Dest, void _FAR_ *Srce, int Len);
  void LIBENTRY fmemset(void _FAR_ *dest,char c,unsigned len);
#endif


/*
#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif
*/


#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif
int LIBENTRY xmodem(char *String, int Len);
#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif

#endif  /*ifndef H_MISC */
