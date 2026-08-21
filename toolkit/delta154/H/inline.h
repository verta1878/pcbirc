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


/* these header file overcomes a bug in the "BC++ for OS/2" v2.0 compiler   */
/* which would fail to inline functions regardless of the #pragma intrinsic */
/* or /Oi settings and would only inline them if the function calls had the */
/* internal form, such as __strcpy__().                                     */

#define strcpy(d,s)      __strcpy__(d,s)
#define memset(d,c,l)    __memset__(d,c,l)
#define memcmp(s1,s2,n)  __memcmp__(s1,s2,n)
#define strcmp(s1,s2)    __strcmp__(s1,s2)
#define strlen(s)        __strlen__(s)
#define memchr(s,c,n)    __memchr__(s,c,n)
#define strcat(d,s)      __strcat__(d,s)
#define strchr(s,c)      __strchr__(s,c)
#define strrchr(s,c)     __strrchr__(s,c)

/* it has been found that this function, when expanded inline, will fail to
   copy from 1 to 3 bytes on the end unless the length is an even multiple of 4
#define memcpy(d,s,l)    __memcpy__(d,s,l)
*/
