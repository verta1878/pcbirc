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


#ifndef H_PCBSM_PCBFILES
#define H_PCBSM_PCBFILES
#define bool         char
#define TRUE         1
#define FALSE        0
#define PROGRAM      "pcbsm"
#define SEARCHNAM    'F'
#define SEARCHSTR    'S'
#define SEARCHSEC    'L'
#define SEARCHEXP    'E'
#define SEARCHDEL    'O'
#define BUFFERED     1
#define NONBUFFERED  0
#define PREALLOC     1
#define NONPREALLOC  0
#define INFBUFSIZE   1024

#ifdef DEMO
  #undef BIGNDX
#endif

#include "users.h"

/*************************************************************************/

typedef struct {
  long Num;
  int  Sec;
} SecTableType;

struct PackType {
  bool     Deleted;
  bool     KeepLockOut;
  unsigned NumDays;
  unsigned Since;
  unsigned RegExp;
  int      KeepSec;
  bool     Printer;
};

/*****  init.c  **********************************************************/

void pascal init(void);

/*****  acct.c  **********************************************************/

void pascal adjustacct(void);
/* 15.4: Personal PSA editor (Gender/Birthdate/Email/Web) */
void pascal adjustpersonal(void);

/*****  adjust.c  ********************************************************/

void pascal readsectable(void);
void pascal adjustsecurity(void);
void pascal copyexpiredlevel(void);
void pascal initupdncounters(void);

/*****  batch.c  *********************************************************/

void pascal batch(char *Str);
int  pascal intparam(char *Param);
unsigned pascal dateparam(char *Param);
void pascal checkforretry(int argc, char *argv[]);

/*****  colors.c  ********************************************************/

void pascal customcolors(void);

/*****  conf.c  **********************************************************/

void pascal adjustconf(void);

/*****  confmove.c  ******************************************************/

void pascal confmove(void);

/*****  defedit.c  *******************************************************/

void pascal defineeditors(void);

/*****  demo.c  **********************************************************/

#ifdef DEMO
void pascal demoscreen(void);
#endif

/*****  edit.c  **********************************************************/

void pascal edituser(long UserNum);
void pascal editusername(char *Name);
void pascal edit(void);

/*****  expdate.c  *******************************************************/

void pascal expdate(void);

/*****  index.c  *********************************************************/

void pascal createindexfiles(void);
void pascal createallindexes(void);
void pascal createallindexesprompt(void);
long pascal finduser(char *SrchName);
int  pascal adduser(char *AddName, long AddNum);
long pascal findusersoundex(char *SrchName, char *FoundStr, long *StartRecord);
void pascal removeuserfromindex(char *Name);
void pascal updateindex(char *Name, long NewRecNum);

/*****  pack.c  **********************************************************/

int pascal dopack(void);

/*****  packinpt.c  ******************************************************/

void pascal pack(void);

/*****  packon.c  ********************************************************/

int pascal packonline(void);

/*****  pcbsm.c  *********************************************************/

void pascal adjustseclevels(void);
void pascal usersmenu(void);
void pascal userinfomenu(void);
void pascal filer(void);
void pascal pcbdiag(void);
void pascal pcbsetup(void);

/*****  phone.c  *********************************************************/

void pascal standardizephone(void);

/*****  print.c  *********************************************************/

void pascal openprint(void);
int  pascal printheading(const bool Top);
int  pascal printrec(const bool PerformHeadCheck);
void pascal printlisting(void);

/*****  process.c  *******************************************************/

void pascal processusersfile(long Recs, bool ProcessSysop, bool Printer, bool UserInfo, bool DosBuffer, int pascal (*sub)(URead *p));

/*****  sort.c  **********************************************************/

void  pascal sortall(void);
void  pascal sortprimary(void);
void  pascal sortsecondary(void);
long  pascal ludratio(long NumUps, long NumDns);
long  pascal ddratio(double NumUps, double NumDns);
void  pascal sortmenu(void);

/*****  undo.c  **********************************************************/

void pascal undo(void);

/*****  users.c  *********************************************************/

void     pascal setpsaorder(void);
void     pascal usersalloc(void);
void     pascal usersdealloc(void);
void     pascal movestr(char *Dest, char *Srce, int Size);
void     pascal yymmddtostr(char Dest[], const char Srce[]);
void     pascal strtoyymmdd(char Dest[], const char Srce[]);
void     pascal decryptusersrec(URead *Rec);
void     pascal convertreadtoconfandmsgptrs(URead *Rec, char far *Conf);
void     pascal convertreadtodata(void);
void     pascal encryptusersrec(URead *Rec);
void     pascal convertconfandmsgptrstoread(URead *Rec, char far *Conf);
int      pascal convertdatatoread(char *First, char *Second);
void     pascal convertreadtostr(void);
long     pascal lockusersfile(char *Header, int Buffered);
int      pascal openusersfile(void);
int      pascal openusersinffile(char *FileName);
void     pascal closeusersfile(bool Aborted, int Buffered, bool UserInfFile);
int      pascal readusersinffile(void);
int      pascal readdosusersrecord(void);
int      pascal readusersfile(long RecNum);
int      pascal readpackinfo(void);
int      pascal writeusersfile(long RecNum, bool LockRecord, bool ClearApp);
int      pascal writeusersinffile(bool ClearApp);
long     pascal adduserrecord(void);
long     pascal numrecs(const int Handle, const int RecSize);
int      pascal copysysoprecord(int UsersBuffered, int TempBuffered);
int      pascal createtemp(int PreAlloc, int Buffered, bool UserInfo);

/*****  userinfo.c  ******************************************************/

long pascal calcconfsize(unsigned NumAreas);
int  pascal copyinfbytes(DOSFILE *Dest, DOSFILE *Srce, char *Buffer, long Bytes);
int  pascal writezeroes(DOSFILE *Dest, char *Buffer, long Bytes);
void pascal updateuserinfo(void);
void pascal createuserinfo(void);
void pascal listtpas(void);
void pascal updatetpa(void);
void pascal removetpa(void);
void pascal addpcboardtpa(void);
void pascal removepcboardtpa(void);
void pascal aliastpa(void);
void pascal addresstpa(void);
void pascal passwordtpa(void);
void pascal verifytpa(void);
void pascal statstpa(void);
void pascal notestpa(void);
void pascal accounttpa(void);
void pascal qwknettpa(void);
/* 15.4: Personal PSA installer entry */
void pascal personaltpa(void);
void pascal banktpa(void);

/*****  usernet.c  *******************************************************/

unsigned pascal foundinusernet(char *Name);
int  pascal openusernet(void);
void pascal closeusernet(void);

#endif  /* H_PCBSM_PCBFILES */
