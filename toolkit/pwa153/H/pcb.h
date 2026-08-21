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


#ifndef H_PCB
#define H_PCB

#ifndef ___TYPES_HPP___
  #include <types.hpp>
#endif

#ifndef H_NEWDATA
  #include <newdata.h>
#endif

typedef enum {SAVEWITHOUTCHECK,SAVEWITHCHECK,ASKTOCHECK} exitsavetype;

#pragma pack(4)
typedef struct {
  char DirPath[31];
  char DskPath[31];
  char DirDesc[36];
  char SortType;
} DirListType;
#pragma pack()

#pragma pack(1)
typedef struct {
  char DirPath[30];
  char DskPath[30];
  char DirDesc[35];
  char SortType;
} DirListType2;
#pragma pack()

#pragma pack(1)
typedef struct {
  char Dev[36];
  char TextEdit[36];
  char GrphEdit[36];
} smConfigType;
#pragma pack()

#pragma pack(4)
typedef struct {
  char   Name[9];
  char   Pwrd[13];
  int    Sec;
  bool   Login;
  char   MakeUserSys;
  bool   MakeDoorSys;
  bool   Shell;
  char   Path[41];
  double ChargePerUse;
  double ChargePerMin;
  bool   Os2;
} DoorType;
#pragma pack()

#pragma pack(4)
typedef struct {
  char NamePath[31];
} ItemListType;
#pragma pack()

#pragma pack(4)
typedef struct {
  char AskPath[31];
  char AnsPath[31];
} ScriptType;
#pragma pack()

#pragma pack(1)
typedef struct {
  char  Name[15];
  char  SecLevel;
  char  File[40];
  float ChargePerUse;
  float ChargePerMin;
} cmdtype;
#pragma pack()

extern char *DatFile;
extern char VerChange;
extern bool PerformValidation;
extern bool OldCnames;


/*
#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif
*/

#ifdef LIB
int  LIBENTRY getconfrecord(unsigned ConfNum, pcbconftype *Conf);
int  LIBENTRY putconfrecord(unsigned ConfNum, pcbconftype *Conf);
void LIBENTRY closecnames(void);
#else  /* ifdef LIB */
void closecnames(void);
#endif  /* ifdef LIB */

void LIBENTRY readconfrecord(pcbconftype *Conf);
void LIBENTRY writeconfrecord(pcbconftype *Conf);
void LIBENTRY confdefaults(unsigned CurConfNum, pcbconftype *q);

void LIBENTRY resetconffile(void);
void LIBENTRY loadcnames(bool Setup);

void LIBENTRY addbackslash(char *Str, int MaxLen);
int  LIBENTRY checkexistence(char *Path, char Choice, char *Reference, bool Display);
void LIBENTRY getconf(char _FAR_ *LongStr, char _FAR_ *ShortStr);
void LIBENTRY putconf(char _FAR_ *LongStr, char _FAR_ *ShortStr);
void LIBENTRY getsmconfig(smConfigType *smConfig, char *ConfigName);
void LIBENTRY writeconfig(smConfigType *smConfig, char *ConfigName);
void LIBENTRY read120file(void);
void LIBENTRY pcbdefaults(char Drive, char * Path);
int  LIBENTRY fgetstr(char *Str, int Len);
int  LIBENTRY checkdatfile(char Ver[]);
void LIBENTRY read150file(void);
void LIBENTRY readdatfile(void);
void LIBENTRY readpcbdat(char *DefaultLoc);
void LIBENTRY writedatafile(void);
void LIBENTRY writeconffile(void);
int  LIBENTRY writefile(exitsavetype SaveType);
char * LIBENTRY parse(char *Srce);
char * LIBENTRY parsepaths(char *Srce);
int  LIBENTRY allowmore(int Num, int Max);
int  LIBENTRY srchpath(char *FileName);
bool LIBENTRY savetext(void);
void LIBENTRY getcolor(void);
bool LIBENTRY userabort(void);
bool LIBENTRY checkuserabort(void);
bool LIBENTRY getlog(void);
int  LIBENTRY getint(void);
long LIBENTRY getlong(void);
void LIBENTRY getstr(char Dest[]);
void LIBENTRY errorexittodos(char *Str);
void LIBENTRY getdays(char *Days, char *Str);
void LIBENTRY undogetdays(char *Str, char *Days);

/*
#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif
*/

#endif /* ifndef H_PCB */
