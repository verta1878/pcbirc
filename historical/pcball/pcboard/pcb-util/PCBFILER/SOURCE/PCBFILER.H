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


#ifndef H_PCBFILER
#define H_PCBFILER

#define bool       char
#define TRUE       1
#define FALSE      0
#define PROGRAM    "PCBSM"

// #define NUM_NONDUPE_DIRS 1000    /* number of non-duplicate DIR filenames    */
#define NUM_NONDUPE_LSTS  100    /* number of non-duplicate DIR.LST names    */
// #define NUM_NONDUPE_PATHS 512    /* number of non-duplicate DLPATH.LST names */
#define DIRNAME_LENGTH     33    /* DIR file names are 31 bytes              */
#define DIRLIST_LENGTH     31    /* DIR.LST file names are 33 bytes          */
#define DLPATH_LENGTH      31    /* DL PATHs are 31 bytes                    */

#define NAMESIZE   8
#define EXTSIZE    3
#define FILESIZE   NAMESIZE + EXTSIZE

// typedef enum { MISSDISK, DUPDISK, MISSDIR, DUPDIR }           StatType;

typedef enum { DIRECT, LISTING, BOTH, /* DUPLICATE */ }       ExistType;
typedef enum { TEXTLINE, DIRLINE, DUPELINE }                  LineType;
typedef enum { KEEP, MOVE, DELETE, COPYONLY, REMOVE, TAGGED } KeepType;

typedef enum { NOLOADFLAGS = 0, SHOWACTION=1, UPDATECOMBINED=2, UPDATEDUPES=4,
               LOADMOVES=8 } loadflagstype;
typedef enum { NOSAVEFLAGS = 0, OFFLINEDIR=1, SCANMOVES=2 } saveflagstype;

enum { NAME, SIZE, FDTE, FDSC, HEAD, TEXT, DUPE, DLTD, OFLN, NEWF};

typedef struct {
  uint     NumFiles;
  uint     RecOffset[26];
  char     Reserved[73];
  char     Id;
} idxhdrtype1;

typedef struct {
  ulong    NumFiles;
  ulong    RecOffset[26];
  char     Reserved[19];
  char     Id;
} idxhdrtype2;

typedef union {
  idxhdrtype1 Type1;
  idxhdrtype2 Type2;
} idxhdrtype;

typedef struct {
  char     Name[NAMESIZE];
  char     Ext[EXTSIZE];
  uint     PathNum;
} nametype1;

typedef struct {
  char     Name[NAMESIZE];
  char     Ext[EXTSIZE];
  ulong    PathNum;
  ulong    FileSize;
} nametype2;

typedef union {
  nametype1 Type1;
  nametype2 Type2;
} nametype;

typedef struct {
  char Path[64];
} pathtype;

/*****  pcbfiler.c  ******************************************************/

void pascal editmenu(void);

/*****  action.c  ********************************************************/

void pascal showaction(void);
void pascal startaction(char ShowChar, bool ShowMem, int Max);
void pascal resetaction(char ShowChar, bool ShowMem, int Max);

/*****  arcv.asm  ********************************************************/

extern "C" int ARCV(char far *filename);

/*****  batch.c  *********************************************************/

void pascal checkforretry(int argc, char *argv[]);
void pascal batch(char *Str);

/*****  confmenu.c  ******************************************************/

void pascal confmenu(void);

/*****  ctod2.c  *********************************************************/

unsigned pascal ctod2(char *Date);

/*****  defaults.c  ******************************************************/

void pascal savedefaults(void);
void pascal getdefaults1(void);
void pascal getdefaults2(void);

/*****  dircolor.c  ******************************************************/

void pascal choosedircolors(void);

/*****  dirfiles.c  ******************************************************/

int pascal includedirfiles(char *DskPath, int ConfNum, int DirNum, loadflagstype Flags);

/*****  dupes.c  *********************************************************/

void dupeschanged(void);

