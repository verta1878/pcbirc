/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
#include <conio.h>
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



#include    <constrea.h>

#include    <uucp.hpp>

/******************************************************************************/

extern int       ExtendedError;
extern char      ExtendedAction;
extern char      ExtendedClass;
extern char      ExtendedLocus;

extern char      Int24Error;
extern char      Int24Flags;

extern constream conerr;

extern cDOSFILE  logFile;

/******************************************************************************/

const int ESC = 27;

inline int keyhit ( void ) { return kbhit(); }

inline int getkey ( void )
{
    int k = getch();
    if (k == 0) k = getch()+0x0100;
    return k;
}

/******************************************************************************/

char * LIBENTRY doserror ( char * Str, int ExtendedError );

// extern "C" {

int pascal showerror(char * path, char * str, bool timed)
{
    static bool InHere = FALSE;

    if (timed)
    {
        settimer(3,18);
        while (gettimer(3) >= 0)
        {
            giveup();
            if (keyhit() && (getkey() == ESC))
            {
                settimer(4,-1);
                break;
            }
        }
    }

    if (!InHere)
    {
        InHere = TRUE;
        conerr << endl << '(' <<
            int(ExtendedError)  << ':' <<
            int(ExtendedClass)  << ':' <<
            int(ExtendedLocus)  << ':' <<
            int(ExtendedAction) << ") ";
        conerr << "Error " << str << " File " << path << ' ' <<
            '(' << doserror(NULL,ExtendedError) << ')';
        if (logFile.isOpen())
            logFile.printf("*** (%d:%d:%d:%d) Error %s File %s (%s)\r\n",
                int(ExtendedError), int(ExtendedClass),
                int(ExtendedLocus), int(ExtendedAction),
                str, path, doserror(NULL,ExtendedError));
        InHere = FALSE;
    }
    return 0;
}

unsigned pascal retrycount(unsigned count, char * path, char * str)
{
    if (ExtendedError == 83)
    {
        ExtendedError  = Int24Error+19;
        ExtendedAction = Int24Flags;
    }

    if (ExtendedAction > 2)
    {
        showerror(path,str,FALSE);
        return 0xFFFFU;
    }

    tickdelay(4);

    ++count;
    if (count < 8) return count;

    if (count == 8)
    {
        conerr << endl << "Network delay";
        if (logFile.isOpen()) logFile.putln("*** Network delay");
        settimer(4,20*182/10);
        showerror(path,str,TRUE);
    }
    else
    {
        tickdelay(18);
        if (gettimer(4) < 0) return 0xFFFFU;
    }

    return count;
}

// }

/******************************************************************************/

