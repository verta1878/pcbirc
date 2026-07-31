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


#ifndef H_DOSFUNC
#define H_DOSFUNC

#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

#if defined(__BORLANDC__) || defined(__TURBOC__)
  #include <dir.h>
#elif defined(__WATCOMC__)
  #include <direct.h>
  #include "borland.h"
#else
  #include <dos.h>
  #include <direct.h>
  #include <borland.h>
#endif

#if defined(_MSC_VER) && _MSC_VER >= 600 && ! defined(asm)
  #define asm _asm
#endif

#ifdef SECURE
  #define int21()  asm int 3h
#else
  #define int21()  asm int 21h
#endif

#define  MAXNAMELEN    66

#ifdef __OS2__
  #define  MAXHANDLES    30
  #define  BEGBUFSIZE    4096   /* first attempted buffer size             */
#else
  #define  MAXHANDLES    25
  #define  BEGBUFSIZE    2048   /* first attempted buffer size             */
#endif

#define  EOFBIT        0x80     /* signals file has read-past-end          */
#define  DIRTYBIT      0X40     /* signals when buffer has been WRITTEN to */

#define  OPEN_PRIVATE  0x80     /* file should be PRIVATE; no-inheritance  */

#define  OPEN_DENYRDWR 0x10     /* share mode: deny read/write (exclusive) */
#define  OPEN_DENYWRIT 0x20     /* share mode: deny write access           */
#define  OPEN_DENYREAD 0x30     /* share mode: deny read access            */
#define  OPEN_DENYNONE 0x40     /* share mode: deny none                   */

#define  OPEN_READ     0x00     /* open for read access                    */
#define  OPEN_WRIT     0x01     /* open for write access                   */
#define  OPEN_RDWR     0x02     /* open for read/write access              */
             /* these three are for dosfopen() only! */
#define  OPEN_APPEND   0x04     /* added to OPEN_WRIT - opens for APPEND   */
#define  OPEN_CREATE   0x08     /* added to OPEN_WRIT - makes a NEW file!  */
#define  OPEN_ALREADY  0x100    /* used to dup an opened file handle       */
#define  OPEN_NOERRCHK 0x200    /* used avoid error checking               */

#define  OPEN_NORMAL   0x00     /* create normal attribute file            */
#define  OPEN_RDONLY   0x01     /* create read-only file                   */
#define  OPEN_HIDDEN   0x02     /* create hidden file                      */
#define  OPEN_SYSTEM   0x04     /* create system file                      */
#define  OPEN_VOLUME   0x08     /* create volume label                     */

#define  OP_NONE       0        /* no operation                            */
#define  OP_READ       1        /* last operation was a file read          */
#define  OP_WRIT       2        /* last operation was a file write         */

#pragma pack(4)
typedef struct {      /* struct used with dosfopen, dosfgets, & dosfclose  */
  int      handle;    /* dos file handle                                   */
  char     *buffer;   /* pointer to buffer space                           */
  unsigned bufsize;   /* size of the buffer associated with this file      */
  unsigned offset;    /* offset of current pointer in buffer space         */
  unsigned end;       /* last valid character in buffer space              */
  char     status;    /* file status indicator                             */
  long     curpos;    /* current read/write position in the file           */
  char     last_op;   /* last operation (OP_NONE, OP_READ, OP_WRIT)        */
} DOSFILE;
#pragma pack()


#ifdef __OS2__
  #pragma pack(4)
  typedef struct {
    int           ExtendedError;
    unsigned long ExtendedAction;
    unsigned long ExtendedClass;
    unsigned long ExtendedLocus;
  } os2errtype;
  #pragma pack()
  /* the OS/2 functions require passing the extended error information    */
  /* because if multiple threads are used, the individual error states    */
  /* for each thread need to be handled.  DOS2ERROR declares the Os2Error */
  /* variable in the function prototype, while POS2ERROR passes a pointer */
  /* to the Os2Error variable.                                            */
  #define DOS2ERROR    ,os2errtype *Os2Error
  #define POS2ERROR    ,&Os2Error
  #define POS2ERROR2   ,Os2Error
#else
  /* the DOS functions don't pass the extended error values because the */
  /* extended error variables are global variables.                     */
  #define DOS2ERROR
  #define POS2ERROR
  #define POS2ERROR2
  extern int      ExtendedError;
  extern char     ExtendedAction;
  extern char     ExtendedClass;
  extern char     ExtendedLocus;
  extern char     Int24Error;
  extern char     Int24Flags;
#endif


extern char     OpenFileNames[MAXHANDLES+1][MAXNAMELEN];
extern char    *FileBuffers[MAXHANDLES+1];
extern unsigned MaxRetryCount;
extern char     DosErrorLevel;


/*
#if defined(__cplusplus) && ! defined(__OS2__)
extern "C" {
#endif
*/

#ifdef __OS2__
void  LIBENTRY getextendederror(os2errtype *Os2Error);
#else
void  LIBENTRY getextendederror(void);
#endif

unsigned LIBENTRY retrycount(unsigned Count, char *Path, char *Str DOS2ERROR);

char *LIBENTRY strnchr(char *StrPtr, char C, int Len);
int   LIBENTRY showerror(char *path, char *str, bool Timed DOS2ERROR);

