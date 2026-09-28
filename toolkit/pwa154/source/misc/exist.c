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


#ifdef __OS2__
  #define INCL_DOSFILEMGR
  #include <os2.h>
  #include <string.h>
#else
  #ifdef _MSC_VER
    #include <borland.h>
    #include <direct.h>
  #else
/*  #pragma inline */
    #include <dir.h>
    #include "model.h"
  #endif
#endif

#include <dos.h>
#include "misc.h"
#ifdef DEBUG
#include <memcheck.h>
#endif

#pragma warn -rvl
/********************************************************************
*
*  Function: fileexist()
*
*  Desc    : Determine if file specified as Str[] exists.
*
*  Returns : 255 if file does not exist.  Otherwise it returns the DOS file
*            attribute of the "found" file.
*/

#if defined(__BORLANDC__) || defined(__TURBOC__)
  struct ffblk DTA;
#elif defined(__WATCOMC__)
  struct find_t DTA;
  #define ff_name   name
  #define ff_ftime  wr_time
  #define ff_fdate  wr_date
  #define ff_fsize  size
  #define ff_attrib attrib
#endif

unsigned char LIBENTRY fileexist(char *StrPtr) {
  #ifdef __OS2__
    HDIR          FindHandle;
    FILEFINDBUF3  FindBuffer;
    ULONG         FindCount;
    APIRET        rc;

    FindHandle = 0x0001;
    FindCount  = 1;

    rc = DosFindFirst(StrPtr,
                      &FindHandle,
                      0x17,
                      (PFILEFINDBUF)&FindBuffer,
                      sizeof(FindBuffer),
                      &FindCount,
                      FIL_STANDARD);
    if (rc != 0 || FindCount != 1)
      return(255);

    strcpy(DTA.ff_name,FindBuffer.achName);
    DTA.ff_ftime  = *(USHORT *)&FindBuffer.ftimeLastWrite;
    DTA.ff_fdate  = *(USHORT *)&FindBuffer.fdateLastWrite;
    DTA.ff_fsize  = FindBuffer.cbFile;
    DTA.ff_attrib = FindBuffer.attrFile;
    return((unsigned char) DTA.ff_attrib);

  #else

    #ifdef LDATA
      asm  Push Ds
      asm  Mov  Ax,seg DTA
      asm  Mov  Ds,Ax
    #endif

      asm  Mov  Dx,offset DTA  /* DS:DX points to DTA address */
      asm  Mov  Ah,1Ah         /* Tell DOS where our DTA is   */
      int21();

    #ifdef LDATA
      asm  Lds  Dx,StrPtr      /* DS:DX points to string address        */
    #else
      asm  Mov  Dx,StrPtr      /* DS:DX points to string address        */
    #endif

    /*asm  Mov  Cx,1Fh */      /* Set to find any file                  */
      asm  Mov  Cx,17h         /* Set to find any file                  */
      asm  Mov  Ah,4Eh         /* Dos function call "Find 1st Matching" */
      int21();

      asm  Mov  Al,255 /* Set Return code to 255 and return if error */
      asm  Jc   End

    #ifdef LDATA
      asm  Mov  Ax,seg DTA
      asm  Mov  Ds,Ax
    #endif
      asm  Mov  Al,byte ptr DTA.ff_attrib

    End:
    #ifdef LDATA
      asm  Pop  Ds
    #endif
/*    return; */   /* ignore "must return a value" message, it's in AL already */
  #endif
  ;
}
