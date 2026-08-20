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
/*                                                                            */
/*                                PCBMISC.CPP                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*     Miscellaneous functions fashioned after the PCB Toolkit  (I hope!)     */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                                 Written by                                 */
/*                             Scott Dale Robison                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1994 - Clark Development Company, Inc.            */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

// Pragmas

#ifndef __OS2__
//#pragma inline
#endif

/******************************************************************************/

// Included Files

#include    <ctype.h>
#include    <mem.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <assert.h>

#include    <new.h>

#include    "pcbmisc.hpp"

#include    "newscr.h"
#include    "menu.h"
#ifdef DEBUGSCR
#include    <memcheck.h>
#endif

/******************************************************************************/

// Defined Macros

/******************************************************************************/

// Types

/******************************************************************************/

// Constants

/******************************************************************************/

// Variables
#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

//sSTATUS Status;

#endif

#if defined(LIB) && defined(___EXEC___)

URead * TempRead   = NULL;
UData * TempData   = NULL;

char  * TempReg    = ConfReg;
long  * TempMsgPtr = MsgReadPtr;

#endif

//unsigned long int outbmal = 0;

/******************************************************************************/

// Function Prototypes

/******************************************************************************/

// Inline Functions

/******************************************************************************/

// Functions

#if defined(LIB) && !defined(MENU)

#pragma argsused
int LIBENTRY doMenu(char * f, int nt)
{
    return 0;
}

#endif

    /*--------------------------------------------------------------------*/

#if defined(LIB) && defined(___EXEC___)

#pragma argsused
void   LIBENTRY checkdlfiles            ( int save ) { }

bool   LIBENTRY filestuffinuse          ( void ) { return FALSE; }
char   LIBENTRY bytesleftinkbdbuffer    ( void ) { return 0; }
char   LIBENTRY bytesleftinpplbuffer    ( void ) { return 0; }

long   LIBENTRY lowmsgnum               ( void ) { return 0; }
long   LIBENTRY highmsgnum              ( void ) { return 0; }

#pragma argsused
void   LIBENTRY substenvinfile          ( char * f, int l ) { }

#pragma argsused
int LIBENTRY gettpaheader(char * KeyWord, apptype * AppHeader)
{
    return -1;
}

// #pragma argsused
// long   LIBENTRY diskfreespace           (char * path) {return 0;}


    /*--------------------------------------------------------------------*/
    /*--------------------------------------------------------------------*/
#pragma argsused

int  LIBENTRY msgbasestats(msgbasestattype *MsgStats)
{
    return 0;
}


    /*--------------------------------------------------------------------*/
#pragma argsused
bool LIBENTRY canbeavailable(void)
{
    return 0;
}

    /*--------------------------------------------------------------------*/



