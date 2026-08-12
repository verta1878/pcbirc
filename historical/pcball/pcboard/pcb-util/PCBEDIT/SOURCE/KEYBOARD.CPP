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
/*                                KEYBOARD.CPP                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*                      Functions for keyboard handling.                      */
/*                                                                            */
/*============================================================================*/
/*                                                                            */
/*                       Written by Scott Dale Robison                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*                                                                            */
/*            Copyright (C) 1993, Clark Development Company, Inc.             */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/

// Included Files

#pragma hdrfile "keyboard.sym"

#include    <conio.h>
#include    <stdlib.h>
#include    <string.h>

#include    <system.h>

#include    <keyboard.hpp>

#include    <file.hpp>
#include    <miscedit.hpp>

#include    <pcbedit.hpp>

#pragma hdrstop

/******************************************************************************/

// Variables

int *   buf      = NULL;
int *   bufPtr   = NULL;

int     aKeys[]  = { AA, AB, AC, AD, AE, AF, AG, AH, AI, AJ, AK, AL, AM,
                     AN, AO, AP, AQ, AR, AS, AT, AU, AV, AW, AX, AY, AZ };

int     nFKeys[] = {  F10,  F1,  F2,  F3,  F4,  F5,  F6,  F7,  F8,  F9 };
int     sFKeys[] = { SF10, SF1, SF2, SF3, SF4, SF5, SF6, SF7, SF8, SF9 };
int     cFKeys[] = { CF10, CF1, CF2, CF3, CF4, CF5, CF6, CF7, CF8, CF9 };
int     aFKeys[] = { AF10, AF1, AF2, AF3, AF4, AF5, AF6, AF7, AF8, AF9 };

/******************************************************************************/

// Functions

void pascal putMacro(char * s)
{
    if ((s == NULL) || (strlen(s) == 0)) return;

    int tmpbuf[256+1], * tmpptr = tmpbuf, keyCount = 0;

    memset(tmpbuf,0,sizeof(tmpbuf));

    while (*s != NUL)
    {
        if (*s == '\\')                 // Any key code
        {
            ++s;
            if (isXDIGIT(s[0]) && isXDIGIT(s[1]) && isXDIGIT(s[2]) &&
                isXDIGIT(s[3]))
            {
                char ch = s[4], *tp;
                s[4] = NUL;
                *(tmpptr++) = (int) strtoul(s,&tp,16);
                s[4] = ch;
                s += 4;
            }
            else if (strlen(s) > 0)
                *(tmpptr++) = *(s++);
            else
                *(tmpptr++) = '\\';
        }
        else if (*s == '~')             // Normal characters
        {
            ++s;
            *s = char(toLOWER(*s));
            if (isALPHA(*s))
                *(tmpptr++) = *(s++);
            else if (isDIGIT(*s))
                *(tmpptr++) = nFKeys[*(s++)-'0'];
            else
                *(tmpptr++) = '~';
        }
        else if (*s == '#')             // Shifted characters
        {
            ++s;
            *s = char(toUPPER(*s));
            if (isALPHA(*s))
                *(tmpptr++) = *(s++);
            else if (isDIGIT(*s))
                *(tmpptr++) = sFKeys[*(s++)-'0'];
            else
                *(tmpptr++) = '#';
        }
        else if (*s == '^')             // Control characters
        {
            ++s;
            *s = char(toUPPER(*s));
            if (isALPHA(*s))
                *(tmpptr++) = *(s++)-'A'+1;
            else if (isDIGIT(*s))
                *(tmpptr++) = cFKeys[*(s++)-'0'];
            else
                *(tmpptr++) = '^';
        }
        else if (*s == '!')             // Alt characters
        {
            ++s;
            *s = char(toUPPER(*s));
            if (isALPHA(*s))
                *(tmpptr++) = aKeys[*(s++)-'A'];
            else if (isDIGIT(*s))
                *(tmpptr++) = aFKeys[*(s++)-'0'];
            else
                *(tmpptr++) = '!';
        }
//      else if (*s == '$')             // Keyboard text id's
//      {
//          ++s;
//          bsearch();
//      }
        else
            *(tmpptr++) = *(s++);
        ++keyCount;
    }

    if (bufPtr)
    {
        int *tmpbufptr = bufPtr;
        while (*(tmpbufptr++)) ++keyCount;
    }

    int * newbuf = new int[keyCount+1], *newptr = newbuf;
    if (newbuf != NULL)
    {
        tmpptr = tmpbuf;
        while (*tmpptr) *(newptr++) = *(tmpptr++);
        if (bufPtr)
        {
            while (*bufPtr) *(newptr++) = *(bufPtr++);
            delete buf;
        }
        *(newptr++) = 0;
        buf = bufPtr = newbuf;
    }
}

    /*--------------------------------------------------------------------*/

void pascal flushMacro(void)
{
    if (buf != NULL)
    {
        delete buf;
        buf = bufPtr = NULL;
    }
}

    /*--------------------------------------------------------------------*/

int pascal keyHit(void)
{
    return (kbhit() || (bufPtr != NULL));
}

    /*--------------------------------------------------------------------*/

int pascal getKey(void)
{
    unsigned ch;
    if (buf != NULL)
    {
        ch = *(bufPtr++);
        if (*bufPtr == 0)
        {
            delete buf;
            buf = bufPtr = NULL;
        }
    }
    else
    {
     // if ((ch = getch()) == 0)
     //     ch = getch() + 256;
        ch = bgetkey(0);
        if ((ch & 0x00FF) == 0)
            ch = ((ch >> 8) + 256);
        else
            ch = (ch & 0x00FF);
    }
    return ch;
}

/******************************************************************************/

