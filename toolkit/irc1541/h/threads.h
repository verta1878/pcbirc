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


#ifndef H_THREADS
#define H_THREADS

#ifndef ___TYPES_HPP___
  #include <types.hpp>
#endif


#ifdef __BORLANDC__
  #define THREADFUNC _USERENTRY
#else
  #define THREADFUNC
#endif


unsigned long LIBENTRY startthread(void (THREADFUNC *func)(void *), int StackSize, void *Param);
void LIBENTRY waitthread(unsigned long ThreadId);
void LIBENTRY killthread(unsigned long ThreadId, bool Wait);

#endif  /* H_THREADS */
