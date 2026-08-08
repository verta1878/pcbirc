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


#if defined(__OS2__) || defined(__WATCOMC__)
  #define INCL_DOSMISC
  #define INCL_DOSFILEMGR
  #include <os2.h>
#endif

#include "dosfunc.h"

#ifndef __OS2__
  #include <dos.h>
  #include <stdlib.h>
  #include <process.h>
  #ifdef DEBUG
    #include <memcheck.h>
  #endif

  #ifdef _MSC_VER
    void __cdecl __interrupt __far int24hnd(void);   /* prototype of function in INT24HND.ASM */
    void __cdecl __interrupt __far int23hnd(void);   /* prototype of function in INT24HND.ASM */
    void (__cdecl __interrupt __far * __cdecl oldint24hnd)();
    void (__cdecl __interrupt __far * __cdecl oldctrlbreak)();
    void (__cdecl __interrupt __far * __cdecl oldbiosctrlbreak)();
    #define getvect _dos_getvect
    #define setvect _dos_setvect
  #else
    #ifdef __cplusplus
      extern "C" {
      void interrupt int24hnd(...);   /* prototype of function in INT24HND.ASM */
      void interrupt int23hnd(...);   /* prototype of function in INT24HND.ASM */
      }
      void interrupt (*oldint24hnd)(...);
      void interrupt (*oldctrlbreak)(...);
      void interrupt (*oldbiosctrlbreak)(...);
    #else
      void interrupt int24hnd(void);   /* prototype of function in INT24HND.ASM */
      void interrupt int23hnd(void);   /* prototype of function in INT24HND.ASM */
      void interrupt (*oldint24hnd)(void);
      void interrupt (*oldctrlbreak)(void);
      void interrupt (*oldbiosctrlbreak)(void);
    #endif
  #endif
#endif



/********************************************************************
*
*  Function:  ctrlbreak()
*
*  Desc    :  control-break handler (currently empty)
*/

/*
void interrupt ctrlbreak(void) {
}
*/


/********************************************************************
*
*  Function:  installhandlers()
*
*  Desc    :  installs ctrlbreak and int24hnd handlers
*
*  NOTES   :  this function uses atexit() to register the handlers so that
*             upon termination of the program the handlers will automatically
*             be uninstalled.
*/

void LIBENTRY installhandlers(void) {
  #if defined(__OS2__) || defined(__WATCOMC__)
    DosError(FERR_ENABLEHARDERR|FERR_ENABLEEXCEPTION);
    DosSetMaxFH(MAXHANDLES);
  #else
    oldbiosctrlbreak = getvect(0x1B);
    oldctrlbreak     = getvect(0x23);
    oldint24hnd      = getvect(0x24);
    setvect(0x1B,int23hnd);
    setvect(0x23,int23hnd);
    setvect(0x24,int24hnd);
    atexit(uninstallhandlers);
  #endif
}


/********************************************************************
*
*  Function:  uninstallhandlers()
*
*  Desc    :  uninstalls the handlers set up by installhandlers()
*
*  NOTES   :  normally this function will be called by the exit routines
*             since it was set up with an atexit() call
*/

void uninstallhandlers(void) {
  #if defined(__OS2__) || defined(__WATCOMC__)
    DosError(FERR_ENABLEHARDERR|FERR_ENABLEEXCEPTION);
  #else
    if (oldctrlbreak != NULL) {
      setvect(0x1B,oldbiosctrlbreak);
      setvect(0x23,oldctrlbreak);
      setvect(0x24,oldint24hnd);
    }
  #endif
}


void LIBENTRY reinstallhandlers(void) {
  #if defined(__OS2__) || defined(__WATCOMC__)
    DosError(FERR_DISABLEHARDERR|FERR_ENABLEEXCEPTION);
  #else
    setvect(0x1B,int23hnd);
    setvect(0x23,int23hnd);
    setvect(0x24,int24hnd);
  #endif
}