#pragma argsused
int LIBENTRY gettparec(apptype * AppHeader, long RecNum, void * Rec)
{
    return -1;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY puttparec(apptype * AppHeader, long RecNum, void * Rec)
{
    return -1;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY gettpaconfrec(apptype * AppHeader, long RecNum, unsigned ConfNum, void * Rec)
{
    return -1;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY puttpaconfrec(apptype * AppHeader, long RecNum, unsigned ConfNum, void * Rec)
{
    return -1;
}

    /*--------------------------------------------------------------------*/

bool LIBENTRY insearch(void)
{
    return TRUE;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY tokenscan(char * Input, char * CodedInput, bool CaseSensitive)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
bool LIBENTRY parsersearch(char * Text, int TextLen, char * CodedInput,
    bool RetVal, int LastToken)
{
    return FALSE;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY stopsearch(void)
{
}

    /*--------------------------------------------------------------------*/

void LIBENTRY printfoundtext(char * Str)
{
    printxlated(Str);
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY checkscreenforfilenames(int StartLine, char * FileName)
{
    if (FileName) *FileName = '\0';
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY setripmouseregion(int Num, int X1, int Y1, int X2, int Y2,
    int FontX, int FontY, bool Invert, bool Clear, char * Text)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY setnewpassword(userrectype * urp, char * NewPwrd)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY setnewalias(userrectype * urp, char * Alias, long urn)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

char * LIBENTRY meganum(int Num)
{
    static char buf[4+1];
    strupr(itoa(Num,buf,36));
    if (strlen(buf) < 2)
    {
        buf[2] = '\0';
        buf[1] = buf[0];
        buf[0] = '0';
    }
    else
    {
        strcpy(buf,buf+strlen(buf)-2);
    }
    return buf;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY setlanguage(int LangNum)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY updateuserinformation(void)
{
}

    /*--------------------------------------------------------------------*/

void LIBENTRY makepcboardsys(void)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY tempuseralloc(bool AllocMsgPtrs)
{
    return -1;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY tempuserdealloc(void)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY puttemprecord(long RecNo, dirtytype Flag)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY gettemprecord(long RecNo)
{
    return -1;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY convertdatatoread(UData * pud, URead * pur)
{
    *pur = *pud;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY convertreadtodata(URead * pur, UData * pud)
{
    *pud = *pur;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
long LIBENTRY numrecs(int Hand, int Size)
{
    return 0L;
}

    /*--------------------------------------------------------------------*/

void LIBENTRY clearpplbuffer(void)
{
}

    /*--------------------------------------------------------------------*/

void LIBENTRY closeautostuff(void)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY xlatetext(char * New, char * Old)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY pcbcopyfile(char * Srce, char * Dest, bool CheckForEOF,
    bool ShowActivity, char * LockDrives, bool CopyAttributes)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY allocatefilelist(xfertype Mode)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY addfiletolist(char * Name, char * Path, long Size,
    bool ForceIntoList, bool Showdupe)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY send(int NumTokens)
{
}

    /*--------------------------------------------------------------------*/

void LIBENTRY getdisplaynames(void)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY entermessagefromfile(unsigned ConfNum, char * ToName,
    char * FromName, char * Subject, char Security, unsigned PackDate,
    bool ReqReceipt, bool Echo, char * FileName)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY joinconfcommand(int NumTokens)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY doquestionnaire(int Qnum)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
int LIBENTRY showblt(int BltNum)
{
    return 0;
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY dirmenu(int NumTokens)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY opendoor(int NumTokens)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY stuffbufferstr(char * Str)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY stuffpplbuffer(char * Str)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY stuffkbdwithfile(char * FileName)
{
}

    /*--------------------------------------------------------------------*/

void LIBENTRY byecommand(void)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY broadcast(int NumTokens)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY readusernetrecord(unsigned NodeNum, nodetype * Buf)
{
}

    /*--------------------------------------------------------------------*/

#pragma argsused
void LIBENTRY updateusernetrecord(unsigned NodeNum, nodetype * Buf)
{
}

#endif

    /*--------------------------------------------------------------------*/

#if defined(___COMP___) || defined(___SCAN___)

#pragma argsused
unsigned LIBENTRY retrycount(unsigned count, char * path, char * str)
{
    return 0xFFFF;
}

#endif

    /*--------------------------------------------------------------------*/

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

void LIBENTRY newline(void)
{
    putchar('\n');
}

#endif

    /*--------------------------------------------------------------------*/

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

void LIBENTRY print(char * s)
{
    printf("%s",s);
}

#endif

    /*--------------------------------------------------------------------*/

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

void LIBENTRY println(char * s)
{
    print(s);
    newline();
}

#endif

    /*--------------------------------------------------------------------*/

#if defined(___COMP___) || defined(___INFO___) || defined(___SCAN___)

void LIBENTRY printxlated(char * s)
{
    while (*s != '\0')
    {
        if (isATX(s))
        {
            s += 4;
        }
        else if (memcmp(s,"@OPTEXT@",8) == 0)
        {
            print(Status.DisplayText);
            s += 8;
        }
        else
        {
            putchar(*(s++));
        }
    }
}

#endif

    /*--------------------------------------------------------------------*/

int LIBENTRY isATX(char * s)
{
    return ((*(s++) == '@') && (*(s++) == 'X') &&
        (isdigit(*s) || isupper(*s)) && isxdigit(*(s++)) &&
        (isdigit(*s) || isupper(*s)) && isxdigit(*s));
}

    /*--------------------------------------------------------------------*/

int LIBENTRY getATX(char * s)
{
    char tmp = *(s+4);
    *(s+4) = '\0';
    int attr = int(strtol(s+2,NULL,16));
    *(s+4) = tmp;
    return attr;
}

    /*--------------------------------------------------------------------*/

#if defined(DEBUGSCR) && defined(DBGNEWDEL)

void far * operator new(unsigned long size)
{
    if (size == 0) size = 1;
    return farmalloc(size);
}

    /*--------------------------------------------------------------------*/

#if defined(__TINY__) || defined(__SMALL__) || defined(__MEDIUM__)

void operator delete(void far * ptr)
{
    farfree(ptr);
}

#endif

    /*--------------------------------------------------------------------*/

void * operator new(unsigned size)
{
    if (size == 0) size = 1;

    void * ptr;

    void (*cur_new_handler)(void) = set_new_handler(NULL);

    do {
        ptr = malloc(size);
        unsigned long int i = mc_alloc_count();
        if(i == 6365)
          i = 0;
        if (ptr != NULL) break;

        if (cur_new_handler != NULL)
            cur_new_handler();
        else
            break;

    } while (ptr == NULL);

    set_new_handler(cur_new_handler);
    return ptr;
}

    /*--------------------------------------------------------------------*/

void operator delete(void * ptr)
{
    free(ptr);
}

#endif

    /*--------------------------------------------------------------------*/

#ifdef  ___EXEC___

void LIBENTRY ttyforward(int x)
{
    #ifdef __OS2__
       char * sp = (char *) Scrn_Buf;
    #else
      char far * sp = (char far *) Scrn_Addr;
    #endif

    sp += awherey()*160;
    sp += awherex()*2;

    char buf[80+1], * ptr = buf;

    for (int i = 0; i < x; ++i)
    {
        *(ptr++) = *(sp++);
        ++sp;
    }

    *ptr = '\0';

    print(buf);
}

    /*--------------------------------------------------------------------*/

bool LIBENTRY lastChar(char * s, char c)
{
    // If no string was passed in return immediately
    if ((s == NULL) || (*s == '\0')) return FALSE;

    // Return TRUE if c is the last character in s, FALSE otherwise
    return (s[strlen(s)-1] == c);
}

    /*--------------------------------------------------------------------*/

void LIBENTRY dispString(char * s, int nt)
{
    // If no string was passed in return immediately
    if ((s == NULL) || (*s == '\0')) return;

    // Buffers to split s into file and parameter fields
    char str   [ 128+1 ];
    char file  [ 128+1 ];
    char parms [ 128+1 ];

    // Make a duplicate copy of s and strip spaces from right
    maxstrcpy(str,s,sizeof(str));
    stripright(str,' ');

    // See if the last character is an underscore and strip if appropriate
    bool suppress = lastChar(str,'_');
    if (suppress) str[strlen(str)-1] = '\0';

    // Copy str to file
    maxstrcpy(file,str,sizeof(file));

    // Find the first space in file
    char * p = strchr(file,' ');

    // If a space was found
    if (p)
    {
        // Copy everthing after it to parameters and terminate the file field
        maxstrcpy(parms,p+1,sizeof(parms));
        *p = '\0';
    }
    // Otherwise
    else
    {
        // Terminate the parameters field because there are none
        *parms = '\0';
    }

    // Strip leading spaces from the parameters field
    stripleft(parms,' ');

    // Assume it isn't a file
    bool fileFlag = FALSE;

    // See if it is a file
    switch (*file)
    {
        case '%': // Is it a display file spec?
        case '!': // Is it a PPE     file spec?
        case '$': // Is it a MNU     file spec?
            #ifndef LIB
            fileFlag = checkenvfileexist(file+1,sizeof(file)-1) != 255;
            #endif
            break;
    }

    // Now process the line
    if (fileFlag)
    {
        // If parameters were specified then tokenize them
        if (strlen(parms) > 0) nt = tokenizestr(parms);

        switch (*file)
        {
            case '%': // Is it a display file spec?
                displayfile(file+1,
                    displayfiletype(GRAPHICS|SECURITY|LANGUAGE));
                break;

            case '!': // Is it a PPE     file spec?
                doScript(file+1,NULL,nt);
                break;

            case '$': // Is it a MNU     file spec?
                doMenu(file+1,nt);
                break;
        }
    }
    else
    {
        printxlated(s);
    }
}

#endif

    /*------------------------------------------------------------------*/

#ifdef ___EXEC___

void LIBENTRY clearBiosKbd(void)
{
#if defined(__OS2__) || defined(__WATCOMC__)
    /* Watcom/OS2: use BIOS keyboard function via int386 */
    while (_bios_keybrd(_KEYBRD_READY))
        _bios_keybrd(_KEYBRD_READ);
#else
TOP:

    asm mov ah,0x01 // Report Whether Character Ready
    asm int 0x16

    asm jz  EXIT    // No keypress pending so exit

    asm mov ah,0x00 // Read Next Keyboard Character
    asm int 0x16

    goto TOP;       // Do it all again

EXIT:
#endif
}

#endif

    /*--------------------------------------------------------------------*/

char * LIBENTRY pcbstrstr(char * s1, char * s2)
{
    return ((s1 && s2) ? strstr(s1,s2) : NULL);
}

    /*--------------------------------------------------------------------*/

char * LIBENTRY pcbstrchr(char * s, char c)
{
    return ((s && c) ? strchr(s,c) : NULL);
}

    /*--------------------------------------------------------------------*/

char * LIBENTRY pcbstrdup(char * s)
{
    return (s ? strdup(s) : NULL);
}


#ifndef ___COMP___
char * LIBENTRY bstrdup (char * s)
{
    if(!s)
      return NULL;

    char * str = (char *) malloc (strlen(s) +1);          // Used to be BMALLOC

    if(str)
      strcpy(str,s);

    return str;
}
#endif

/******************************************************************************/

