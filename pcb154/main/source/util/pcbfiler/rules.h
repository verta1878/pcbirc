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


typedef enum {EDITMOVES,SORTMOVES} editmovestype;

typedef struct {
  unsigned    Cnf;
  unsigned    Dir;
  int         Days;
  char        Name[13];
  char        InSpec[13];
  char        Criteria[1024];
} rulestype;

typedef struct {
  unsigned    SrceCnf;
  unsigned    SrceDir;
  char        Name[13];
  unsigned    TargetCnf;
  unsigned    TargetDir;
  unsigned    FileDate;
  int         Days;
  char        CopyMove;
} movestype;

int  pascal readonerule(DOSFILE *File, rulestype *Rec);
void pascal editrulesfile(char *FileName);
void pascal editrules(void);

int  pascal readonemove(DOSFILE *File, movestype *Rec);
int  pascal writeonemove(DOSFILE *File, unsigned SrceCnf, unsigned SrceDir, char *Name, unsigned TargetCnf, unsigned TargetDir, char *FileDate, unsigned Days, char CopyMove);
void pascal editmoves(void);
long pascal loadmoves(editmovestype Action);

void pascal automove(void);

int  pascal applyrulesfile(char *FileName, savescrntype *ScrnBuf);
void pascal applyrulesinit(char *DskPath, char *PathListName, unsigned BoardNumber, unsigned DirNumber);
