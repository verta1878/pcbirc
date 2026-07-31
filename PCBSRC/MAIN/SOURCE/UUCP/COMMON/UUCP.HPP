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



#ifndef ___UUCP_HPP___

#define ___UUCP_HPP___

/******************************************************************************/

#include    <stdlib.h>
#include    <string.h>

#include    <pcbtools.h>
#undef bool
#undef TRUE
#undef FALSE

#include    <misc.h>

#include    <dosclass.hpp>

//#include    <memcheck.h>

/******************************************************************************/

#define     defInitStrBuf(n,l)  char n [ l + 1 ]; n[0] = NUL

/******************************************************************************/

const int   IN_EMAIL    = 1;
const int   IN_JUNK     = 2;
const int   UN_MOD_NEWS = 3;
const int   UN_PUB_NEWS = 4;
const int   FIDO_TYPE   = 5;

/******************************************************************************/

// extern bool uucpRole;
   extern char uucpSystem  [   8 + 1 ];
   extern int  uucpDebug;
   extern bool uucpQuiet;
// extern long uucpRate;
// extern int  uucpPort;
// extern bool uucpAnon;
// extern bool uucpRevCon;
   extern int  uucpResTime;
   extern char uucpName    [  16 + 1 ];
   extern bool uucpOffHook;
   extern char uucpConfs   [ 128 + 1 ];
   extern bool uucpForce;
   extern long uucpFileSize;
   extern bool uucp7E1;
   extern bool uucpNoJunk;
   extern bool uucpLongTo;
   extern bool uucpDecode;
   extern int  uucpBusyTries;
   extern bool uucpCurDate;
   extern bool uucpGiveup;
   extern char uucpGrade;
   extern int  uucpWaitDays;

#define uucpJunkOut uucpOffHook

/******************************************************************************/

extern "C" {

long   pascal getticks     ( void );

}

/******************************************************************************/

void     pascal initialize       ( void );
void     pascal deinitialize     ( void );

void     pascal munge            ( char * d, char * s, char * n );

char *   pascal maxstr           ( char * s, int l );

unsigned pascal dstrlen          ( char * s );
void     pascal dstrcpy          ( char * d, char * s, unsigned l );

int      pascal validatefile     ( char * path, char * result );
int      pascal validatedir      ( char * path, char * result );

bool     pascal validfile        ( char * file );

void     pascal maxstrcat        ( char * Dest, char * Srce, int Max );

void     pascal parseName        ( char * nname, char * sname, unsigned ssize,
                                   char * lname, unsigned lsize,
                                   int lookupAlias = FALSE);

void     pascal sepsToSpaces     ( char * s );

/******************************************************************************/

inline char * sstrdup(char * s)
{
    return (s ? strdup(s) : NULL);
}

    /*--------------------------------------------------------------------*/

inline void sstrfre(char * & s)
{
    if (s)
    {
        free(s);
        s = NULL;
    }
}

    /*--------------------------------------------------------------------*/

inline size_t sstrlen(char * s)
{
    return (s ? strlen(s) : 0);
}

    /*--------------------------------------------------------------------*/

inline void strTrunc(char * str, char chr)
{
    char * p = strchr(str,chr);
    if (p) p[0] = NUL;
}

    /*--------------------------------------------------------------------*/

inline void spacesToSeps(char * s)
{
    change(s,' ',PcbData.uucpSeparator[0]);
}

    /*--------------------------------------------------------------------*/

inline char * sstripall(char * s, char c)
{
    return (s ? stripall(s,c) : s);
}

    /*--------------------------------------------------------------------*/

inline bool exist(char * f)
{
    return ((fileexist(f) & 0x18) ? FALSE : TRUE);
}

/******************************************************************************/

#endif

