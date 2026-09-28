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



/*
    File:       envfix.c
    Created:    11/03/93
    For:        MemCheck 3.0 Professional for DOS

    Fix for putenv()-related "free on invalid ptr".
    Borland C/C++ only (tested with BC++ 3.x).

    Routines:
    ---------
    mc_envfix()     Reallocates environment block
    mc_envfree()    Installed as exit function to free env block
    _msize          Helper func to determine size of env block

    Synopsis:
    ---------
    The environment block is allocated before MemCheck
    starts up;  MemCheck, almost by definition, waits
    until the environment is defined before starting up.
    The environment block is a pointer to a table of
    strings, e.g.

        char ** environ;

    When putenv() is used, the environment block may
    be resized with a malloc-free or a realloc;
    MemCheck, not knowing about the env block ptr, will
    issue a "free on invalid pointer" message.

    Solution:
    ---------
    After MemCheck starts up, call the mc_envfix() routine
    below. This allocates a new environment block and copies
    the contents of the old.

    The routine "_msize(void *)" is used by the mc_envfix()
    function to figure out the size of the initial environment
    block.

    Finally, mc_envfix() installs an atexit() function that
    will free the environment block;  otherwise MemCheck will
    issue a memory leakage message on the buffer.

    Notes:
    ------
    All three routines below follow MemCheck Rules, doing nothing
    if MemCheck is not active, and compiling out if NOMEMCHECK is
    #defined.  Thus, the test program in main() will work correctly
    in all situations.

    This file is available in the MEMCHECK\SOURCE\MISC directory
    and on the StratosWare BBS at (313) 996-2993.
*/

#if defined(DEBUG) && ! defined(__OS2__)
#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>

#include "memcheck.h"

#define NUM_TEST_VARS   20
char *envArr[NUM_TEST_VARS];

#if defined (MEMCHECK)

/*  _msize
    Returns size given a heap pointer,
    or 0 if not found.
*/
size_t _msize (void *ptr)
{
    struct heapinfo block;      /* MC heap block */

    if ( heapcheck () < 0)
    {   mc_debug ("Borland heap corrupt in _msize!");
        return (0);
    }

    /* Init pointer to indicate start */
    block.ptr = NULL;               /* indicate first block */

    /* Walk the heap */
    while ( heapwalk ((struct heapinfo *)&block) == _HEAPOK)
    {
        if ((block.ptr == ptr) && block.in_use)
            return ((size_t) block.size);
    }

    return ( 0 );
}
#endif  /* def MEMCHECK */

void mc_envfree (void)
{
#if defined (MEMCHECK)
    if (mc_is_active ())
        free (environ);
#endif  /* def MEMCHECK */
}


/*  mc_envfix
    Call with MemCheck active before using
    putenv()'s.
*/
void mc_envfix (void)
{
#ifdef MEMCHECK
    if (mc_is_active () && environ) /* if exists--safety */
    {
        char **newEnv;
        size_t envSize;

        /* Find size of current env block */
        envSize = _msize ((void *) environ);
        /*  printf ("Size of environ is %d\n", envSize);    */

        /* If found ... */
        if (envSize)
        {
            /* Allocate space for copy... */
            newEnv = (char **)malloc (envSize);
            /* Copy ... */
            memcpy (newEnv, environ, envSize);
            /* Free old */
            RTLFREE ((void *)environ);
            /* Set new */
            environ = newEnv;
        }

        /* Free at exit, or you'll git a MemCheck message */
        atexit (mc_envfree);
    }
#endif  /* def MEMCHECK */
}


/*
    Just a "test" program that uses the
    two routines above.  Calls mc_envfix() and
    adds some environment strings to test it out.
*/
#ifdef TEST
main ()
{
    int envvar;

    /* Not needed with AutoInit, but ignored with... */
    mc_startcheck (NULL);

    /* Fix up environment; take this out & MemCheck reports err */
    mc_envfix ();

    printf ("Adding environment variables...\n");
    for (envvar = 0; envvar < NUM_TEST_VARS; envvar++)
    {
        envArr[envvar] = (char *)malloc (100);
        sprintf (envArr[envvar], "V%d=%d 1234567890", envvar, envvar);
        if (putenv ( envArr[envvar] ))
            printf ("putenv #%d failed!\n", envvar);
    }

    /* TEST */
    printf ("getenv(V0)=[%s]\n", getenv("V0"));

    /* Free malloc'd environment test strings */
    for (envvar = 0; envvar < NUM_TEST_VARS; envvar++)
        free (envArr[envvar]);

    /* Fer kicks... */
    if (heapcheck () < 0)
        printf ("Error in heap!\n");

    return 0;
}
#endif
#endif
