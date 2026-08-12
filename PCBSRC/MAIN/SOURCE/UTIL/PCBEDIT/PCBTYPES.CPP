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


#include    <ctype.h>

inline int  isDISP(int c)
{
    return ((c >= 0) && (c <= 255));
}

    /*--------------------------------------------------------------------*/

inline int  isASCII(int c)
{
    return (isDISP(c) && isascii(c));
}

    /*--------------------------------------------------------------------*/

int  isALNUM(int c)
{
    return (isASCII(c) && isalnum(c));
}

    /*--------------------------------------------------------------------*/

int  isALPHA(int c)
{
    return (isASCII(c) && isalpha(c));
}

    /*--------------------------------------------------------------------*/

int  isDIGIT(int c)
{
    return (isASCII(c) && isdigit(c));
}

    /*--------------------------------------------------------------------*/

int  isXDIGIT(int c)
{
    return (isASCII(c) && isxdigit(c));
}

    /*--------------------------------------------------------------------*/

int  isUPPER(int c)
{
    return (isASCII(c) && isupper(c));
}

    /*--------------------------------------------------------------------*/

int  isLOWER(int c)
{
    return (isASCII(c) && islower(c));
}

    /*--------------------------------------------------------------------*/

int  toUPPER(int c)
{
    return (isASCII(c) ? toupper(c) : c);
}

    /*--------------------------------------------------------------------*/

int  toLOWER(int c)
{
    return (isASCII(c) ? tolower(c) : c);
}

