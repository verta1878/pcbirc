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


/******************************************************************************/

// Note:  The File I/O sections exist to port Borland C++ functions to
//        equivalent CDC functions

/******************************************************************************/

// File I/O Included Files

#if defined(__OS2__) && (defined(__WATCOMC__) || defined(__BORLANDC__))
  #include <stdarg.h>
#endif

#include    <errno.h>
#include    <fcntl.h>
#include    <mem.h>
#include    <share.h>
#include    <stdio.h>
#include    <sys\stat.h>

#ifdef __WATCOMC__
  #define SH_DENYNONE SH_DENYNO
#endif

#include    <pcbmisc.hpp>

/******************************************************************************/

// File I/O Defined Macros

#if defined(__BORLANDC__) && ! defined(__OS2__)
  #undef      errno
#endif

/******************************************************************************/

// File I/O Variables

bool fileAppendMode[50];

#ifdef __OS2__
  #define ExtendedError Os2Error.ExtendedError
#else
  extern int ExtendedError;
#endif

/******************************************************************************/

// File I/O Functions

//extern "C" {

    /*--------------------------------------------------------------------*/

long filelength(int handle)
{
    long fpos = doslseek(handle,0,SEEK_CUR);

    if (fpos == -1)
        return -1;

    long flen = doslseek(handle,0,SEEK_END);
    fpos = doslseek(handle,fpos,SEEK_SET);

    if (flen == -1 || fpos == -1)
        return -1;

    return flen;
}

    /*--------------------------------------------------------------------*/

int chsize(int handle, long size)
{
    long flen = filelength(handle);
    #ifdef __OS2__
       os2errtype Os2Error;
    #endif

    if (flen == -1)
        return -1;

    if (flen < size)
    {
        char buf[1024];
        clearBuf(buf);
        doslseek(handle,0,SEEK_END);
        while ((size-flen) >= 1024)
        {
            doswrite(handle,buf,1024 POS2ERROR);
            flen += 1024;
        }
        doswrite(handle,buf,unsigned(size-flen) POS2ERROR);
    }
    #ifndef LIB
    else if (flen > size)
    {
        dostrunc(handle,size POS2ERROR);
    }
    #endif

    flen = filelength(handle);

    if      (flen == -1)
        return -1;
    else if (flen != size)
    {
        errno = EACCES;
        return -1;
    }

    return 0;
}

    /*--------------------------------------------------------------------*/

#if defined(__OS2__) && (defined(__WATCOMC__) || defined(__BORLANDC__))
int sopen(const char * path,int access,int shflag, ...)
#else
int sopen(char * path, int access, int shflag, int mode)
#endif
{
    int handle = -1;
    int OpenFlags = 0;
    #ifdef __OS2__
       os2errtype Os2Error;
    #endif

    if      ((access & O_RDWR)   == O_RDWR)
        OpenFlags = OPEN_RDWR;
    else if ((access & O_WRONLY) == O_WRONLY)
        OpenFlags = OPEN_WRIT;
    else if ((access & O_RDONLY) == O_RDONLY)
        OpenFlags = OPEN_READ;

    switch (shflag)
    {
        case SH_COMPAT:
        case SH_DENYRW:
            OpenFlags |= OPEN_DENYRDWR;
            break;

        case SH_DENYWR:
            OpenFlags |= OPEN_DENYWRIT;
            break;

        case SH_DENYRD:
            OpenFlags |= OPEN_DENYREAD;
            break;

        case SH_DENYNONE:
            OpenFlags |= OPEN_DENYNONE;
            break;
    }

    // The following access flags are ignored:
    // O_NDELAY, O_BINARY, O_TEXT

    if      (((access & O_CREAT) == O_CREAT) && (fileexist((char *) path) == 255))
    {

        int Attributes;
        #if defined(__OS2__) && (defined(__WATCOMC__) || defined(__BORLANDC__))
          int mode;
          va_list args;
          va_start(args,shflag);
          mode = va_arg(args,int);
        #endif

        Attributes = OPEN_NORMAL;
        if ((mode & S_IWRITE) != S_IWRITE) Attributes |= OPEN_RDONLY;
        handle = doscreate((char *) path,OpenFlags,Attributes POS2ERROR);
    }
    else if (((access & O_CREAT) == O_CREAT) && (fileexist((char *) path) != 255) &&
             ((access & O_EXCL)  == O_EXCL))
    {
        handle = -1;
        #ifndef __WATCOMC__
          errno = EINVACC;
        #endif
    }
    else
    {
        handle = dosopen((char *) path,OpenFlags POS2ERROR);
    }

    if (handle != -1)
        fileAppendMode[handle] = FALSE;
    else
        errno = EACCES;

    if      (((access & O_TRUNC)  == O_TRUNC)  && (handle != -1))
    {
        chsize(handle,0);
    }
    else if (((access & O_APPEND) == O_APPEND) && (handle != -1))
    {
        doslseek(handle,0,SEEK_END);
        fileAppendMode[handle] = TRUE;
    }

    return handle;
}

    /*--------------------------------------------------------------------*/

int dup(int handle)
{
    #ifdef __OS2__
      os2errtype Os2Error;
    #endif

    return(dosdup(handle POS2ERROR));
}

    /*--------------------------------------------------------------------*/

int close(int handle)
{
    dosclose(handle);
    return 0;
}

    /*--------------------------------------------------------------------*/

long lseek(int handle, long offset, int fromwhere)
{
    return doslseek(handle,offset,fromwhere);
}

    /*--------------------------------------------------------------------*/

int read(int handle, void * buf, unsigned len)
{
    #ifdef __OS2__
       os2errtype Os2Error;
    #endif

    int stat = dosread(handle,buf,len POS2ERROR);

    if (stat == -1) errno = ExtendedError;

    return stat;
}

    /*--------------------------------------------------------------------*/

int write(int handle, void * buf, unsigned len)
{
    #ifdef __OS2__
       os2errtype Os2Error;
    #endif

    if (fileAppendMode[handle]) doslseek(handle,0,SEEK_END);

    int stat = doswrite(handle,buf,len POS2ERROR);

    if (stat == -1) errno = ExtendedError;

    return stat;
}

    /*--------------------------------------------------------------------*/

//}

/******************************************************************************/

