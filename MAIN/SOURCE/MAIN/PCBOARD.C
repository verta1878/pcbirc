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


#include "project.h"
#pragma hdrstop

// MC_SET_TRACKF(trackf_all);

#include <setjmp.h>

#pragma warn -use
static char *Copyright  = "Copyright (C) 1996 Clark Development Company, Inc";

#ifndef __OS2__
extern unsigned int _stklen = 0x8800;  //lint !e509
#ifdef CPU386
long BC386BUG;  // this variable can be removed after Borland fixes their bug!  (see bug.h for details)
#endif
#endif

/** GLOBAL VARIABLES *************************************************/
#if defined(__LARGE__) || defined(__COMPACT__)
statustype far Status;
#else
statustype Status;
#endif
asytype     Asy;
disptype    Display;
controltype Control;
int         prn;
jmp_buf     TopOfProgram;
/*********************************************************************/


#ifdef __WATCOMC__
  int _argc;
  char **_argv;
#endif


#pragma argsused
int main(int argc, char **argv) {
  #ifdef DEBUG
    mc_startcheck(erf_standard);
  //mc_startcheck(NULL);
  #endif

  #ifdef __WATCOMC__
    _argc = argc;
    _argv = argv;
  #endif

  initnontransient();
  setjmp(TopOfProgram);      //lint !e534

  #ifdef KBD3
    setint3();
  #endif

  init();

  switch (reload()) {
    case CALLWAIT_SYSOP    : Status.User = SYSOP;
                             Asy.Online = LOCAL;
                             Asy.ModemSpeed   =
                             Asy.ConnectSpeed =
                             Asy.CarrierSpeed = PcbData.ModemSpeed;
                             Asy.ErrorCorrected = TRUE;
                             login();
                             command();
                             break;
    case CALLWAIT_LOCALUSER: Status.User = USER;
                             Asy.Online = LOCAL;
                             Asy.ModemSpeed   =
                             Asy.ConnectSpeed =
                             Asy.CarrierSpeed = PcbData.ModemSpeed;
                             Asy.ErrorCorrected = TRUE;
                             login();
                             command();
                             break;
    case CALLWAIT_CALLER   : Asy.Online = REMOTE;
                             login();
                             command();
                             break;
    case CALLWAIT_ONLINE   : if (Control.GraphicsMode) { /* force it to reset the color! */
                               asetcolor(0);
                               printdefcolor();
                               if (Control.GraphicsMode && (Display.DefaultColor & 0xF0))
                                 printcls();
                             }
                             asetcolor(Display.DefaultColor); /* just in case we're in non-graphics, set the local color anyway */
                             command();
                             break;
    case CALLWAIT_RECYCLE  : recycle();
                             break;
    case CALLWAIT_EXIT     : PcbData.ExitToDos = TRUE;  /* force exit to dos */
                             break;
  }
  recycle();
  return(0);  /* it will never get this far */
}
