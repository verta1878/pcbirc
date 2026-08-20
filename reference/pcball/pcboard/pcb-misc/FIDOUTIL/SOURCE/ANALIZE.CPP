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


/****************************************************************************
*
*                             PCB FIDO Utility
*                             Module ANALIZE.CPP
*
*                         Written by Stan Paulsen
*                            Copyright (C) 1995
*                            Clark Development
*
*****************************************************************************
*
*        Functions to analize and report Fido System configuration
*
*****************************************************************************/


// Borland Header files
#include <iostream.h>
#include <stdio.h>
// #include <conio.h>
#include <dir.h>

#include <stdlib.h>
#include <string.h>

// PCBoard headers

//#include <misc.h>
#include <bug.h>
#include <screen.h>
#include <scrnio.h>
#include <scrnio.ext>
//#include <pcbtools.h>
#include <misc.h>
#include <pcb.h>
#include <help.h>

extern "C" int _Cdecl getch( void );

void LIBENTRY analize(void)
{

  cls();
  cout <<endl<<"Under construction. Hang on! =:)"<<endl;
  cout <<endl<<"Press any key"<<endl;
  getch();

}

