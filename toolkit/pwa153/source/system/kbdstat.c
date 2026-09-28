/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* KBDSTAT.C - the KbdStatus global.                                         */
/*                                                                           */
/* NOT Clark's file.  Reconstructed 2026-09-22, one line of actual code.      */
/*                                                                           */
/* SYSTEM\BGETKEY.C says                                                     */
/*                                                                           */
/*     extern uint far *KbdStatus;                                           */
/*                                                                           */
/* and getkbdstatus()/setkbdstatus() read and write through it.  Nothing in   */
/* this toolkit defines it, and neither did the SYSTEM_L.386 we were handed   */
/* (3 modules: BGETKEY, SYSDATE, SYSTIME).  SCRNIO\INITSCRN.C line 45 still   */
/* carries Clark's own note over the commented-out definition:               */
/*                                                                           */
/*     /* moved to SYSTEM.LIB */                                             */
/*     /* char far *KbdStatus;    pointer to Keyboard status byte */         */
/*                                                                           */
/* so SYSTEM.LIB is where Clark put it; the module just never reached us.     */
/* PCBOARD does not need this one: MAIN\INIT.C declares its own KbdStatus     */
/* and sets it at line 1104 with MK_FP(0x40,0x17).  Because this module       */
/* defines nothing else, TLINK pulls it out of the library only when          */
/* KbdStatus is still unresolved, so PCBOARD keeps using INIT.C's copy and    */
/* the utilities that do not link INIT.C (PCBSETUP, FIDOUTIL, PCBSM) get      */
/* this one.                                                                 */
/*                                                                           */
/* 0040:0017 is the BIOS keyboard shift-flags word, which is what INIT.C      */
/* points at.                                                                */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <dos.h>

#include "types.hpp"

uint far *KbdStatus = (uint far *) MK_FP(0x0040,0x0017);
