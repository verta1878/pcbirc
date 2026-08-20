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


#ifndef H_SYSTEM
#define H_SYSTEM

#ifndef ___TYPES_HPP___
  #include <types.hpp>
#endif

#ifdef SECURE
  #define int21()  asm int 3h
#else
  #define int21()  asm int 21h
#endif

#pragma pack(2)
typedef struct {
  ubyte  Month;
  ubyte  Day;
  uint   Year;
  ubyte  DayOfWeek;
} sysdatetype;
#pragma pack()

#pragma pack(1)
typedef struct {
  ubyte  Hours;
  ubyte  Minutes;
  ubyte  Seconds;
  ubyte  Hundredths;
} systimetype;
#pragma pack()

#pragma pack(1)
typedef union {
  uint W;
  struct {
    ubyte LoB,HiB;
  } B;
} bkeytype;
#pragma pack()

void LIBENTRY getsysdate(sysdatetype *SysDate);
void LIBENTRY getsystime(systimetype *SysTime);
uint LIBENTRY bgetkey(ubyte Option);
uint LIBENTRY getkbdstatus(void);
void LIBENTRY setkbdstatus(uint Status);
void LIBENTRY setkbdbinarymode(void);

#ifdef __OS2__
void LIBENTRY enablekbdmonitor(void);
void LIBENTRY disablekbdmonitor(void);
void LIBENTRY openkbdhandle(void);
void LIBENTRY closekbdhandle(void);
#endif

#endif  /* H_SYSTEM */
