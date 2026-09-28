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


// Header file for COMMON.CPP
//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
#ifndef MaxModem
  #define MaxModem 500         // 500 different manufacturers & modems
#endif

#ifndef word
  typedef unsigned word;
#endif

#ifndef READ
  #define READ     OPEN_READ|OPEN_DENYNONE
  #define READWR   OPEN_RDWR|OPEN_DENYNONE
  #define WRITE    OPEN_WRIT|OPEN_DENYNONE
  #define CREATE   OPEN_WRIT|OPEN_CREATE|OPEN_DENYNONE
#endif

extern  char MFile   [  60 ],
             TempStr [ 120 ],
             Mans    [ 190 ][ 15 ],
             Names   [  54 ][ 25 ];

extern  char       Head1 [];
extern  char       Head2 [];
extern  char    VerifyStr[];

extern  bool     local;
extern  int      ManCount;
extern  int      DefaultNum;
extern  unsigned CurrentModem;
extern  unsigned Date;
extern  long     Offset;
extern  long     ModemdataOffset;
extern  DOSFILE  DataFile;

void  title(void);
void  anykey(void);
void  initprogram(void);
void  createarray(bool OpenFile);

void  pascal quit(int ErrNum, char *Module);

int   selectmodem(void);
int   selectmanuf(void);
int   pascal selectname(int DataOffset);
void  pascal getmdmdata(bool isDefault);

int   sortmanuf(const void *l, const void *r);
int   sortname(const void *l, const void *r);
int   sortmodemnum(const void *l, const void *r);

void  pascal dcrypt(bool Which);
void  pascal ecrypt(bool Which);
int   pascal srchpath(char *FileName);

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�
