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


#ifndef bool
  #define bool  char
  #define TRUE  1
  #define FALSE 0
#endif

#define _FARDATA_ far

#ifndef H_DOSFUNC
  #include <dosfunc.h>
#endif

#ifndef H_USERS
  #include "users.h"
#endif

#ifndef H_PCB
  #include <pcb.h>
#endif

#define movestr(dest,srce,size) { memcpy(dest,srce,size); dest[size] = 0; }

extern char         PCBVer[];
extern char         SMVer[];
extern char         ConfigName[66];
extern DOSFILE      prn;
extern smConfigType smConfig;

extern int          ColSize1;
extern int          ColSize2;
extern int          HighestHandle;
extern char         CurrentPath[40];
extern char         RemoteFile[66];
extern char         CompressFile[66];
extern char         BoardBatFile[66];
extern char         QwkCompressFile[66];

extern UData _FARDATA_ UsersData;
extern URead _FARDATA_ UsersRead;
extern bool         ShareStatus;
extern IndexType    Index;

extern pcbconftype  OldConf;
extern pcbconftype  Conf;

extern int  OverrunErrors;
extern int  ParityErrors;
extern int  FramingErrors;
extern int  InBytes;
extern int  OutBytes;
extern char RTSoff;
extern char B16550;
extern char B8250;
extern char CTSokay;

void init(void);
void checkshare(void);

void openprint(void);
int   printtop(char Heading[]);
int   printstr(char *Desc, char *Str);
int   printnum(char Desc[], int Num);
int   printlog(char Desc[], bool Value);
int   printkey(char Desc[], char Str[]);
void printdatafile(void);
void printconffiles(void);
int   printtextfile(char *Name, char *Desc, char ConfNum);
int   openfilename(char *Name, char *Desc, char ConfNum,DOSFILE *File);
void resetfilenames(void);
void freefilelist(void);
void printblt(char *FileName, char ConfNum);
void printscr(char *FileName, char ConfNum);
void printdir(char *FileName, char ConfNum);
void printconfig(void);

int  openusersfile(void);
// int   numrecs(const int Handle, const int RecSize);
long finduser(char SrchName[]);

void checkpath(char Msg[], char Path[]);
void checkdownloadpaths(char *Srce);
void checkfile(char *FileName, int Needed, char *Desc);
void checkboth(char *Name, char *Desc);
int   maxfilesopen(int Current);
bool  checkbaudrate(long Baud);
void checkcputype(void);
void checkcommport(void);

int  getsecurity(char *Table);
bool  foundsecurity(char *Table, char Level, int Max);
bool  confregokay(char *Reg);
void checkusersfile(void);

void checkfsec(char *Name);
void checkdoors(char *DoorsFile);
void checkprot(void);

// void scanusersfile -- local version in SCANUSER.C

void analysis(void);
void analyze(void);

void createsingleindex(void);
void closesingleindex(void);

void turnonrts(void);
void turnoffrts(void);
void turnondtr(void);
void turnoffdtr(void);
void turnonxmit(void);
void turnonfifo(char TriggerLevel);
int   online(void);
void asyncinit(char IrqNum, int Address, void far *PtrToInBuf,void far *PtrToOutBuf,int InSize,int OutSize,bool UseCTS,bool ShareIRQ);
void setport(int BaudDivisor,int DataBits);
int   opencom(int BaudDivisor,int DataBits);
void closecom(void);
int   cgetstr(char *Buffer, int BufLen);
void csendstr(char *Str, int StrLen);
