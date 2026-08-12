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


void pascal zsort(char huge *base, long nel, unsigned width, int (*comp)(const void huge *a, const void huge *b), void (*cswap)(char huge *a, char huge *b));
char huge * pascal zsearch(void *Key, void huge *Base, long Num, unsigned Len, int (*comp)(void));

void zswapint(unsigned huge *a, unsigned huge *b);
void zswaplong(long huge *a, long huge *b);
void zswapstr(char huge *a, char huge *b);
void zswapvir(char huge *a, char huge *b);

extern void huge *ZcompKey;  /* defined in ZSORT.C-used in comp() function of both zsort() and zsearch() */
extern void huge *ZcompPtr;  /* defined in ZSORT.C-used in comp() function of both zsort() and zsearch() */
