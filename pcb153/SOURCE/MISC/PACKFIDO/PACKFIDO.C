/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* PACKFIDO.C - STUB.  Not Clark's code.                                     */
/*                                                                           */
/* Clark's PACKFIDO packs the FIDO configuration file PCBFIDO.CFG, removing  */
/* records that are no longer referenced by any conference, and reports each */
/* removal on screen ("removed %5d %s...").  WHATSNEW.FID says to run it     */
/* before FIDOUTIL's conversion so duplicates are packed out first.          */
/*                                                                           */
/* FIDOUTIL links one symbol from this module: do_pack().  CONVERT.CPP       */
/* declares it at line 53 and its only call, at line 124, is commented out,  */
/* so nothing in FIDOUTIL reaches this code today.  This stub exists so the  */
/* build is complete and honest: the module is present, the symbol resolves, */
/* and the behaviour is not faked.                                           */
/*                                                                           */
/* TO REPLACE THIS FILE: the shipped PACKFIDO.EXE is in PCBinstalled.zip     */
/* (PCB\PACKFIDO.EXE) with PACK.DOC beside it.  It is a Borland C++ build,   */
/* "Copyright 1991" in the runtime, and reads PCBOARD.DAT, CNAMES.@@@,       */
/* CNAMES.ADD, FIDOQUE.DAT and tmp.cfg.  The v2 (15.21) PCBFIDO.CFG record   */
/* layout it walks is the `oldver == 2` path in FIDOUTIL's CONVERT.CPP.      */
/*                                                                           */
/* pcbirc, 2026-09-22.  GPLv3.                                               */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <stdio.h>

void do_pack(void);

/*
 * do_pack() - stub.
 *
 * Clark's version rewrites PCBFIDO.CFG without the dead records.  Doing
 * that wrongly would corrupt a live FIDO configuration, so the stub does
 * nothing at all and says so.  Losing records silently is worse than not
 * packing.
 */
void do_pack(void) {
  printf("PACKFIDO: not implemented in this build - PCBFIDO.CFG left unchanged.\n");
}
