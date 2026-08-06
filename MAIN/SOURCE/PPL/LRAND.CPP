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


#define MULTIPLIER  0x015A4E35L
#define INCREMENT   1

static long LSeed = 1;

void slrand(unsigned long lseed)
{
    LSeed = lseed;
}

signed long int lrand(void)
{
    LSeed = MULTIPLIER * LSeed + INCREMENT;
    LSeed = (LSeed << 1) + ((LSeed & 0x80000000L) ? 1 : 0);
    return (LSeed & 0x7FFFFFFFL);
}

