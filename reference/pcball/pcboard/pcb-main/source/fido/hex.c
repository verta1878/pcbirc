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


#ifdef FIDO

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>

#include <misc.h>
#include <pcb.h>
#include <pcboard.h>
#include <screen.h>

#include "defines.h"
#include "structs.h"
#include "prototyp.h"

#ifdef DEBUG
#include <memcheck.h>
#endif


//static void _NEAR_ LIBENTRY DecHex (char _FAR_ *string, unsigned long number);

unsigned long LIBENTRY HexDec (char _FAR_ *string)
{
	int len = strlen (string)-1;
	int x;
	unsigned char value;
	unsigned long spot;
	unsigned long num=0;

	spot=1;
	for (x=len; x>=0; x--) {
		value = toupper (*(string+x));
		if (value>='A' && value<='F')
			num+=(10+(value-'A'))*spot;
		else
			num+=(value-'0')*spot;
		spot<<=4;
	}
	return (num);
}

/*
static void _NEAR_ LIBENTRY DecHex (char _FAR_ *string, unsigned long number)
{
	ultoa (number, (char *) string, 16);
}

*/
int LIBENTRY iHexDec (char _FAR_ *string)
{
	int len = strlen (string)-1;
	int x;
	unsigned char value;
	int spot;
	int num=0;

	spot=1;
	for (x=len; x>=0; x--) {
		value = toupper (*(string+x));
		if (value>='A' && value<='F')
			num+=(10+(value-'A'))*spot;
		else
			num+=(value-'0')*spot;
		spot<<=4;
	}
	return (num);
}

#endif
