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


#ifdef COMM

// Define the name below if you want to log the IOCTL calls
// #define MODEMOS2TEST
#ifdef MODEMOS2TEST
  #include "project.h"
  #pragma hdrstop
#endif

#ifdef __OS2__
  #define INCL_DOSDEVICES
  #include <os2.h>
#endif

#include "devioctl.h"

#ifdef _MSC_VER
  #include <memory.h>
  #include <borland.h>
#else
  #include <mem.h>
  #ifndef __OS2__
//  #pragma inline
  #endif
#endif


#ifdef MODEMOS2TEST
void LIBENTRY logger(int Handle, int Category, int Function) {
  char Str[80];

  // NOTE:  Because there are so many calls to check for carrier status,
  // which is a function 0x67, this logger will IGNORE all calls for function
  // 0x67 unless MODEMOS2TEST2 is defined.

  #ifndef MODEMOS2TEST2
    if (Function == 0x67)
      return;
  #endif

  sprintf(Str,"IOCTL: H=%d, Cat=%x, Func=%x",Handle,Category,Function);
  writedebugrecord(Str);
}
#else
#define logger(h,c,f)
#endif


#ifdef __OS2__
int LIBENTRY DevIOCtl(int Handle, int Cat, int Func, void _FAR_ *DataPacket,unsigned long DataLen, void _FAR_ *CmdPacket, unsigned long CmdLen) {
#else
int LIBENTRY DevIOCtl(int Handle, int CatFunc, void _FAR_ *DataPacket, void _FAR_ *CmdPacket) {
#endif
  #ifdef __OS2__
    logger(Handle,Cat,Func);
    return(DosDevIOCtl(Handle,
                       Cat,
                       Func,
                       CmdPacket,
                       CmdLen,
                       &CmdLen,
                       DataPacket,
                       DataLen,
                       &DataLen));
  #else
    asm   mov  ax, 0x440C      /* Ah=0x44 IOCTL,  Al=0x0C  Handle based call */
    asm   mov  bx, [Handle]
    asm   mov  cx, [CatFunc]

    asm   les  di, CmdPacket
    asm   mov  si, es          /* SI:DI = pointer to command packet */

    asm   push ds
    asm   lds  dx, DataPacket  /* DS:DX = pointer to data packet */
    asm   int  0x21
    asm   pop  ds
    asm   jc   error
    return(0);

    error:
    #ifdef _MSC_VER
      return;        /* ignore error message, return value is in AX */
    #else
      return(_AX);
    #endif
  #endif  /* ifdef __OS2__ */
}        /*lint !e563 */


static int CommonHandle;
/* static v86mode NullPacket = {{{0,0}},0}; */

void LIBENTRY setioctrlhandle(int Handle) {
  CommonHandle = Handle;
}


#ifdef __OS2__
int LIBENTRY getioctrl(int Cat, int Func, void _FAR_ *DataPacket, unsigned long DataLen) {
#else
int LIBENTRY getioctrl(int CatFunc, void _FAR_ *DataPacket) {
#endif
  #ifdef __OS2__
    unsigned long CmdLen = 0;
    logger(CommonHandle,Cat,Func);
    return(DosDevIOCtl(CommonHandle,
                       Cat,
                       Func,
                       NULL,
                       0,
                       &CmdLen,
                       DataPacket,
                       DataLen,
                       &DataLen));
  #else
    asm   mov  ax, 0x440C      /* Ah=0x44 IOCTL,  Al=0x0C  Handle based call */
    asm   mov  bx, [CommonHandle]
    asm   mov  cx, [CatFunc]

    asm   xor  si, si
    asm   xor  di, di                 /* SI:DI = pointer to NULL command packet */

    asm   push ds
    asm   lds  dx, DataPacket  /* DS:DX = pointer to data packet */
    asm   int  0x21
    asm   pop  ds
    asm   jc   error
    return(0);

    error:
    #ifdef _MSC_VER
      return;        /* ignore error message, return value is in AX */
    #else
      return(_AX);
    #endif
  #endif  /* ifdef __OS2__ */
}        /*lint !e563 */


#ifdef __OS2__
int LIBENTRY setioctrl(int Cat, int Func, void _FAR_ *CmdPacket, unsigned long CmdLen) {
#else
int LIBENTRY setioctrl(int CatFunc, void _FAR_ *CmdPacket) {
#endif
  #ifdef __OS2__
    unsigned long DataLen = 0;
    logger(CommonHandle,Cat,Func);
    return(DosDevIOCtl(CommonHandle,
                       Cat,
                       Func,
                       CmdPacket,
                       CmdLen,
                       &CmdLen,
                       NULL,
                       0,
                       &DataLen));
  #else
    asm   mov  ax, 0x440C      /* Ah=0x44 IOCTL,  Al=0x0C  Handle based call */
    asm   mov  bx, [CommonHandle]
    asm   mov  cx, [CatFunc]

    asm   les  di, CmdPacket
    asm   mov  si, es          /* SI:DI = pointer to command packet */

    asm   push ds
    asm   xor  dx, dx
    asm   mov  ds, dx                 /* DS:DX = pointer to NULL data packet */
    asm   int  0x21
    asm   pop  ds
    asm   jc   error
    return(0);

    error:
    #ifdef _MSC_VER
      return;        /* ignore error message, return value is in AX */
    #else
      return(_AX);
    #endif
  #endif  /* ifdef __OS2__ */
}        /*lint !e563 */


#ifdef __OS2__
void LIBENTRY callioctrl(int Cat, int Func) {
#else
void LIBENTRY callioctrl(int CatFunc) {
#endif
  #ifdef __OS2__
    unsigned long CmdLen  = 0;
    unsigned long DataLen = 0;
    logger(CommonHandle,Cat,Func);
    DosDevIOCtl(CommonHandle,
                Cat,
                Func,
                NULL,
                0,
                &CmdLen,
                NULL,
                0,
                &DataLen);     //lint !e534
  #else
    asm   mov  ax, 0x440C      /* Ah=0x44 IOCTL,  Al=0x0C  Handle based call */
    asm   mov  bx, CommonHandle
    asm   mov  cx, [CatFunc]

    asm   xor  si, si
    asm   xor  di, di                 /* SI:DI = pointer to NULL command packet */

    asm   push ds
    asm   mov  ds, si
    asm   mov  dx, si                 /* DS:DX = pointer to NULL data packet */
    asm   int  0x21
    asm   pop  ds
  #endif  /* ifdef __OS2__ */
}

#endif   /*lint !e551 */