/*****  editdirs.c  ******************************************************/

void pascal editdirlist(unsigned BoardNumber, unsigned DirNumber, bool GetArea, bool PromptForConf);

/*****  editexit.c  ******************************************************/

char pascal editexit(char *Str);

/*****  editor.c  ********************************************************/

int pascal checkconf(unsigned CnfNum);
int pascal checkdir(unsigned CnfNum, unsigned DirNum);
int pascal editdir(char *Desc, char *DskPath, char *PathListName, unsigned BoardNumber, unsigned DirNumber);

/*****  findpath.c  ******************************************************/

int pascal fastfinddirpath(unsigned DirNum, DirListType *Rec, pcbconftype *Conf, unsigned NumTextDirs, int DirLstFile);
int pascal finddirpath(unsigned BoardNum, unsigned DirNum, DirListType *Rec);

/*****  findtext.c  ******************************************************/

void pascal massagesrchcriteria(void);
void pascal removecodes(char *Str);
/* bool pascal foundinlist(char *DirPath, char *List); */
int  pascal showline(char *Str, int Color);
void pascal scandirs(void);
void pascal scandisk(void);

/*****  getdiz.c  ********************************************************/

int  pascal getdates(char *FileName, unsigned *OldestDate, unsigned *NewestDate);
bool pascal wildcardinsidefile(char *FileName, char *FileSpec);
int  pascal extractdiz(char *FileName);

/*****  idxname.asm  *****************************************************/

extern "C" void pascal makeidxname(char *FileName, char *IdxName);

/*****  init.c  **********************************************************/

void pascal init(void);
int  pascal numrandrecords(char *FileName, int RecSize);

/*****  preedit.c  *******************************************************/

void pascal confgetdirtoedit(void);
void pascal maingetdirtoedit(void);
void pascal readofflinedir(void);
#ifndef DEMO
void pascal processdirfile(unsigned BoardNumber, unsigned DirNumber, bool ScanDlPath, char *ApplyRulesName);
#endif

/*****  process.c  *******************************************************/

char * pascal makebitarray(char *Str);
int  pascal openchangewrite(void);
int  pascal openchangeread(void);
void pascal process(void);
void pascal makeallindexfiles(void);
void pascal makefileslist(void);

/*****  readdir.c  *******************************************************/

int pascal readdirfile(char *FileName, int ConfNum, int DirNum, loadflagstype Flags);

/*****  savedir.c  *******************************************************/

bool pascal alreadyinlist(char *FileName);
int  pascal backupfile(char *FileName, char SortType);
void pascal undolastchange(void);
int  pascal savedir(DirListType *SaveRec, char *PathListName, int CnfNum, int DirNum, saveflagstype Flags);
void pascal printscroll(char *Str, int Color);

/*****  scandupe.c  ******************************************************/

void pascal noworktodo(char *Str);
void pascal scandupe(void);

/*****  shell.c  *********************************************************/

int  pascal findfile(char *FileName, char *DskPath, char *PathListName);
void pascal fileview(char *FileName);
void pascal shelltodos(void);
void pascal shelltoprogram(char *Program, char *Param);

/*****  showfree.c  ******************************************************/

void pascal showfreespace(void);

/*****  sort.c  **********************************************************/

void pascal writedirfile(char *FileName);
void pascal sortchanged(void);
int pascal sortinplace(char Order, bool UpdateCombined);

/*****  verify.c  ********************************************************/

void pascal convertspectoname(char *Name, nametype *File);
void pascal verifyfiles(char VerifyType, char *DskPath, char *PathListName);
int  pascal verifyexist(char VerifyType, char *DskPath, char *PathListName, char *Name);
int  pascal findfileindlpathrecord(char *PathRec, char *Token);

/*****  zipv.c  **********************************************************/

int  pascal ListZipFile(char *ZipFileName);

/*************************************************************************/

#endif //ifndef H_PCBFILER
