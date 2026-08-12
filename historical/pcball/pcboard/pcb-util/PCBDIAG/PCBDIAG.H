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

extern UData        UsersData;
extern URead        UsersRead;
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

void pascal init(void);
void pascal checkshare(void);

void pascal openprint(void);
int  pascal printtop(char Heading[]);
int  pascal printstr(char *Desc, char *Str);
int  pascal printnum(char Desc[], int Num);
int  pascal printlog(char Desc[], bool Value);
int  pascal printkey(char Desc[], char Str[]);
void pascal printdatafile(void);
void pascal printconffiles(void);
int  pascal printtextfile(char *Name, char *Desc, char ConfNum);
int  pascal openfilename(char *Name, char *Desc, char ConfNum,DOSFILE *File);
void pascal resetfilenames(void);
void pascal freefilelist(void);
void pascal printblt(char *FileName, char ConfNum);
void pascal printscr(char *FileName, char ConfNum);
void pascal printdir(char *FileName, char ConfNum);
void pascal printconfig(void);

int  pascal openusersfile(void);
// int  pascal numrecs(const int Handle, const int RecSize);
long pascal finduser(char SrchName[]);

void pascal checkpath(char Msg[], char Path[]);
void pascal checkdownloadpaths(char *Srce);
void pascal checkfile(char *FileName, int Needed, char *Desc);
void pascal checkboth(char *Name, char *Desc);
int  pascal maxfilesopen(int Current);
bool pascal checkbaudrate(long Baud);
void pascal checkcputype(void);
void pascal checkcommport(void);

int pascal getsecurity(char *Table);
bool pascal foundsecurity(char *Table, char Level, int Max);
bool pascal confregokay(char *Reg);
void pascal checkusersfile(void);

void pascal checkfsec(char *Name);
void pascal checkdoors(char *DoorsFile);
void pascal checkprot(void);

void pascal scanusersfile(int pascal (*sub)(long Counter, URead *p));

void pascal analysis(void);
void pascal analyze(void);

void pascal createsingleindex(void);
void pascal closesingleindex(void);

void pascal turnonrts(void);
void pascal turnoffrts(void);
void pascal turnondtr(void);
void pascal turnoffdtr(void);
void pascal turnonxmit(void);
void pascal turnonfifo(char TriggerLevel);
int  pascal online(void);
void pascal asyncinit(char IrqNum, int Address, void far *PtrToInBuf,void far *PtrToOutBuf,int InSize,int OutSize,bool UseCTS,bool ShareIRQ);
void pascal setport(int BaudDivisor,int DataBits);
int  pascal opencom(int BaudDivisor,int DataBits);
void pascal closecom(void);
int  pascal cgetstr(char *Buffer, int BufLen);
void pascal csendstr(char *Str, int StrLen);
