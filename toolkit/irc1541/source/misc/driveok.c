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

#ifndef __OS2__
//#pragma inline
#endif

#include "dosfunc.h"
#include "misc.h"

bool LIBENTRY driveok(char Drive) {
  unsigned CurDisk;

  CurDisk = dosgetcurdrive();
  dossetcurdrive(Drive);
  if (dosgetcurdrive() == Drive) {
    dossetcurdrive(CurDisk);
    return(TRUE);
  }
  return(FALSE);
}


/* --------------------------------------------------------------------------*/


#ifdef OLDMETHOD
#ifdef __OS2__
  #ifdef __BORLANDC__
    #include <dir.h>
  #elif defined(__WATCOMC__)
    #include <dos.h>
    #include <direct.h>
  #endif
#endif

#include <misc.h>
#ifdef DEBUG
  #include <memcheck.h>
#endif

/* Drive must be 'A', 'B', 'C' and so on */


bool LIBENTRY driveok(char Drive) {

  Drive -= 'A';        /* set Drive to base 0         */

#ifdef __OS2__
  unsigned CurDisk;
  bool     RetVal;

  #ifdef __WATCOMC__
    unsigned Temp,Total;
    _dos_getdrive(&CurDisk);
    _dos_setdrive(Drive,&Total);
    _dos_getdrive(&Temp);
    RetVal = (bool) (Temp == Drive);
    _dos_setdrive(CurDisk,&Total);
  #else
    CurDisk = getdisk();
    setdisk(Drive);
    RetVal = (bool) (getdisk() == Drive);
    setdisk(CurDisk);
  #endif
  return(RetVal);

#else  /* ifdef __OS2__ */


                       /* Ah = DOS function call      */
                       /* Al = DOS function result    */
                       /* Bh = function result        */
                       /* Bl = original drive         */
                       /*                             */
  asm  Mov Ah,19h      /* Get current drive           */
  asm  Int 21h         /*                             */
  asm  Cmp Drive,Al    /* is drv = current drive?     */
  asm  Jne J1          /*   No , continue             */
  asm  Mov Bh,1        /*   Yes, Drive is OK (Bh=1)   */
  asm  Jmp J2          /*                             */
J1:                    /*                             */
  asm  Mov Bl,Al       /* store original drive Bl     */
                       /*                             */
  asm  Mov Ah,0Eh      /* change to new Drv           */
  asm  Mov Dl,Drive    /*                             */
  asm  Int 21h         /*                             */
                       /*                             */
  asm  Mov Ah,19h      /* Get current drive           */
  asm  Int 21h         /*                             */
  asm  Cmp Bl,Al       /* is OldDrv<>NewDrv           */
  asm  Je  J3          /*                             */
  asm  Mov Bh,1        /*   Yes, Drive is OK  (Bh=1)  */
  asm  Jmp J4          /*                             */
J3:                    /*                             */
  asm  Xor Bh,Bh       /*   No , Drive is BAD (Bh=0)  */
J4:                    /*                             */
  asm  Mov Ah,0Eh      /* change to original drive    */
  asm  Mov Dl,Bl       /*                             */
  asm  Int 21h         /*                             */
J2:                    /*                             */
  return(_BH);
#endif  /* ifdef __OS2__ */
}
#endif
