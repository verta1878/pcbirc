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


#include <dos.h>
#include <borland.h>
#include <alloc.h>
#include <stdlib.h>
#include <string.h>
#include "_process.h"
#include "swap.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

/*------------------------------------------------------------------------
 * filename - dosenv.c
 *
 * function(s)
 *        __DOSenv - Prepare Spawn/Exec environment
 *-----------------------------------------------------------------------*/

/*
 *      C/C++ Run Time Library - Version 5.0
 *
 *      Copyright (c) 1987, 1992 by Borland International
 *      All Rights Reserved.
 *
 */

/*-----------------------------------------------------------------------*

Name            DOSenv -- Prepare Spawn/Exec environment

Usage           char * pascal __DOSenv(char **envV,
                                       char *pathP,
                                       void **envSave);

Prototype in    _process.h

Description     This function allocates  a buffer and fill it  with all the
                environment  strings one  after  the  others. If  the pathP
                variable is nonzero, then it is  appended to the end of the
                end of  the environment assumed it  is for a spawn  or exec
                purpose.


Return value    DOSenv  returns a  pointer  to  the environment  buffer if
                successful, and NULL on error.

-------------------------------------------------------------------------*/
static char * pascal near __DOSenv(char **envV, char *pathP, void **envSave)
{
        register char   **envW;
        register unsigned       envS;
        char    *bufP;

/*      Compute the environment size including  the NULL string at the
        end of the environment. (Environment size < 32 Kbytes)
*/
        envS = 1;
        if ((envW = envV) != NULL)
                for (envS = 0; *envW && **envW; envS += strlen(*envW++) + 1)
                        ;
        envS++;
        if (pathP)
                envS += 2 + strlen(pathP) + 1;
        if (envS >= 0x2000)
                return (NULL);

/*      Allocate a buffer
*/
        if ((bufP = (char *) malloc(envS + 15)) != NULL) {

/*              The environment MUST be paragraph aligned
*/
                *envSave = bufP;
                bufP += 15;
                (*((unsigned *)&(bufP))) &= 0xFFF0;

/*              Concatenate all environment strings
*/
                if ((envW = envV) != NULL && *envW != NULL)
                        while (*envW && **envW)
                {
                                bufP = stpcpy(bufP, *envW++);
                                *bufP++ = '\0';
                }
        else
            *bufP++ = '\0';
                *bufP++ = '\0';

/*              Append program name to the environment
*/
                if (pathP) {
                        *((short *)bufP) = 1; bufP += sizeof(short);
                        bufP = stpcpy(bufP, pathP);
                        *bufP++ = '\0';
                }
                return bufP - envS;
        }
        else
                return NULL;
}


int swapenv (char *program_name,
             char *command_line,
             char *exec_return,
             char *swap_fname) {

  char *p;
  void *envsave;
  int  retval;

  envsave = NULL;
  #if __BORLANDC__ >= 0x500
    p = __DOSenv(_environ,"", &envsave);
  #else
    p = __DOSenv(environ,"", &envsave);
  #endif

  /*
     if p is NULL that's OKAY!
     NULL means use parent environment instead of Borland's environment
  */

  retval = swap(program_name,command_line,exec_return,swap_fname,p);

  if (envsave != NULL)
    free(envsave);

  return(retval);
}
