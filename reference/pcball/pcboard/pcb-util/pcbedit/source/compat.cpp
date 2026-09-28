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



#ifdef __OS2__

#define     INCL_DOSPROCESS
#include    <os2.h>

#else

#include    <dos.h>

void DosBeep ( unsigned long frequency, unsigned long duration )
{
    sound(unsigned(frequency));
    delay(unsigned(duration));
    nosound();
}

void DosSleep ( unsigned long duration )
{
    delay(unsigned(duration));
}

#endif

void doSound ( unsigned long frequency, unsigned long duration )
{
    DosBeep(frequency,duration);
}

void doDelay ( unsigned long duration )
{
    DosSleep(duration);
}

