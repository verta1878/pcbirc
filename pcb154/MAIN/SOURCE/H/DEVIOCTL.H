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


#ifndef ___TYPES_HPP___
  #include "types.hpp"
#endif

typedef struct {
    unsigned short Offset;
    unsigned short Segment;
} farptr;

typedef struct {
  union {
    farptr    Registers;
    void _FAR_ *Direct;
  } Ptr;
  unsigned Length;
} v86mode;

#ifdef __OS2__
int  LIBENTRY DevIOCtl(int Handle, int Cat, int Func, void _FAR_ *DataPacket, unsigned long DataLen, void _FAR_ *CmdPacket, unsigned long CmdLen);
void LIBENTRY setioctrlhandle(int Handle);
int  LIBENTRY getioctrl(int Cat, int Func, void _FAR_ *DataPacket, unsigned long DataLen);
int  LIBENTRY setioctrl(int Cat, int Func, void _FAR_ *CmdPacket, unsigned long CmdLen);
void LIBENTRY callioctrl(int Cat, int Func);
#else
int  LIBENTRY DevIOCtl(int Handle, int CatFunc, void _FAR_ *DataPacket, void _FAR_ *CmdPacket);
void LIBENTRY setioctrlhandle(int Handle);
int  LIBENTRY getioctrl(int CatFunc, void _FAR_ *DataPacket);
int  LIBENTRY setioctrl(int CatFunc, void _FAR_ *CmdPacket);
void LIBENTRY callioctrl(int CatFunc);
#endif
