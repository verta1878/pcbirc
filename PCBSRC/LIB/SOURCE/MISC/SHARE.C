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

#include <io.h>
#include <stdio.h>
#include "dosfunc.h"
#include "misc.h"
#ifdef DEBUG
  #include <memcheck.h>
#endif

#pragma warn -rvl
/********************************************************************
*
*  Function: shareloaded()
*
*  Desc    : Checks to see if SHARE is loaded
*
*  Returns : TRUE if loaded, FALSE otherwise
*/

bool LIBENTRY shareloaded(void) {
#ifdef __OS2__
  return(TRUE);
#else
  asm  Push  Ds
  asm  Mov   Ax,1000h
  asm  Int   2Fh
  asm  Pop   Ds
  asm  Or    Al,Al
  asm  Jnz   exit

  asm  Push Ds
  asm  Xor  Ax,Ax
  asm  Mov  Ds,Ax
  asm  Mov  Bx,Ds:[0BCh]
  asm  Mov  Dx,Ds:[0BEh]
  asm  Mov  Ds,Dx
  asm  Cmp  byte ptr [Bx],80h
  asm  Jne  end
  asm  Cmp  byte ptr [Bx+1],0FCh
  asm  Jne  end
  asm  Cmp  byte ptr [Bx+2],10h
  asm  Jne  end
  asm  Inc  Ax
end:
  asm  Pop  Ds

  /* we'll return AL here */
exit:;
#endif
}


/********************************************************************
*
*  Function: testforshareloaded()
*
*  Desc    : Checks to see if SHARE is loaded
*
*  Returns : TRUE if loaded, FALSE otherwise
*/

#pragma argsused
bool LIBENTRY testforshareloaded(char *FileName) {
#ifdef __OS2__
    return(TRUE);
#else
  int  Handle;
  bool RetVal;

  if (FileName[0] == 0)
    return(FALSE);

  if (fileexist(FileName) == 255) {
    if ((Handle = doscreate(FileName,OPEN_WRIT|OPEN_DENYNONE,OPEN_NORMAL)) == -1)
      return(FALSE);
    dosclose(Handle);
  }

  if ((Handle = dosopen(FileName,OPEN_READ|OPEN_DENYNONE)) == -1)
    return(FALSE);
  else {
    if (lock(Handle,0,6) == -1)
      RetVal = FALSE;
    else {
      unlock(Handle,0,6);
      RetVal = TRUE;
    }
    dosclose(Handle);
  }

  return(RetVal);
#endif
}