int   LIBENTRY dosdup(int OldHandle DOS2ERROR);
int   LIBENTRY dosopen(char *FileName, int OpenFlags DOS2ERROR);
int   LIBENTRY doscreate(char *FileName, int OpenFlags, int Attributes DOS2ERROR);
int   LIBENTRY dosappend(char *FileName, int OpenFlags DOS2ERROR);
int   LIBENTRY doswrite(int handle, void *buf, unsigned len DOS2ERROR);
int   LIBENTRY dosread(int handle, void *buf, unsigned len DOS2ERROR);
void  LIBENTRY dosclose(int handle);
long  LIBENTRY doslseek(int handle, long distance, char method);
int   LIBENTRY doscommit(int Handle);
int   LIBENTRY dostrunc(int handle, long offset DOS2ERROR);

void  LIBENTRY setdosopencallback(void (LIBENTRY *cb)(char *FileName,int OpenFlags,int Handle,int ExtendedError));
void  LIBENTRY setdosreadcallback(void (LIBENTRY *cb)(char *FileName,int Handle,unsigned BytesRead,int ExtendedError));
void  LIBENTRY setdoswritecallback(void (LIBENTRY *cb)(char *FileName,int Handle,unsigned BytesWritten,int ExtendedError));
void  LIBENTRY setdosclosecallback(void (LIBENTRY *cb)(char *FileName,int Handle,int ExtendedError));
void  LIBENTRY setdosdupcallback(void (LIBENTRY *cb)(char *FileName,int NewHandle,int OldHandle,int ExtendedError));
void  LIBENTRY setdoscreatecallback(void (LIBENTRY *cb)(char *FileName,int OpenFlags,int Attributes,int Handle,int ExtendedError));

int   LIBENTRY dosfopen(char *FileName, int OpenFlags, DOSFILE *file);
int   LIBENTRY dosfgets(char *Str, unsigned MaxChars, DOSFILE *file);
int   LIBENTRY dosfugets(char *Str, unsigned MaxChars, DOSFILE *file);
int   LIBENTRY dosfputs(char *Str, DOSFILE *file);
int   LIBENTRY dosfread(void *Buffer, unsigned NumChars, DOSFILE *file);
int   LIBENTRY dosfwrite(void *Buffer, unsigned NumChars, DOSFILE *file);
long  LIBENTRY dosfseek(DOSFILE *file, long distance, char method);
void  LIBENTRY dosrewind(DOSFILE *file);
int   LIBENTRY dosflush(DOSFILE *file);
int   LIBENTRY dosftrunc(DOSFILE *file, long offset);
int   LIBENTRY dossetbuf(DOSFILE *file, unsigned BufSize);
int   LIBENTRY dosfclose(DOSFILE *file);
void  LIBENTRY closedosfopenfiles(void);
void  LIBENTRY closedosopenfiles(void);

char *LIBENTRY doserror(char *Str, int ExtendedError);

int      LIBENTRY dosappendcheck(char *path, int access);
int      LIBENTRY dosopencheck(char *path, int access);
int      LIBENTRY doscreatecheck(char *path, int access, unsigned mode);
unsigned LIBENTRY writecheck(int handle, void *buffer, unsigned count);
unsigned LIBENTRY readcheck(int handle, void *buffer, unsigned count);
int      LIBENTRY doslockcheck(int handle, long offset, long length);
int      LIBENTRY dosunlinkcheck(char *path);

void     LIBENTRY installhandlers(void);
void     LIBENTRY reinstallhandlers(void);
void              uninstallhandlers(void);

char *   LIBENTRY getopenfilename(int Handle);
bool     LIBENTRY fileisopen(char *FileName);
void     LIBENTRY say(char *Str);

void     LIBENTRY dosgetcurpath(unsigned Drive, char *Path, int PathSize);
unsigned LIBENTRY dossetcurdrive(unsigned Drive);
unsigned LIBENTRY dosgetcurdrive(void);

#ifdef __OS2__
  #define DDIRHANDLE      ,int *DirHandle
  #define DDIRHANDLE2     ,int DirHandle
  #define PDIRHANDLE      ,&DirHandle
  #define PDIRHANDLE2     ,DirHandle
  #define SYSTEMDIRHANDLE  1
  #define MAKEDIRHANDLE    0xFFFFFFFF
#else
  #define DDIRHANDLE
  #define DDIRHANDLE2
  #define PDIRHANDLE
  #define PDIRHANDLE2
#endif

int  LIBENTRY dosfindfirst(char *FileSpec, struct ffblk *ffblk, int Attrib, int *FindCount DDIRHANDLE);
int  LIBENTRY dosfindnext(struct ffblk *ffblk, int *FindCount DDIRHANDLE2);
#ifdef __OS2__
void LIBENTRY dosclosedirhandle(int DirHandle);
#endif


/* these functions should no longer be used */
#ifdef OLDFUNCTIONS
int   LIBENTRY dosfopencheck(char *Path, int OpenFlags, DOSFILE *file);
int   LIBENTRY fprintfcheck(FILE *handle);
FILE *LIBENTRY fopencheck(char *path, char *mode);
void  LIBENTRY increasehandles(void);
void  LIBENTRY decreasehandles(void);
#endif

/*
#if defined(__cplusplus) && ! defined(__OS2__)
}
#endif
*/

#endif  /* ifndef H_DOSFUNC */
