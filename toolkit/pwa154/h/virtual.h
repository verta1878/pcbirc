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

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* MERGED HEADER — one file for both implementations:                       */
/*                                                                          */
/*   #define VIRTUAL_HUGE  -> huge pointers, long counts (>64KB datasets)   */
/*   (default)             -> near pointers, unsigned counts (memory-only)  */
/*                                                                          */
/* Formerly two files (VIRTUAL.H and VIRTUAL1.H). The .C file that builds   */
/* the matching implementation must define (or not define) VIRTUAL_HUGE the */
/* same way this header is included.                                        */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#ifndef _VIRTUAL_H_
#define _VIRTUAL_H_

typedef struct {
  char  Rec;
  union {
    long  Offset;
#ifdef VIRTUAL_HUGE
    void  huge *Buf;
#else
    void  *Buf;
#endif
  } Loc;
} VirType;

#define MEMBUFM     0           /* indicates buffer was from MainMem  */
#define MEMBUFA     1           /* indicates buffer was ALLOCATED     */
#define DISKBUF     2           /* indicates a disk buffer            */

#define VIRDISKERR -1           /* indicates a disk error             */
#define VIROKAY     0           /* indicates no errors                */
#define VIRPASTEND  1           /* tried to go past end of Virtual[]  */

#ifdef VIRTUAL_HUGE

extern VirType huge *Virtual;

#define getvirtualptr(p,ptr,buffer)  \
          if (p->Loc.Buf >= Virtual) \
            ptr = p->Loc.Buf;        \
          else {                     \
            getvirtualrec(p,buffer); \
            ptr = buffer;            \
          }

#ifdef __cplusplus
extern "C" {
#endif

int  pascal delvirtualrec(long RecNum, long TotRecs);
void pascal freevirtual(long NumRecs);
void pascal getvirtualrec(const VirType huge *p, void *Buffer);
int  pascal insvirtualrec(void *EmptyBuffer, long RecNum, long TotRecs);
void pascal openvirtual(long MaxRecs, unsigned Size);
int  pascal putvirtualrec(VirType huge *p, void *Buffer);
void pascal updvirtualrec(const VirType huge *p, void *Buffer);

#ifdef __cplusplus
}
#endif

#else  /* near-pointer, memory-only */

extern VirType  *Virtual;

#define getvirtualptr(p,ptr,buffer)  \
          if (p->Rec != DISKBUF)     \
            ptr = p->Loc.Buf;        \
          else {                     \
            getvirtualrec(p,buffer); \
            ptr = buffer;            \
          }

int  pascal delvirtualrec(unsigned RecNum, unsigned TotRecs);
void pascal freevirtual(unsigned NumRecs);
void pascal getvirtualrec(const VirType *p, void *Buffer);
int  pascal insvirtualrec(void *EmptyBuffer, unsigned RecNum, unsigned TotRecs);
void pascal openvirtual(unsigned MaxRecs, unsigned Size);
int  pascal putvirtualrec(VirType *p, const void *Buffer);
void pascal updvirtualrec(const VirType *p, void *Buffer);

#endif /* VIRTUAL_HUGE */

#endif /* _VIRTUAL_H_ */
